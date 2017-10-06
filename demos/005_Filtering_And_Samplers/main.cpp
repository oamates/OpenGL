//========================================================================================================================================================================================================================
// DEMO 005 : Samplers and HW/SW texture filtering
//========================================================================================================================================================================================================================

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "imgui_window.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include "vao.hpp"
#include "sampler.hpp"
#include "polyhedron.hpp"
#include "plato.hpp"
#include "image.hpp"

const char* subroutine_names[] = 
{
    "ewa_SW",
    "mipmap_filter_SW",
    "textureGrad_HW",
    "area_distortion_HW",
    "nearest_filter_HW",
    "linear_filter_HW",
    "mipmap_filter_HW",
    "anisotropic_filter_HW",
    "linear_filter_SW",
    "bicubic_filter_SW",
    "ewa2tex_SW",
    "ewa4tex_SW",
    "approximate_ewa_SW",
    "approximate_ewa_spatial_SW",
    "approximate_ewa_temporal_SW",
    "lodError_SW",
    "anisotropyLevel_SW",
    "mipLevel_SW"
};

struct demo_window_t : public imgui_window_t
{
    bool pause = false;
    const unsigned int SAMPLER_MODE_COUNT = 4;

    unsigned int TEXTURE_COUNT;
    unsigned int texture_index;
    GLuint* textures;

    unsigned int FILTER_MODE_COUNT;
    unsigned int left_filter_mode_index = 0;
    unsigned int right_filter_mode_index = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);        
    }

    void set_texture(int texture_index)
    {
        int tex_id = textures[texture_index];
        for(unsigned int i = 0; i < SAMPLER_MODE_COUNT; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, tex_id);
        }
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if (action != GLFW_RELEASE)
            return;

        if (key == GLFW_KEY_ENTER)
        {
            pause = !pause;
        }
        else if (key == GLFW_KEY_SPACE)
        {
            texture_index = (texture_index + 1) % TEXTURE_COUNT;
            set_texture(texture_index);
        }
        else if (key == GLFW_KEY_KP_1)
        {
            left_filter_mode_index = (left_filter_mode_index + FILTER_MODE_COUNT - 1) % FILTER_MODE_COUNT;
        }
        else if (key == GLFW_KEY_KP_7)
        {
            left_filter_mode_index = (left_filter_mode_index + 1) % FILTER_MODE_COUNT;
        }
        else if (key == GLFW_KEY_KP_3)
        {
            right_filter_mode_index = (right_filter_mode_index + FILTER_MODE_COUNT - 1) % FILTER_MODE_COUNT;
        }
        else if (key == GLFW_KEY_KP_9)
        {
            right_filter_mode_index = (right_filter_mode_index + 1) % FILTER_MODE_COUNT;
        }
    }

    void on_mouse_move() override
    {
    }


    void update_ui() override
    {
        //===============================================================================================================================================================================================================
        // show a simple fps window.
        //===============================================================================================================================================================================================================
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Left  :: %s", subroutine_names[left_filter_mode_index]);
        ImGui::Text("Right :: %s", subroutine_names[right_filter_mode_index]);

/*
        ImGui::Text("HW linear filtering mode");
        ImGui::Text("HW mipmap filtering mode");
        ImGui::Text("HW anisotropic filtering mode");
        ImGui::Text("HW anisotropic filtering mode");
        ImGui::Text("HW anisotropic filtering mode");

        ImGui::Text("SW elliptical filtering mode");   // filtering_mode = 2
        ImGui::Text("SW elliptical linear filtering mode");   // filtering_mode = 3
        ImGui::Text("SW elliptical bilinear filtering mode");   // filtering_mode = 4
        ImGui::Text("SW approximate elliptical filtering mode");   // filtering_mode = 5
        ImGui::Text("SW spatial elliptical filtering mode");   // filtering_mode = 6
        ImGui::Text("SW temporal elliptical filtering mode");   // filtering_mode = 7

        ImGui::Text("Mip-map selection absolute deviation x2 (black = zero error)");   // filtering_mode = 8
        ImGui::Text("Anisotropy level (pure red > 16)");   // filtering_mode = 9
        ImGui::Text("Mip-Map level visualization");   // filtering_mode = 9
*/
    }
};

void writeppm(char* file_name, int res_x, int res_y, char* rgb_data)
{
    FILE* f = fopen(file_name, "wb");
    fprintf(f, "P6 %d %d 255\n", res_y, res_x);

    for(int i = 0; i < res_y; i++)
        for(int j = 0; j < res_x; j++)
        {
            putc(rgb_data[3 * (i * res_x + j) + 0], f);
            putc(rgb_data[3 * (i * res_x + j) + 1], f);
            putc(rgb_data[3 * (i * res_x + j) + 2], f);
        }

    fclose(f);
}

void screenshot(int frame, int res_x, int res_y)
{
    int size = 3 * res_x * res_y;
    char* rgb_data = (char*) malloc(size);

    glFinish();
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, res_x, res_y, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
    char file_name[128];

    sprintf(file_name,"shot%u.ppm", frame);
    writeppm(file_name, res_x, res_y, rgb_data);    
}

template<typename axial_func> vao_t surface_of_revolution(float z0, float z1, int m, int n)
{
    axial_func func;

    int m2 = m + m;
    GLuint V = m2 * (n + 1);
    vertex_pnt2_t* vertices = (vertex_pnt2_t*) malloc(V * sizeof(vertex_pnt2_t));

    float z = z0;
    float delta_z = (z1 - z0) / n;

    int index = 0;

    const float delta = 1.0f / 256.0f;
    const float inv_delta = 2.0f / delta;

    for (int v = 0; v <= n; ++v)
    {
        float R = func(z);
        float dRdz = inv_delta * (func(z + delta) - func(z - delta));
        float inv_norm = 1.0f / glm::sqrt(1.0f + dRdz * dRdz);

        for (int u = 0; u < m2; ++u)
        {
            int q = (u <= m) ? u : m2 - u;

            float theta = constants::pi * u / m;
            float cs = glm::cos(theta);
            float sn = glm::sin(theta);


            vertices[index].position = glm::vec3(R * cs, R * sn, z);
            vertices[index].normal = inv_norm * glm::vec3(-cs, -sn, dRdz);
            vertices[index].uv = glm::vec2(z, float(q) / m);
            ++index;
        }
        z += delta_z;
    }
    
    //===================================================================================================================================================================================================================
    // Create VAO and fill the attribute buffer                                                                                                                                                                          
    //===================================================================================================================================================================================================================
    vao_t vao;
    glGenVertexArrays(1, &vao.id);
    glBindVertexArray(vao.id);
    vao.vbo.init(vertices, V);

    //===================================================================================================================================================================================================================
    // Fill the index buffer                                                                                                                                                                                             
    //===================================================================================================================================================================================================================
    free(vertices);

    index = 0;
    GLuint index_count = n * (4 * m + 3);
    GLuint* indices = (GLuint*) malloc (index_count * sizeof(GLuint));
    GLuint p = 0;
    GLuint q = m2;
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v < m2; ++v)
        {
            indices[index++] = q;
            indices[index++] = p;
            ++p; ++q;
        }
        indices[index++] = q - m2;
        indices[index++] = p - m2;
        indices[index++] = -1;
    }
    vao.ibo.init(GL_TRIANGLE_STRIP, indices, index_count);

    free(indices);

    return vao;
}


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
    // shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t filtering(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/filtering.vs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/filtering.fs"));

    filtering.enable();

    uniform_t uni_pv_matrix = filtering["projection_view_matrix"];
    uniform_t uni_frame = filtering["frame"];

    filtering["nearest_mode_tex"] = 0;
    filtering["linear_mode_tex"] = 1;
    filtering["mipmap_mode_tex"] = 2;
    filtering["anisotropic_mode_tex"] = 3;

    const GLuint SUBROUTINE_COUNT = sizeof(subroutine_names) / sizeof(char*);
    GLuint subroutine_index[SUBROUTINE_COUNT];
    for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        subroutine_index[i] = filtering.subroutine_index(GL_FRAGMENT_SHADER, subroutine_names[i]);

    window.FILTER_MODE_COUNT = SUBROUTINE_COUNT;

    //===================================================================================================================================================================================================================
    // create models for texture filtering tests :
    //  1. rotating cube
    //  2. cylinder
    //===================================================================================================================================================================================================================
    const float cube_size = 2.0;
    polyhedron cube;
    cube.regular_pnt2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, cube_size, false);

    struct hyperbola
    {
        float operator () (float z)
        { 
            return 5.0 / z;
        }
    };

    vao_t hyperboloid = surface_of_revolution<hyperbola>(1.0, 20.0, 64, 64);

    //===================================================================================================================================================================================================================
    // load test textures
    //===================================================================================================================================================================================================================
    const int TEXTURE_COUNT = 5;
    char tex_name[128];

    glActiveTexture(GL_TEXTURE0);
    GLuint tex_id[TEXTURE_COUNT];

    for (int t = 0; t < TEXTURE_COUNT; ++t)
    {
        sprintf(tex_name, "res/test_tex%u.png", t);
        tex_id[t] = image::png::texture2d(tex_name, 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, false);
    }

    window.TEXTURE_COUNT = TEXTURE_COUNT;
    window.textures = tex_id;
    window.set_texture(0);

    //===================================================================================================================================================================================================================
    // initialize samplers and bind them to the texture units 0,1,2 and 3
    //  -- TEXTURE UNIT 0 : plain nearest filtering mode 
    //  -- TEXTURE UNIT 1 : (bi)linear filtering mode 
    //  -- TEXTURE UNIT 2 : (bi)linear filtering mode with mipmaps
    //  -- TEXTURE UNIT 3 : hardware anisotropic filtering
    //===================================================================================================================================================================================================================
    sampler_t nearest_sampler(GL_NEAREST, GL_NEAREST, GL_REPEAT);
    nearest_sampler.bind(0);

    sampler_t linear_sampler(GL_LINEAR, GL_LINEAR, GL_REPEAT);
    linear_sampler.bind(1);

    sampler_t mipmap_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    mipmap_sampler.bind(2);

    GLfloat max_af_level;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_af_level);
    debug_msg("Maximal HW supported anisotropy level is %.1f", max_af_level);

    sampler_t anisotropy_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    anisotropy_sampler.set_max_af_level(max_af_level);
    anisotropy_sampler.bind(3);

    glm::mat4 projection_matrix = glm::infinitePerspective(constants::two_pi / 6.0f, (float) (0.5 * window.aspect()), 0.5f);
    float t = 0;

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);
    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        //===============================================================================================================================================================================================================
        // render scene on two halfs of the screen
        //===============================================================================================================================================================================================================
        int x = window.res_x;
        int half_x = x / 2;
        int y = window.res_y;

        if (!window.pause)
            t += window.frame_dt;

        glm::vec2 r0 = glm::vec2(glm::cos(0.517f * t), glm::sin(0.517f * t));
        glm::vec2 r1 = glm::vec2(glm::cos(0.211f * t), glm::sin(0.211f * t));

    #if 0
        glm::vec3 eye = 7.0f * glm::vec3(r0.x * r1.x, r0.x * r1.y, r0.y);
    #endif
        glm::vec3 eye = 3.5f * glm::vec3(r0.x * r1.x, r0.x * r1.y, -2.5f + r0.y);

        glm::mat4 view_matrix = glm::lookAt(eye, glm::vec3(0.0), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

        filtering.enable();

        uni_pv_matrix = projection_view_matrix;
        uni_frame = (int) window.frame;

        glViewport(0, 0, half_x, y);
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index[window.left_filter_mode_index]);
//        cube.render();
        hyperboloid.render();
 
        glViewport(half_x, 0, half_x, y);
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index[window.right_filter_mode_index]);
//        cube.render();
        hyperboloid.render();


        // saveFrameBuffer();
        //===============================================================================================================================================================================================================
        // After end_frame call ::
        //  - GL_DEPTH_TEST is disabled
        //  - GL_CULL_FACE is disabled
        //  - GL_SCISSOR_TEST is enabled
        //  - GL_BLEND is enabled -- blending mode GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA with blending function GL_FUNC_ADD
        //  - VAO binding is destroyed
        //===============================================================================================================================================================================================================
        window.end_frame();
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        //glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}