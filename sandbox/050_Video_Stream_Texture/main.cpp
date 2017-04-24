//========================================================================================================================================================================================================================
// DEMO 050 : Streaming to texture
//========================================================================================================================================================================================================================

extern "C"
{ 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "glfw_window.hpp"
#include "log.hpp"
#include "H264.hpp"
#include "YUV420P.hpp"

H264_Decoder* decoder_ptr = 0;
YUV420P_Player* player_ptr = 0;
bool playback_initialized = false;

void log_callback(void* ptr, int level, const char* format, va_list vargs)
{
    FILE *output = fopen("debug.log", "a+");
    if (!output) return;
    fprintf(output, "LibAV debug :: level = %d :: message = ", level);
    vfprintf(output, format, vargs);
    fprintf(output, "\n");
    fclose(output);
}

void error_callback(int error, const char* description) 
{
    debug_msg("GLFW error: %s (%d)\n", description, error);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    if(action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) 
        glfwSetWindowShouldClose(win, GL_TRUE);
}

void initialize_playback(AVFrame* frame, AVPacket* pkt) 
{
    if(frame->format != AV_PIX_FMT_YUV420P)
        exit_msg("his code only support YUV420P data.");

    if(!player_ptr)
        exit_msg("player_ptr not found.");

    if(!player_ptr->setup(frame->width, frame->height))
        exit_msg("Cannot setup the yuv420 player.");
}

void frame_callback(AVFrame* frame, AVPacket* pkt, void* user)
{
    debug_msg(" ... frame_callback called ... ");
    if(!playback_initialized)
    {
        initialize_playback(frame, pkt);
        playback_initialized = true;
    }
    if(player_ptr)
    {
        player_ptr->setYPixels(frame->data[0], frame->linesize[0]);
        player_ptr->setUPixels(frame->data[1], frame->linesize[1]);
        player_ptr->setVPixels(frame->data[2], frame->linesize[2]);
    }
}

void resize_callback(GLFWwindow* window, int width, int height) 
{
    if(player_ptr)
        player_ptr->resize(width, height);
}

void button_callback(GLFWwindow* win, int bt, int action, int mods)
{
    double x, y;
    if(action == GLFW_PRESS || action == GLFW_REPEAT)
        glfwGetCursorPos(win, &x, &y);
}

void cursor_callback(GLFWwindow* win, double x, double y) { }
void char_callback(GLFWwindow* win, unsigned int key) { }

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main()
{
    av_log_set_level(AV_LOG_VERBOSE); 
    av_log_set_callback(log_callback);

    //===================================================================================================================================================================================================================
    // GLFW window creation
    // 8AA samples, OpenGL 3.3 context
    //===================================================================================================================================================================================================================
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) exit_msg("Failed to initialize GLFW.");

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                                                                    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int res_x = 1516;
    int res_y = 853;

    GLFWwindow* window = glfwCreateWindow(res_x, res_y, "LibAV :: Streaming to texture", 0, 0);

    if(!window)
    {
        glfwTerminate();
        exit_msg("Failed to open GLFW window. No open GL 3.3 support.");
    }

    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, button_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    debug_msg("GLFW initialization done ... ");

    //===================================================================================================================================================================================================================
    // GLEW library initialization
    //===================================================================================================================================================================================================================
    glewExperimental = true;                                                                                                // needed in core profile 
    GLenum result = glewInit();                                                                                             // initialise GLEW
    if (result != GLEW_OK) 
    {
        glfwTerminate();
        exit_msg("Failed to initialize GLEW : %s", glewGetErrorString(result));
    };
    debug_msg("GLEW library initialization done ... ");     

    debug_msg("GL_VENDOR = %s.", glGetString(GL_VENDOR));                                       
    debug_msg("GL_RENDERER = %s.", glGetString(GL_RENDERER));                                   
    debug_msg("GL_VERSION = %s.", glGetString(GL_VERSION));                                     
    debug_msg("GL_SHADING_LANGUAGE_VERSION = %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));   
    debug_msg("GL_EXTENSIONS = %s.", glGetString(GL_EXTENSIONS));

    //===================================================================================================================================================================================================================
    // OpenGL rendering parameters setup : background color -- dark blue
    //===================================================================================================================================================================================================================
	glClearColor(0.01f, 0.0f, 0.08f, 1.0f);



    H264_Decoder decoder(frame_callback, 0);
    YUV420P_Player player;
    player_ptr = &player;
    decoder_ptr = &decoder;


    if(!decoder.load("res/h264_sintel_trailer_1080p.mp4", 30.0f))
        debug_msg("Failed to load video file ... ");

    //===================================================================================================================================================================================================================
    // main program loop : just clear the buffer in a loop
    //===================================================================================================================================================================================================================

    debug_msg("Starting main loop");

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        decoder.readFrame();
        player.draw(0, 0, res_x, res_y);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfwTerminate();
  	return 0;
}