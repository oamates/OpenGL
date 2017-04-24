//========================================================================================================================================================================================================================
// DEMO 041: Object loader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "obj.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
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
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};


int main(int argc, char* argv[])
{
    //===================================================================================================================================================================================================================
    // extract filename and directory to work with
    //===================================================================================================================================================================================================================
    if (argc < 2)
    {
        printf("\nUsage : %s <filename>\n\n", argv[0]);
        return 0;
    }

    std::string filename(argv[1]);
    std::string dir;
    const size_t slash_idx = filename.rfind('\\');
    if (std::string::npos != slash_idx)
        dir = filename.substr(0, slash_idx);

    debug_msg("Loading model = %s", filename.c_str());
    debug_msg("Directory = %s", dir.c_str());


    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("OBJ Loader", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program_t blinn_phong(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blinn_phong.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blinn_phong.fs"));

    blinn_phong.enable();
    uniform_t camera_ws = blinn_phong["camera_ws"];
    uniform_t light_ws  = blinn_phong["light_ws"];
    uniform_t Ka        = blinn_phong["Ka"];
    uniform_t Kd        = blinn_phong["Kd"];
    uniform_t Ks        = blinn_phong["Ks"];
    uniform_t Ns        = blinn_phong["Ns"];
    uniform_t d         = blinn_phong["d"];


    //===================================================================================================================================================================================================================
    // standard Blinn-Phong shader : model includes texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program_t blinn_phong_tex(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/blinn_phong_tex.vs"),
                                   glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/blinn_phong_tex.fs"));

    blinn_phong_tex.enable();
    blinn_phong_tex["map_Ka"]   = 0;             // ambient texture binding point 0                      
    blinn_phong_tex["map_Kd"]   = 1;             // diffuse texture                       
    blinn_phong_tex["map_Ks"]   = 2;             // specular texture                      
    blinn_phong_tex["map_Ns"]   = 3;             // specular shininess texture            
    blinn_phong_tex["map_bump"] = 4;             // bump texture                          
    blinn_phong_tex["map_d"]    = 5;             // mask texture                          

    uniform_t tex_camera_ws = blinn_phong_tex["camera_ws"];
    uniform_t tex_light_ws  = blinn_phong_tex["light_ws"];
    uniform_t tex_Ka        = blinn_phong_tex["Ka"];
    uniform_t tex_Kd        = blinn_phong_tex["Kd"];
    uniform_t tex_Ks        = blinn_phong_tex["Ks"];
    uniform_t tex_Ns        = blinn_phong_tex["Ns"];
    uniform_t tex_d         = blinn_phong_tex["d"];
    uniform_t tex_bm        = blinn_phong_tex["bm"];
    uniform_t tex_flags     = blinn_phong_tex["flags"];


    //===================================================================================================================================================================================================================
    // Global OpenGL state
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glEnable(GL_CULL_FACE);

    //===============================================================================================================================
    // create UBO for common uniforms, bind it to UBO target 0 and connect with shader uniform blocks
    //===============================================================================================================================
    struct
    {
        glm::mat4 projection_view_matrix;
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 camera_matrix;
    } matrices;

    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);

    blinn_phong.bind_ubo("matrices", 0);
    blinn_phong_tex.bind_ubo("matrices", 0);

    //===============================================================================================================================
    // Load the model
    //===============================================================================================================================
    model mdl(filename, dir);

    //===================================================================================================================================================================================================================
    // Log the materials information
    //===================================================================================================================================================================================================================
    for (size_t i = 0; i < mdl.materials.size(); i++)
    {
        material_t& material = mdl.materials[i];
        debug_msg("material #%u.", (unsigned int) i);
        print_material(material);
    }

    if (mdl.textured)
        blinn_phong_tex.enable();
    else
        blinn_phong.enable();

    const float light_radius = 7500.0f;

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();

        matrices.projection_matrix = window.camera.projection_matrix;
        matrices.view_matrix = window.camera.view_matrix;
        matrices.projection_view_matrix = window.camera.projection_matrix * window.camera.view_matrix;
        matrices.camera_matrix = glm::inverse(window.camera.view_matrix);

        float time = glfw::time();
        glm::vec3 _light_ws = glm::vec3(light_radius * cos(time), 3500.0f * sin(0.577 * time), light_radius * sin(time));
        glm::vec3 _camera_ws = window.camera.position();

        if (mdl.textured)
        {
            tex_light_ws = _light_ws;
            tex_camera_ws = _camera_ws;
        }
        else
        {
            light_ws = _light_ws;
            camera_ws = _camera_ws;
        }

        //===============================================================================================================================================================================================================
        // Write common shader data to shared uniform buffer
        //===============================================================================================================================================================================================================
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
        GLvoid* buf_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        memcpy(buf_ptr, &matrices, sizeof(matrices));
        glUnmapBuffer(GL_UNIFORM_BUFFER);

//        glBindVertexArray(mdl.vao_id);

        unsigned int index = 0;
        for (size_t i = 0; i < mdl.material_index.size(); ++i)
        {
            int material_id = mdl.material_index[i].first; 

            if (mdl.textured)
            {
                if (material_id == -1)
                {
                    tex_Ka = glm::vec3(0.17f);
                    tex_Kd = glm::vec3(0.50f);
                    tex_Ks = glm::vec3(0.33f);
                    tex_Ns = 20.0f;
                    tex_d = 1.0f;
                    tex_flags = 0;
                }
                else
                {
                    material_t& material = mdl.materials[material_id];

                    if (!material.map_Ka.empty())
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_Ka]);
                    }
                    
                    if (!material.map_Kd.empty())
                    {
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_Kd]);
                    }
                    
                    if (!material.map_Ks.empty())
                    {
                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_Ks]);
                    }
                    
                    if (!material.map_Ns.empty())
                    {
                        glActiveTexture(GL_TEXTURE3);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_Ns]);
                    }
                    
                    if (!material.map_bump.empty())
                    {
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_bump]);
                    }
                    
                    if (!material.map_d.empty())
                    {
                        glActiveTexture(GL_TEXTURE5);
                        glBindTexture(GL_TEXTURE_2D, mdl.textures[material.map_d]);
                    }
                    
                    tex_Ka = material.Ka;
                    tex_Kd = material.Kd;
                    tex_Ks = material.Ks;
                    tex_Ns = material.Ns;
                    tex_d  = material.d;
                    tex_bm = material.bm;
                    tex_flags = (int) material.flags;
                }
            }
            else
            {
                if (material_id == -1)
                {
                    Ka = glm::vec3(0.17f);
                    Kd = glm::vec3(0.50f);
                    Ks = glm::vec3(0.33f);
                    Ns = 20.0f;
                    d = 1.0f;
                }
                else
                {
                    material_t& material = mdl.materials[material_id];
                    Ka = material.Ka;
                    Kd = material.Kd;
                    Ks = material.Ks;
                    Ns = material.Ns;
                    d  = material.d;
                }
            }

            GLuint last_index = mdl.material_index[i].second; 
            
            mdl.vertex_array.render(last_index - index, (const GLvoid *) (sizeof(GLuint) * index));

            index = last_index;                                
        }
        
        window.new_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    glfw::terminate();
    return 0;
}                               