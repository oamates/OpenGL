//========================================================================================================================================================================================================================
// DEMO 008 : Geometry Shader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "polyhedron.hpp"
#include "plato.hpp"
#include "normalmap.hpp"
#include "image/stb_image.h"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool pause = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true),
          camera(32.0, 0.125, glm::mat4(1.0f))
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
            pause = !pause;
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

normalmap_params_t normalmap_params =
{
    .filter = FILTER_SOBEL_5x5,
    .minz = 0.0,
    .scale = 2.0,
    .wrap = 1,
    .height_source = 0,
    .alpha = ALPHA_HEIGHT,
    .conversion = CONVERT_NONE,
    .dudv = DUDV_NONE,
    .xinvert = 0,
    .yinvert = 0,
    .contrast = 0.0
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Samplers and HW/SW texture filtering", 4, 3, 3, res_x, res_y);

    //===================================================================================================================================================================================================================
    // load texture to test normal map generation routines
    //===================================================================================================================================================================================================================
    int width, height, bpp;
    const char* file_name = "../../../resources/plato_tex2d/cube.png";
    unsigned char* src_data = stbi_load(file_name, &width, &height, &bpp, 0);

    if ((!src_data) || ((bpp != 3) && (bpp != 4)))
        exit_msg("stbi :: failed to load image : %s", file_name);
    
    debug_msg("Texture loaded :: width = %u, height = %u, bpp = %u.", width, height, bpp);

    GLenum format = (bpp == 1) ? GL_RED :
                    (bpp == 2) ? GL_RG : 
                    (bpp == 3) ? GL_RGB : GL_RGBA;

    glActiveTexture(GL_TEXTURE0);
    GLuint diffuse_tex_id, bump_tex_id;
    glGenTextures(1, &diffuse_tex_id);
    glBindTexture(GL_TEXTURE_2D, diffuse_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, src_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    unsigned char* dst_data = (unsigned char*) malloc(width * height * 4);
    normalmap<double>(src_data, dst_data, width, height, bpp, normalmap_params);

    FILE* f = fopen("data.rgb", "w");
    int idx = 0;
    for(int y = 0; y < height; ++y)
        for(int x = 0; x < width; ++x)
        {
            fprintf(f, "rgb[%u, %u] = {%u, %u, %u, %u}\n", x, y, dst_data[idx + 0], dst_data[idx + 1], dst_data[idx + 2], dst_data[idx + 3]);
            idx += 4;
        }
    fclose(f);

    glActiveTexture(GL_TEXTURE1);

    glGenTextures(1, &bump_tex_id);
    glBindTexture(GL_TEXTURE_2D, bump_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(src_data);
    free(dst_data);

    debug_msg("Texture processing done");

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t simple_light(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/phong_light.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/phong_light.fs"));

    simple_light.enable();

    uniform_t uniform_projection_matrix = simple_light["projection_matrix"];
    uniform_t uniform_view_matrix       = simple_light["view_matrix"];      
    uniform_t uniform_time              = simple_light["time"];             
    uniform_t uniform_light_ws          = simple_light["light_ws"];         
    uniform_t uniform_camera_ws         = simple_light["camera_ws"];         

    simple_light["solid_scale"] = 1.0f;
    simple_light["diffuse_tex"] = 0;
    simple_light["normal_tex"] = 1;

    glm::vec4 shift_rotor[128];

    float middle = 1.5f;
    float cell_size = 2.0f;
    int index = 0;
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) for (int k = 0; k < 4; ++k)
    {
        shift_rotor[index++] = glm::vec4(cell_size * glm::vec3(float(i) - middle, float(j) - middle, float(k) - middle), 0.0f);
        shift_rotor[index++] = glm::vec4(glm::sphericalRand(1.0f), glm::gaussRand(0.0f, 1.0f));
    }

    simple_light["shift_rotor"] = shift_rotor;

    //===================================================================================================================================================================================================================
    // initialize buffers : vertex + tangent frame + texture coordinates 
    // for five regular plato solids and load the diffuse + bump textures for each
    //===================================================================================================================================================================================================================
    polyhedron cube;
    cube.regular_pft2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces);

    //===================================================================================================================================================================================================================
    // global GL settings :
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    uniform_projection_matrix = window.camera.projection_matrix;

    float t = 0.0f;
    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        if (!window.pause)
            t += window.frame_dt;

        uniform_time = t;
        uniform_view_matrix = window.camera.view_matrix;
    
        glm::vec3 light_ws = cell_size * glm::vec3(glm::cos(0.25f * t), glm::sin(0.25f * t), 2.0f);
        uniform_light_ws = light_ws;

        glm::vec3 camera_ws = window.camera.position();
        uniform_camera_ws = camera_ws;

        cube.instanced_render(64);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0; 
}