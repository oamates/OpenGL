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

/*
int filtering_mode = 2;
int render_scene = 1;
int split_screen = 1;
int num_probes = 6;
float num_texels = 1.0;
float glob_speed = 0.001;
float filter_width = 1.0;
float filter_sharpness = 2.0;
int vsync = 1;
int showHelp = 0;
int showOSD = 1;
int supports_gl4 = 0;
int max_anisotropy = 16;
int uniform_frame = 0;
int uniform_time = 0;
float fps = 60;
float global_time = 0;

*/

struct demo_window_t : public imgui_window_t
{
    unsigned int TEXTURE_COUNT;
    unsigned int texture_index;
    GLuint* textures;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);        
    }

    void set_texture(int texture_index)
    {
        for(int i = 0; i < 4; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[texture_index]);
        }
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if ((key == GLFW_KEY_SPACE) && (action == GLFW_RELEASE))
        {
            texture_index = (texture_index + 1) % TEXTURE_COUNT;
            set_texture(texture_index);
        }
    }
    /*
    switch (key) {
    case 27:
        exit(0);
        break;
    case '1':
        printf("hardware texture filtering\n");
        filtering_mode = 1;
        break;
    case '2':
        printf("high quality elliptical texture filtering\n");
        filtering_mode = 2;
        break;
    case '3':
        printf("high quality elliptical texture filtering (linear)\n");
        filtering_mode = 3;
        break;
    case '4':
        printf("high quality elliptical texture filtering (bilinear)\n");
        filtering_mode = 4;
        break;
    case '5':
        printf("approximated elliptical texture filtering\n");
        filtering_mode = 5;
        break;
    case '6':
        printf("spatial elliptical texture filtering\n");
        filtering_mode = 6;
        break;
    case '7':
        printf("temporal elliptical texture filtering\n");
        filtering_mode = 7;
        break;
    case '8':
        printf("absolute lod error display\n");
        filtering_mode = 8;
        num_texels = 1;
        max_anisotropy = 16; //just to match NVIDIA hardware
        break;
    case '9':
        printf("anisotropy level\n");
        filtering_mode = 9;
        break;
    case '0':
        printf("MIP-MAP level\n");
        filtering_mode = 0;
        break;
    case 'a':
    case 'A':
        toggle(render_scene);
        if(render_scene==1){
            filter_width=1.0;
            filter_sharpness=2.0f;
            num_probes=6;
            num_texels = 1;
            max_anisotropy = 16;
        }
        else{
            filter_width=1.8f;
            filter_sharpness=5.5;
            num_probes=10;
            num_texels = 1;
            max_anisotropy = 16;
        }
        break;
    case 's':
    case 'S':
        printf("toggle split screen\n");
        toggle(split_screen);
        break;
    case '=':
    case '+':
        glob_speed+=0.001f;
        printf("speed: %f\n", glob_speed);
        break;
    case '-':
        glob_speed-=0.001f;
        printf("speed: %f\n", glob_speed);
        break;
    case 'e':
        num_probes-=2;
        printf("num_probes: %d\n", num_probes);
        break;
    case 'E':
        num_probes+=2;
        printf("num_probes: %d\n", num_probes);
        break;
    case 'q':
        filter_width-=0.1f;
        printf("speed: %f\n", filter_width);
        break;
    case 'Q':
        filter_width+=0.1f;
        printf("filter_width: %f\n", filter_width);
        break;
    case 'r':
        num_texels-=0.1f;
        break;
    case 'R':
        num_texels+=0.1f;
        break;
    case 't':
        max_anisotropy-=1;
        break;
    case 'T':
        max_anisotropy+=1;
        break;
    case 'h':
    case 'H':
        toggle(showHelp);
        break;
    case 'd':
    case 'D':
        toggle(showOSD);
        break;
    case 'w':
        filter_sharpness-=0.1f;
        break;
    case 'W':
        filter_sharpness+=0.1f;
        break;
    case 'v':
    case 'V':
        printf("toggle vertical sync\n");
        toggle(vsync);
        if(vsync==0){
            wglSwapIntervalEXT(0);
        }
        else{
            wglSwapIntervalEXT(1);
        }
        break;
    }

    }
*/
    /*
    void on_mouse_move() override
    {
    }
    */

    void update_ui() override
    {
        //===============================================================================================================================================================================================================
        // show a simple fps window.
        //===============================================================================================================================================================================================================
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

/*
        if(showOSD)
        {
            overlay->Begin();

            DisplayFilterType();

            
            if(frameCounter==timingNumFrames){
                int timeInterval=glutGet(GLUT_ELAPSED_TIME)-timeCounter;
                fps = float(timingNumFrames)*1000.0f/float(timeInterval);
                timeCounter=glutGet(GLUT_ELAPSED_TIME);
                frameCounter=0;
                
            }   

            static char tmpString[128];
            sprintf(tmpString,"fps: %.1f",fps);
            overlay->Print(RESX-130,10,1,tmpString);

            if(filtering_mode<9)
                DisplayFilterInfo();

            if(showHelp)
                DisplayHelp();
        }
*/

    }

};



//write a ppm file to disk
/*
void writeppm(char* file_name, int resx, int resy, char* raster)
{

    FILE * out = fopen(file_name, "wb");
    fprintf(out, "P6 %d %d 255\n", resy, resx);

    for(int i=0; i<resy; i++)
        for(int j=0; j<resx; j++)
        {
            putc(raster[3 * (i * resx + j) + 0], out);
            putc(raster[3 * (i * resx + j) + 1], out);
            putc(raster[3 * (i * resx + j) + 2], out);
        }

    fclose(out);
}
*/

//    glFinish();
//    glReadBuffer( GL_BACK );
//    glReadPixels( 0, 0, RESX, RESY, GL_RGB, GL_UNSIGNED_BYTE, fb_mem);
//    sprintf(file_name,"capture%05d.ppm", frameCounter);
//    writeppm(file_name, RESX, RESY, fb_mem);
    // Shader dynamic macro setting
/*
    resetShadersGlobalMacros();
    setShadersGlobalMacro("RENDER_SCENE", render_scene);
    setShadersGlobalMacro("FILTERING_MODE", filtering_mode);
    setShadersGlobalMacro("SPLIT_SCREEN", split_screen);
    setShadersGlobalMacro("USE_GL4", supports_gl4);
    setShadersGlobalMacro("SPEED", glob_speed);
    setShadersGlobalMacro("FILTER_WIDTH", filter_width);
    setShadersGlobalMacro("NUM_PROBES", num_probes);
    setShadersGlobalMacro("FILTER_SHARPNESS", filter_sharpness);
    setShadersGlobalMacro("TEXELS_PER_PIXEL", num_texels);
    setShadersGlobalMacro("MAX_ECCENTRICITY", max_anisotropy);
*/
/*
    switch (filtering_mode){
    case 1:
        sprintf(tmpString,"Hardware Filtering");
        break;
    case 2:
        sprintf(tmpString,"Elliptical Filtering");
        break;
    case 3:
        sprintf(tmpString,"Elliptical Filtering (2-tex)");
        break;
    case 4:
        sprintf(tmpString,"Elliptical Filtering (4-tex)");
        break;
    case 5:
        sprintf(tmpString,"Approximated Elliptical Filter");
        break;
    case 6:
        sprintf(tmpString,"Spatial Elliptical Filtering");
        break;
    case 7:
        sprintf(tmpString,"Temporal Elliptical Filtering");
        break;
    case 8:
        if(supports_gl4)
            sprintf(tmpString,"Mip-map selection absolute deviation x2 (black = zero error)");
        else
            sprintf(tmpString,"Error: Requires an OpenGL 4 context");
        break;
    case 9:
        sprintf(tmpString,"Anisotropy level (pure red > 16)");
        break;
    case 0:
        sprintf(tmpString,"Mip-Map level visualization");
        break;
    };
*/

template<typename axial_func> vao_t surface_of_revolution(float z0, float z1, int m, int n)
{
    axial_func func;

    GLuint V = (2 * m + 1) * (n + 1);
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
        float inv_norm = 1.0f / glm::sqrt(dRdz * dRdz);

        for (int u = 0; u <= 2 * m; ++u)
        {
            int q = (u <= m) ? u : 2 * m - u;

            float theta = constants::pi * u / m;
            float cs = glm::cos(theta);
            float sn = glm::sin(theta);


            vertices[index].position = glm::vec3(R * cs, R * sn, z);
            vertices[index].normal = inv_norm * glm::vec3(-cs, -sn, dRdz);
            vertices[index].uv = glm::vec2(z, float(q) / m);
            ++index;
        }
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
    GLuint q = 2 * m;
    GLuint p = 0;
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v <= 2 * m; ++v)
        {
            indices[index++] = q;
            indices[index++] = p;
            ++p; ++q;
        }
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

    const char* subroutine_names[] = 
    {
        "nearest_filter_HW",
        "linear_filter_HW",
        "mipmap_filter_HW",
        "anisotropic_filter_HW"
    };

    const GLuint SUBROUTINE_COUNT = sizeof(subroutine_names) / sizeof(char*);
    GLuint subroutine_index[SUBROUTINE_COUNT];
    for (int i = 0; i < SUBROUTINE_COUNT; ++i)
        subroutine_index[i] = filtering.subroutine_index(GL_FRAGMENT_SHADER, subroutine_names[i]);

    //===================================================================================================================================================================================================================
    // create models for texture filtering tests :
    //  1. rotating cube
    //  2. cylinder
    //===================================================================================================================================================================================================================
    const float cube_size = 2.0;
    polyhedron cube;
    cube.regular_pnt2_vao(8, 6, plato::cube::vertices, plato::cube::normals, plato::cube::faces, cube_size, true);

    struct hyperbola
    {
        float operator () (float z)
        {
            return 10.0 / z;
        }
    };

    vao_t hyperboloid = surface_of_revolution<hyperbola>(0.5, 20.0, 64, 64);

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

    sampler_t anisotropy_sampler(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
    GLfloat max_af_level;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_af_level);
    anisotropy_sampler.set_max_af_level(max_af_level);
    anisotropy_sampler.bind(3);

    //===================================================================================================================================================================================================================
    // GL_FRAGMENT_SHADER subroutines
    //===================================================================================================================================================================================================================


    //glEnable(GL_CULL_FACE);
    glm::mat4 projection_matrix = glm::infinitePerspective (constants::two_pi / 6.0f, (float) (0.5 * window.aspect()), 0.5f);

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
        float time = window.frame_ts;
        int x = window.res_x;
        int half_x = x / 2;
        int y = window.res_y;


        float cs0 = glm::cos(0.517f * time);
        float sn0 = glm::sin(0.517f * time);
        float cs1 = glm::cos(0.211f * time);
        float sn1 = glm::sin(0.211f * time);

        glm::vec3 eye = 7.0f * glm::vec3(cs0 * cs1, cs0 * sn1, sn0);

        glm::mat4 view_matrix = glm::lookAt(eye, glm::vec3(0.0), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection_view_matrix = projection_matrix * view_matrix;

        filtering.enable();

        uni_pv_matrix = projection_view_matrix;

        glViewport(0, 0, half_x, y);
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index[0]);
        cube.render();

 
        glViewport(half_x, 0, half_x, y);
        uniform_t::subroutine(GL_FRAGMENT_SHADER, &subroutine_index[2]);
        cube.render();


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