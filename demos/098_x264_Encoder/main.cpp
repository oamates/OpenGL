//========================================================================================================================================================================================================================
// DEMO 098 : x264 video player
//========================================================================================================================================================================================================================
#include <cstdlib>
#include <cstdio>

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
#include <glm/glm.hpp>

#include "log.hpp"
#include "glfw_window.hpp"
#include "gl_aux.hpp"
#include "camera.hpp"
#include "shader.hpp"

#include "x264.hpp"
#include "yuv420p.hpp"

struct demo_window_t : public glfw_window_t
{
    x264_decoder* decoder_ptr = 0;
    yuv420p_player* player_ptr = 0;

    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
        { gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO); }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================

    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if(action != GLFW_RELEASE) return;

        if (key == GLFW_KEY_ESCAPE) 
            set_should_close(GL_TRUE);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }

    virtual void on_resize(int width, int height) override
    {
        if(player_ptr) 
            player_ptr->resize(width, height);
    }
};

static bool playback_initialized = false;
static yuv420p_player* player_ptr = 0;

void frame_callback(AVFrame* frame, AVPacket* pkt, void* user)
{
    if(!playback_initialized)
    {
        if(frame->format != AV_PIX_FMT_YUV420P)
            exit_msg("This code only support yuv420P data.\n");

        player_ptr->setup(frame->width, frame->height);
        playback_initialized = true;
    }
    if(player_ptr)
    {
        player_ptr->setYPixels(frame->data[0], frame->linesize[0]);
        player_ptr->setUPixels(frame->data[1], frame->linesize[1]);
        player_ptr->setVPixels(frame->data[2], frame->linesize[2]);
    }
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================

int main(int argc, char *argv[])
{

    if (argc < 2)
        exit_msg("Usage : %s <filename> ", argv[0]);

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("x264 GLSL video player", 4, 3, 3, 1920, 1080, true);
 
    x264_decoder decoder(frame_callback, 0);
    yuv420p_player player;

    window.player_ptr = &player;
    window.decoder_ptr = &decoder;

    player_ptr = &player;
  
    if(!decoder.load(argv[1], 30.0f)) 
        exit_msg("Cannot load decoder. Exiting ...");

    

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        decoder.readFrame();
        player.draw(0, 0, window.res_x, window.res_y);

        window.swap_buffers();
        glfw::poll_events();
    }
     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}