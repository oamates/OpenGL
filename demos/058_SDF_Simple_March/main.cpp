//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D raymarcher
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

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


GLuint generate_spheric_sdf(GLenum texture_unit, double radius, const char* file_name)
{
    //===============================================================================================================================================================================================================
    // create 3d texture of the type GL_R32F
    //===============================================================================================================================================================================================================
    GLuint texture_id;
    glActiveTexture(texture_unit);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const int p2 = 256;
    const int texture_size = p2 * p2 * p2;

    glm::vec4* texture_data = (glm::vec4*) malloc(texture_size * sizeof(glm::vec4));

    int index = 0;
    double scale = 1.0 / p2;

    for(int w = -p2 + 1; w <= p2 - 1; w += 2)
        for(int v = -p2 + 1; v <= p2 - 1; v += 2)
            for(int u = -p2 + 1; u <= p2 - 1; u += 2)
            {
                glm::dvec3 p = scale * glm::dvec3(u, v, w);
                glm::dvec4 q = glm::dvec4(glm::normalize(p), -radius);
                texture_data[index++] = glm::vec4(q);
            }

    if (file_name)
    {
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_RGBA32F,
            .format = GL_RGBA,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = (uint32_t) texture_size * sizeof(glm::vec4)
        };  

        FILE* f = fopen(file_name, "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, p2, p2, p2, 0, GL_RGBA, GL_FLOAT, texture_data);

    free(texture_data);
    return texture_id;
}


GLuint generate_spheric_udf(GLenum texture_unit, double radius, const char* file_name)
{
    //===============================================================================================================================================================================================================
    // create 3d texture of the type GL_R32F
    //===============================================================================================================================================================================================================
    GLuint texture_id;
    glActiveTexture(texture_unit);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const int p2 = 256;
    const int texture_size = p2 * p2 * p2;

    float* texture_data = (float*) malloc(texture_size * sizeof(float));

    int index = 0;
    double scale = 1.0 / p2;

    for(int w = -p2 + 1; w <= p2 - 1; w += 2)
        for(int v = -p2 + 1; v <= p2 - 1; v += 2)
            for(int u = -p2 + 1; u <= p2 - 1; u += 2)
            {
                glm::dvec3 p = scale * glm::dvec3(u, v, w);
                //texture_data[index++] = glm::length(p) - radius;
                texture_data[index++] = glm::max(glm::max(glm::abs(p.x), glm::abs(p.y)), glm::abs(p.z)) - radius;
            }

    if (file_name)
    {
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_R32F,
            .format = GL_RED,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = (uint32_t) texture_size * sizeof(glm::vec4)
        };  

        FILE* f = fopen(file_name, "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

    free(texture_data);
    return texture_id;
}


double sdf(const glm::dvec3& p)
{
    const double phi = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
    const double psi = 1.511522628152341460960267404050002785276889577787122118459; // (sqrt(5) + 3) / (2 * sqrt(3))
    glm::dvec3 q = glm::abs(p);
    glm::dvec3 l = q + phi * glm::dvec3(q.y, q.z, q.x);
    return 0.5 * glm::max(l.x, glm::max(l.y, l.z)) - 0.5 * psi;
}

glm::dvec3 grad(const glm::dvec3& p)
{
    const double delta = 0.125 * 0.00048828125;
    const double inv_delta2 = 8.0 * 1024.0;
    glm::dvec3 dF = glm::dvec3(
                        sdf(glm::dvec3(p.x + delta, p.y, p.z)) - sdf(glm::dvec3(p.x - delta, p.y, p.z)),
                        sdf(glm::dvec3(p.x, p.y + delta, p.z)) - sdf(glm::dvec3(p.x, p.y - delta, p.z)),
                        sdf(glm::dvec3(p.x, p.y, p.z + delta)) - sdf(glm::dvec3(p.x, p.y, p.z - delta))
                    );

    return inv_delta2 * dF;
}

GLuint generate_dodecahedron_sdf(GLenum texture_unit, double radius, const char* file_name)
{
    const double phi = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
    const double psi = 1.511522628152341460960267404050002785276889577787122118459; // (sqrt(5) + 3) / (2 * sqrt(3))

    //===============================================================================================================================================================================================================
    // create 3d texture of the type GL_R32F
    //===============================================================================================================================================================================================================
    GLuint texture_id;
    glActiveTexture(texture_unit);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const int p2 = 256;
    const int texture_size = p2 * p2 * p2;

    glm::vec4* texture_data = (glm::vec4*) malloc(texture_size * sizeof(glm::vec4));

    int index = 0;
    double scale = 1.0 / p2;

    for(int w = -p2 + 1; w <= p2 - 1; w += 2)
        for(int v = -p2 + 1; v <= p2 - 1; v += 2)
            for(int u = -p2 + 1; u <= p2 - 1; u += 2)
            {
                glm::dvec3 p = scale * glm::dvec3(u, v, w);

                double value = sdf(p);
                glm::dvec3 q = grad(p);

                glm::dvec4 w = glm::dvec4(q, value - glm::dot(q, p));
                texture_data[index++] = glm::vec4(w);
            }

    if (file_name)
    {
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_RGBA32F,
            .format = GL_RGBA,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = (uint32_t) texture_size * sizeof(glm::vec4)
        };  

        FILE* f = fopen(file_name, "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, p2, p2, p2, 0, GL_RGBA, GL_FLOAT, texture_data);

    free(texture_data);
    return texture_id;
}

GLuint generate_dodecahedron_udf(GLenum texture_unit, double radius, const char* file_name)
{
    const double phi = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
    const double psi = 1.511522628152341460960267404050002785276889577787122118459; // (sqrt(5) + 3) / (2 * sqrt(3))

    //===============================================================================================================================================================================================================
    // create 3d texture of the type GL_R32F
    //===============================================================================================================================================================================================================
    GLuint texture_id;
    glActiveTexture(texture_unit);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const int p2 = 256;
    const int texture_size = p2 * p2 * p2;

    float* texture_data = (float*) malloc(texture_size * sizeof(float));

    int index = 0;
    double scale = 1.0 / p2;

    for(int w = -p2 + 1; w <= p2 - 1; w += 2)
        for(int v = -p2 + 1; v <= p2 - 1; v += 2)
            for(int u = -p2 + 1; u <= p2 - 1; u += 2)
            {
                glm::dvec3 p = scale * glm::dvec3(u, v, w);
                glm::dvec3 q = glm::abs(p);
                glm::dvec3 l = q + phi * glm::dvec3(q.y, q.z, q.x);
                texture_data[index++] = 0.5 * glm::max(l.x, glm::max(l.y, l.z)) - 0.5 * psi;
            }

    if (file_name)
    {
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_R32F,
            .format = GL_RED,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = (uint32_t) texture_size * sizeof(float)
        };  

        FILE* f = fopen(file_name, "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

    free(texture_data);
    return texture_id;
}


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Simple Volume RayMarch", 4, 3, 3, res_x, res_y, true);

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

//    texture3d_t demon_sdf(GL_TEXTURE2, "../../../resources/sdf/demon_rgba.sdf");

//    GLuint q = generate_dodecahedron_sdf(GL_TEXTURE2, 0.71319747, "sphere_rgba.sdf");

    bbox_t bbox;
    texture3d_t demon_sdf = texture3d_t::load_sdf(GL_TEXTURE2, "demon.sdf", bbox);

    glm::dvec3 bbox_size = bbox.size;
    glm::dvec3 bbox_center = bbox.center;
    glm::dvec3 bbox_min = bbox_center - 0.5 * bbox_size;
    glm::dvec3 bbox_max = bbox_center + 0.5 * bbox_size;    

    debug_msg("SDF loaded :: bbox size = %s", glm::to_string(bbox_size).c_str());
    debug_msg("              bbox center = %s", glm::to_string(bbox_center).c_str());

    sdf_raymarch["bbox_min"] = glm::vec3(bbox_min);
    sdf_raymarch["bbox_center"] = glm::vec3(bbox_center);
    sdf_raymarch["bbox_half_size"] = glm::vec3(0.5 * bbox_size);
    sdf_raymarch["bbox_inv_size"] = glm::vec3(1.0 / bbox_size);

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

if (0 == 0) {


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
