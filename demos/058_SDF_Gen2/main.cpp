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
#include "sdf.hpp"
#include "tex3d.hpp"
#include "makelevelset3.hpp"

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
    int res_x = 1920;
    int res_y = 1080;
/*
    if(argc != 4)
    {
<<<<<<< HEAD
        printf("Usage :: %s <filename> <delta> <padding> ", argv[0]);
=======
        printf("Usage %s <filename> <dx> <padding>", argv[0]);
>>>>>>> Fixed Gen2
        std::exit(-1);
    }

    std::string filename(argv[1]);

    if(filename.size() < 5 || filename.substr(filename.size() - 4) != std::string(".obj"))
    {
        printf("Error: Expected OBJ file with filename of the form <name>.obj.\n");
        std::exit(-1);
    }


    std::stringstream arg2(argv[2]);
    double delta;
    arg2 >> delta;
  
    std::stringstream arg3(argv[3]);
    int padding;
    arg3 >> padding;

    if(padding < 1)
        padding = 1;
    
    // start with a massive inside out bound box.
    glm::dvec3 min_box( std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()), 
               max_box(-std::numeric_limits<double>::max(),-std::numeric_limits<double>::max(),-std::numeric_limits<double>::max());
  
    std::cout << "Reading data.\n";

    std::ifstream infile(argv[1]);
    if(!infile)
    {
        printf("Failed to open file. Terminating.\n");
        exit(-1);
    }

    int ignored_lines = 0;
    std::string line;
    std::vector<glm::dvec3> positions;
    std::vector<glm::uvec3> faces;
    while(!infile.eof())
    {
        std::getline(infile, line);

        //.obj files sometimes contain vertex normals indicated by "vn"
        if(line.substr(0, 1) == std::string("v") && line.substr(0, 2) != std::string("vn"))
        {
            std::stringstream data(line);
            char c;
            glm::dvec3 point;
            data >> c >> point[0] >> point[1] >> point[2];
<<<<<<< HEAD
            positions.push_back(point);
            min_box = glm::min(min_box, point);
            max_box = glm::max(max_box, point);
=======
            vertList.push_back(point);
            min_box = glm::min(min_box, glm::dvec3(point));
            max_box = glm::max(max_box, glm::dvec3(point));
>>>>>>> Fixed Gen2
        }
        else if(line.substr(0, 1) == std::string("f"))
        {
            std::stringstream data(line);
            char c;
            glm::uvec3 face;
            data >> c >> face.x >> face.y >> face.z;
            faces.push_back(face - glm::uvec3(1));
        }
        else
            ++ignored_lines; 
    }
    infile.close();
  
    if(ignored_lines > 0)
        debug_msg("Warning: %u lines were ignored since they did not contain faces or vertices.", ignored_lines);

    debug_msg("Read in %u vertices and %u faces.", (unsigned int) positions.size(), (unsigned int) faces.size());

    //===================================================================================================================================================================================================================
    // Add padding around the box.
    //===================================================================================================================================================================================================================
    glm::dvec3 unit = glm::dvec3(1.0);
    min_box -= padding * delta * unit;
    max_box += padding * delta * unit;
<<<<<<< HEAD
    double inv_delta = 1.0 / delta;
    glm::ivec3 sizes = glm::ivec3(glm::ceil(inv_delta * (max_box - min_box)));
=======
    glm::ivec3 sizes = glm::ivec3(glm::ceil((max_box - min_box) / delta));
>>>>>>> Fixed Gen2
  
    debug_msg("Bounding box: (%s) to (%s) :: dimensions :: %s", glm::to_string(min_box).c_str(), glm::to_string(max_box).c_str(), glm::to_string(sizes).c_str());
    debug_msg("Computing signed distance field ... ");

    array3d<double> distance_field;

<<<<<<< HEAD
    make_level_set3(faces, positions, min_box, delta, sizes, distance_field);

    std::string outname = filename.substr(0, filename.size() - 4) + std::string(".sdf");
    debug_msg("Writing results to: %s", outname.c_str());
=======
    make_level_set3(faceList, vertList, min_box, delta, sizes, phi_grid);

    std::string outname = filename.substr(0, filename.size() - 4) + std::string(".sdf");
    debug_msg("Writing results to: %s", outname.c_str());

    tex3d_header_t header = 
    {
        .target = GL_TEXTURE_3D,
        .internal_format = GL_R32F,
        .format = GL_RED,
        .type = GL_FLOAT,
        .size = sizes,
        .data_size = sizes.x * sizes.y * sizes.z * sizeof(GLfloat)
    };

    FILE* f = fopen(outname.c_str(), "wb");
    fwrite(&header, sizeof(tex3d_header_t), 1, f);
    fwrite(phi_grid.a.data(), header.data_size, 1, f);
    fclose(f);    

    debug_msg("Processing complete.");

*/

>>>>>>> Fixed Gen2


    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Simple Volume RayMarch", 4, 3, 3, res_x, res_y, true);

<<<<<<< HEAD


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

    FILE* f = fopen(outname.c_str(), "wb");
    fwrite(&header, sizeof(tex3d_header_t), 1, f);
    fwrite(tex_data, data_size, 1, f);
    fclose(f);

    texture3d_t distance_tex(GL_TEXTURE2, header, tex_data);
    free(tex_data);

=======
>>>>>>> Fixed Gen2
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

<<<<<<< HEAD
=======
    //texture3d_t demon_sdf(GL_TEXTURE2, outname.c_str());
    texture3d_t demon_sdf(GL_TEXTURE2, "demon.sdf");

>>>>>>> Fixed Gen2
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

