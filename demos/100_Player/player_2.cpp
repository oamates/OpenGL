//========================================================================================================================================================================================================================
// DEMO 100.2: OpenGL Video Player
//========================================================================================================================================================================================================================
// check out :: https://www.codeproject.com/tips/111468/ffmpeg-tutorial
// check out :: https://habrahabr.ru/post/138426/
// check out :: https://habrahabr.ru/post/137793/
#include <cstdio>

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include "log.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"

int main(int argc, char *argv[])
{    
    if(argc < 2)
        exit_msg("Usage: player_2 <filename>\n");
    
    av_register_all();                                                                                      // Register all formats and codecs
    
    AVFormatContext *pFormatCtx = 0;
    if(avformat_open_input(&pFormatCtx, argv[1], 0, 0) != 0)                                                // Open an input stream and read the header.
        exit_msg("Cannot open video file : %s", argv[1]);
    
    if(avformat_find_stream_info(pFormatCtx, 0) < 0)                                                        // Read packets of a media file to get stream information.
        exit_msg("Couldn't find stream information.\n");
    
    av_dump_format(pFormatCtx, 0, argv[1], 0);                                                              // Print detailed information about the input format
    
    int videoStream = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; i++)                                                         // Find the first video stream
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }

    if (videoStream == -1)                                                                                  // No video stream was found
        exit_msg("Source file does not contain video stream"); 
    
    AVCodecParameters *pCodecParameters = pFormatCtx->streams[videoStream]->codecpar;                       // Get a pointer to the codec context for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);                                     // Find the decoder for the video stream
    if(pCodec == 0)
        exit_msg("Cannot find appropriate codec!\n");
    
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);                                             // Allocate an AVCodecContext and set its fields to default values. 
    if(avcodec_parameters_to_context(pCodecCtx, pCodecParameters) != 0)
        exit_msg("Couldn't copy codec context");
    
    if(avcodec_open2(pCodecCtx, pCodec, 0) < 0)                                                             // Initialize the AVCodecContext to use the given AVCodec. 
        exit_msg("Could not open codec");
    
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, windowed
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    glfw_window_t window("YUV420P Video player", 4, 3, 3, 1280, 720, false /*, true */);
    gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);

    //===================================================================================================================================================================================================================
    // compile  shader
    //===================================================================================================================================================================================================================
    glsl_program_t glsl_video_player(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/yuv420p_conv.fs"));

    glsl_video_player.enable();
    glsl_video_player["y_texture"] = 0;
    glsl_video_player["u_texture"] = 1;
    glsl_video_player["v_texture"] = 2;

    //===================================================================================================================================================================================================================
    // create YUV textures
    //===================================================================================================================================================================================================================
    int res_x = pCodecCtx->width;
    int res_y = pCodecCtx->height;
    GLuint y_texture, u_texture, v_texture;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &y_texture);
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res_x, res_y, 0, GL_RED, GL_UNSIGNED_BYTE, 0); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &u_texture);
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res_x/2, res_y/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &v_texture);
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res_x/2, res_y/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //===================================================================================================================================================================================================================
    // Empty VAO
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glDisable(GL_DEPTH_TEST);

    AVPacket packet;
    AVFrame *pFrame = av_frame_alloc();                                                                     // Allocate video frame
    while((av_read_frame(pFormatCtx, &packet) >= 0) && !window.should_close())
    {
        if(packet.stream_index == videoStream)                                                              // Is this a packet from the video stream?
        {
            avcodec_send_packet(pCodecCtx, &packet);
            while(avcodec_receive_frame(pCodecCtx, pFrame) >= 0)                                            // Decode video frame
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, y_texture);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, pFrame->linesize[0]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, res_x, res_y, GL_RED, GL_UNSIGNED_BYTE, pFrame->data[0]);
 
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, u_texture);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, pFrame->linesize[1]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, res_x/2, res_y/2, GL_RED, GL_UNSIGNED_BYTE, pFrame->data[1]);
 
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, v_texture);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, pFrame->linesize[2]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, res_x/2, res_y/2, GL_RED, GL_UNSIGNED_BYTE, pFrame->data[2]);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                window.swap_buffers();
            }
        }
        av_packet_unref(&packet);                                                                           // Free the packet that was allocated by av_read_frame
        glfw::poll_events();
    }
    av_frame_free(&pFrame);                                                                                 // Free the YUV frame
    avcodec_close(pCodecCtx);                                                                               // Close the codec
    avcodec_parameters_free(&pCodecParameters);                                                             // Free an AVCodecParameters instance and everything associated with it and write NULL to the supplied pointer
    avformat_close_input(&pFormatCtx);                                                                      // Close the video file
    return 0;
}