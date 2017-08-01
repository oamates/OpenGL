//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D raymarcher
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "tex3d.hpp"
#include "sdf_swipe.hpp"
#include "hqs_model.hpp"


#include "tex3d.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.01f);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);  
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt); 
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt); 
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{

    if(argc != 4)
    {
        printf("Usage :: %s <filename> <delta> <padding> ", argv[0]);
        std::exit(-1);
    }

    int max_level = 9;
    int p2 = 1 << max_level;
    int p2m1 = 1 << (max_level - 1);
    double inv_p2m1 = 1.0 / p2m1;
    double texel_size = inv_p2m1;

    std::string filename(argv[1]);
    if(filename.size() < 5 || filename.substr(filename.size() - 4) != std::string(".obj"))
    {
        printf("Error: expected OBJ file with filename of the form <name>.obj.\n");
        std::exit(-1);
    }

    double delta = texel_size;
    if(sscanf(argv[2], "%lf", &delta) != 1)
    {
        printf("Error parsing delta argument, using default value :: delta = %f\n", delta);
    }

    int padding = 2;
    if(sscanf(argv[3], "%u", &padding) != 1)
    {
        printf("Error parsing padding argument, using default value :: padding = %f\n", delta);
    }

    hqs_model_t model(argv[1]);
    model.normalize(1.0 - padding * delta);
    model.sort_faces_by_area();
    
    double inv_delta = 1.0 / delta;
    glm::ivec3 sizes = glm::ivec3(glm::ceil(inv_delta * (model.bbox_max - model.bbox_min))) + 2 * glm::ivec3(padding);

    glm::dvec3 bbox_size = glm::dvec3(sizes) * delta;
    glm::dvec3 bbox_center = 0.5 * (model.bbox_max + model.bbox_min);
    glm::dvec3 bbox_min = bbox_center - 0.5 * bbox_size;
    glm::dvec3 bbox_max = bbox_center + 0.5 * bbox_size;
  
    debug_msg("BBox: min = (%s) to (%s) :: dimensions :: %s", glm::to_string(model.bbox_min).c_str(), glm::to_string(model.bbox_max).c_str(), glm::to_string(sizes).c_str());
    debug_msg("Centralized bbox min :: %s", glm::to_string(bbox_min).c_str());
    debug_msg("Centralized bbox max :: %s", glm::to_string(bbox_max).c_str());
    debug_msg("Computing signed distance field ... ");

    array3d<double> distance_field = make_level_set3(model.faces, model.positions, bbox_min, delta, sizes, padding);


    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    int res_x = 1920;
    int res_y = 1080;

    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Simple Volume RayMarch", 4, 3, 3, res_x, res_y, true);

    uint32_t data_size = (uint32_t) sizes.x * sizes.y * sizes.z * sizeof(GLfloat);
    float* tex_data = (float*) malloc(data_size);

    for(int idx = 0; idx < sizes.x * sizes.y * sizes.z; ++idx)
    {
        tex_data[idx] = distance_field.a[idx];
    }

    tex3d_header_t header 
    {
        .target = GL_TEXTURE_3D,
        .internal_format = GL_R32F,
        .format = GL_RED,
        .type = GL_FLOAT,
        .size = sizes,
        .data_size = data_size
    };

    bbox_t bbox = 
    {
        .center = bbox_center,
        .size = bbox_size        
    };

    sdf_header_t sdf_header
    {
        .tex3d_header = header,
        .bbox = bbox
    };

    std::string outname = filename.substr(0, filename.size() - 4) + std::string(".sdf");
    debug_msg("Writing results to: %s", outname.c_str());

    FILE* f = fopen(outname.c_str(), "wb");
    fwrite(&sdf_header, sizeof(sdf_header_t), 1, f);
    fwrite(tex_data, data_size, 1, f);
    fclose(f);

    texture3d_t distance_tex(GL_TEXTURE2, header, tex_data);
    free(tex_data);

    debug_msg("Processing complete.");



    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t sdf_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sdf_marcher.fs"));

    sdf_raymarch.enable();

    uniform_t uni_cm_camera_matrix = sdf_raymarch["camera_matrix"];       
    uniform_t uni_cm_camera_ws     = sdf_raymarch["camera_ws"];       
    uniform_t uni_cm_light_ws      = sdf_raymarch["light_ws"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);

    sdf_raymarch["focal_scale"] = focal_scale;
    sdf_raymarch["tb_tex"] = 0;
    sdf_raymarch["environment_tex"] = 1;
    sdf_raymarch["sdf_tex"] = 2;

    sdf_raymarch["bbox_min"] = glm::vec3(bbox_min);
    sdf_raymarch["bbox_center"] = glm::vec3(bbox_center);
    sdf_raymarch["bbox_half_size"] = glm::vec3(0.5 * bbox_size);
    sdf_raymarch["bbox_inv_size"] = glm::vec3(1.0 / bbox_size);

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/marble.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    const char* sunset_files[6] = {"../../../resources/cubemap/sunset/positive_x.png",
                                   "../../../resources/cubemap/sunset/negative_x.png",
                                   "../../../resources/cubemap/sunset/positive_y.png",
                                   "../../../resources/cubemap/sunset/negative_y.png",
                                   "../../../resources/cubemap/sunset/positive_z.png",
                                   "../../../resources/cubemap/sunset/negative_z.png"};
    GLuint env_tex_id = image::png::cubemap(sunset_files);

    texture3d_t demon_sdf(GL_TEXTURE2, "demon.sdf");

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT);

        float time = 0.55 * window.frame_ts;
        glm::vec3 light_ws = glm::vec3(10.0f, 2.0f * glm::cos(time), 3.0f * glm::sin(time));

        glm::mat4 cmatrix4x4;
        glm::mat3 camera_matrix;
        glm::vec3 camera_ws;

if (0 != 0) {


        float radius = 2.65f + 1.15f * glm::cos(0.25f * time);
        float z = 0.35f * glm::sin(0.25f * time);
        camera_ws = glm::vec3(radius * glm::cos(0.3f * time), z, radius * glm::sin(0.3f * time));
        glm::vec3 up = glm::normalize(glm::vec3(glm::cos(0.41 * time), -6.0f, glm::sin(0.41 * time)));
        glm::mat4 view_matrix = glm::lookAt(camera_ws, glm::vec3(0.0f), up);
        cmatrix4x4 = glm::inverse(view_matrix);
        camera_matrix = glm::mat3(cmatrix4x4);
}
else
{
        cmatrix4x4 = glm::inverse(window.camera.view_matrix);
        camera_matrix = glm::mat3(cmatrix4x4);
        camera_ws = glm::vec3(cmatrix4x4[3]);
}


        //===============================================================================================================================================================================================================
        // raymarch through signed distance field
        //===============================================================================================================================================================================================================
        uni_cm_camera_matrix = camera_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}

