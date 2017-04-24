//========================================================================================================================================================================================================================
// DEMO 042: VAO loader
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
#include <glm/gtc/quaternion.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vao.hpp"
#include "vertex_types.hpp"

//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const double linear_velocity = 0.66f;
const double angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================

void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);  
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity); 
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity);
};

void mouse_handler(double dx, double dy, double duration)
{
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, dy, angle);
    };
};


int main()
{


    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("VAO Loader", 8, 3, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program blinn_phong(glsl_shader(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();

    GLint uniform_projection_view_matrix = blinn_phong.uniform_id("projection_view_matrix");
    GLint uniform_camera_ws              = blinn_phong.uniform_id("camera_ws");
    GLint uniform_light_ws               = blinn_phong.uniform_id("light_ws");
    GLint uniform_Ka                     = blinn_phong.uniform_id("Ka");
    GLint uniform_Kd                     = blinn_phong.uniform_id("Kd");
    GLint uniform_Ks                     = blinn_phong.uniform_id("Ks");
    GLint uniform_Ns                     = blinn_phong.uniform_id("Ns");

    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glDisable(GL_CULL_FACE);

    const int MODEL_COUNT = 15;

    const char* vao_list[MODEL_COUNT] = 
    {
        "models\\ashtray.vao",
        "models\\azog.vao",
        "models\\bust.vao",

        "models\\chubby_girl.vao",
        "models\\demon.vao",
        "models\\dragon.vao",

        "models\\female_01.vao",
        "models\\female_02.vao",
        "models\\female_03.vao",

        "models\\king_kong_bust.vao",
        "models\\king_kong_solid.vao", 
        "models\\predator.vao",

        "models\\predator_solid.vao",
        "models\\skull.vao",
        "models\\trefoil.vao"
    };

    const float light_radius = 75.0f;

    vao models[MODEL_COUNT];


    for (int i = 0; i < MODEL_COUNT; ++i)
    {
        vao& model = models[i];
        debug_msg("Loading %s ... \n", vao_list[i]);
        model.init(vao_list[i]);

        debug_msg("VAO Loaded :: \n\tvertex_count = %d. \n\tvertex_layout = %d. \n\tindex_type = %d. \n\tprimitive_mode = %d. \n\tindex_count = %d\n\n\n", 
                  model.params.vertex_count, model.params.vertex_layout, model.params.index_type, model.params.primitive_mode, model.params.index_count);
    };

    debug_msg("Done ... \nGL_UNSIGNED_INT = %d.\nGL_TRIANGLES = %d", GL_UNSIGNED_INT, GL_TRIANGLES);


    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = camera.projection_view_matrix();

        float time = window.time();
        glm::vec3 light_ws = glm::vec3(200.0f + light_radius * cos(time), 350.0f, light_radius * sin(time));
        glm::vec3 camera_ws = camera.position();

        glUniformMatrix4fv(uniform_projection_view_matrix, 1, GL_FALSE, glm::value_ptr(projection_view_matrix));

        glUniform3fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
        glUniform3fv(uniform_camera_ws, 1, glm::value_ptr(camera_ws));

        glm::vec3 Ka = glm::vec3(0.17f);
        glm::vec3 Kd = glm::vec3(0.50f);
        glm::vec3 Ks = glm::vec3(0.33f);
        float Ns = 20.0f;
        
        glUniform3fv(uniform_Ka, 1, glm::value_ptr(Ka));
        glUniform3fv(uniform_Kd, 1, glm::value_ptr(Kd));
        glUniform3fv(uniform_Ks, 1, glm::value_ptr(Ks));
        glUniform1f (uniform_Ns, Ns);

        for (int i = 0; i < MODEL_COUNT; ++i) models[i].render();

        window.swap_buffers();
        window.poll_events();
    };

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    return 0;
}                               