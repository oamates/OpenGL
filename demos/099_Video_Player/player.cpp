//========================================================================================================================================================================================================================
// DEMO 099: OpenGL Video Player
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "plato.hpp"
#include "polyhedron.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    float bump_intensity = 1.0f;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
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

        if (key == GLFW_KEY_KP_ADD) bump_intensity += 0.1f;
        if (key == GLFW_KEY_KP_SUBTRACT)
        {
            if (bump_intensity > 0.1f)
                bump_intensity -= 0.1f;
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

extern "C" 
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavfilter/avfilter.h>
    #include <libavdevice/avdevice.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <sys/time.h>
}

struct AppData
{
    int stream_idx;

    AVFormatContext *fmt_ctx;
    AVStream *video_stream;
    AVCodecContext *codec_ctx;
    AVCodec *decoder;
    AVPacket *packet;
    AVFrame *av_frame;
    AVFrame *gl_frame;
    struct SwsContext *conv_ctx;

    GLuint frame_tex;

    void initialize()                                                                                    // initialize the app data structure
    {
        fmt_ctx = 0;
        stream_idx = -1;
        video_stream = 0;
        codec_ctx = 0;
        decoder = 0;
        av_frame = 0;
        gl_frame = 0;
        conv_ctx = 0;
    }

    void clear()                                                                                // clean up the app data structure
    {
        if (av_frame) av_free(av_frame);
        if (gl_frame) av_free(gl_frame);
        if (packet) av_free(packet);
        if (codec_ctx) avcodec_close(codec_ctx);
        if (fmt_ctx) avformat_free_context(fmt_ctx);
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // check that 
    // --- filename was provided
    // --- file exists and contains stream header information
    // --- file contaions video stream
    // --- FFMPEG library has the corresponding codec
    //===================================================================================================================================================================================================================
    if (argc < 2)
        exit_msg("Usage : %s <filename> ", argv[0]);

    av_register_all();                                                                                  // initialize libav
    avformat_network_init();
    
    AppData data;                                                                                       // initialize custom data structure
    data.initialize();
    
    if (avformat_open_input(&data.fmt_ctx, argv[1], 0, 0) < 0)                                          // open video
    {
        data.clear();
        exit_msg("Failed to open input file : %s", argv[1]);
    }
    
    if (avformat_find_stream_info(data.fmt_ctx, 0) < 0)                                                 // find stream info
    {
        data.clear();
        exit_msg("Failed to get stream info.");
    }
    
    av_dump_format(data.fmt_ctx, 0, argv[1], 0);                                                        // dump debug info    
    
    for (unsigned int i = 0; i < data.fmt_ctx->nb_streams; ++i)                                         // find the video stream
    {
        if (data.fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            data.stream_idx = i;
            break;
        }
    }

    if (data.stream_idx == -1)
    {
        data.clear();
        exit_msg("Failed to find video stream.");
    }

    data.video_stream = data.fmt_ctx->streams[data.stream_idx];
    data.codec_ctx = data.video_stream->codec;    
    data.decoder = avcodec_find_decoder(data.codec_ctx->codec_id);                                      // find the decoder

    if (!data.decoder)
    {
        data.clear();
        exit_msg("Failed to find decoder for the stream.");
    }

    
    if (avcodec_open2(data.codec_ctx, data.decoder, 0) < 0)                                             // open the decoder
    {
        data.clear();
        exit_msg("Failed to open codec.");
    }    

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 8AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("FFMPEG Video Player", 8, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // creating shaders and uniforms
    //===================================================================================================================================================================================================================
    glsl_program_t glsl_video_player(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/video.vs"),
                                     glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/video.fs"));

    glsl_video_player.enable();
    glsl_video_player["frameTex"] = 0;
    uniform_t uniform_bump_intensity = glsl_video_player["bump_intensity"]; 
    
    data.av_frame = av_frame_alloc();                                                              // allocate the video frames
    data.gl_frame = av_frame_alloc();
    int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, data.codec_ctx->width, data.codec_ctx->height, 1);
    uint8_t* internal_buffer = (uint8_t *) av_malloc(size * sizeof(uint8_t));
    avpicture_fill((AVPicture *)data.gl_frame, internal_buffer, AV_PIX_FMT_RGB24, data.codec_ctx->width, data.codec_ctx->height);

    data.packet = (AVPacket *) av_malloc(sizeof(AVPacket));


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //===================================================================================================================================================================================================================
    // Simple full-screen quad buffer setup
    //===================================================================================================================================================================================================================
    glm::vec2 uvs[] = 
    {
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f,  1.0f)
    };

    GLuint vao_id, vbo_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //===================================================================================================================================================================================================================
    // Create texture
    //===================================================================================================================================================================================================================
    GLuint frame_tex;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &frame_tex);
    glBindTexture(GL_TEXTURE_2D, frame_tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.codec_ctx->width, data.codec_ctx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uniform_bump_intensity = window.bump_intensity;

        //===============================================================================================================================================================================================================
        // read next frame
        //===============================================================================================================================================================================================================
        do
        {
            if (av_read_frame(data.fmt_ctx, data.packet) < 0) 
            {
                av_packet_unref(data.packet);
                goto exit_program;
            }
    
            if (data.packet->stream_index == data.stream_idx) 
            {
                int frame_finished = 1;
        
                int ret = avcodec_send_packet(data.codec_ctx, data.packet);
                if (ret < 0) 
                {
                    av_packet_unref(data.packet);
                    goto exit_program;
                }

                ret = avcodec_receive_frame(data.codec_ctx, data.av_frame);
                if ((ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) || (ret < 0))
                {
                    debug_msg("Error during decoding or end of file reached\n");
                    av_packet_unref(data.packet);
                    goto exit_program;
                }

                if (!data.conv_ctx) 
                    data.conv_ctx = sws_getContext(data.codec_ctx->width, data.codec_ctx->height, data.codec_ctx->pix_fmt, data.codec_ctx->width, data.codec_ctx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, 0, 0, 0);
            
                sws_scale(data.conv_ctx, data.av_frame->data, data.av_frame->linesize, 0, data.codec_ctx->height, data.gl_frame->data, data.gl_frame->linesize);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.codec_ctx->width, data.codec_ctx->height, GL_RGB, GL_UNSIGNED_BYTE, data.gl_frame->data[0]);
            }
        
            av_packet_unref(data.packet);
        } 
        while (data.packet->stream_index != data.stream_idx);


        glBindTexture(GL_TEXTURE_2D, frame_tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        window.swap_buffers();
        glfw::poll_events();
    }

  exit_program:     
    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glDeleteVertexArrays(1, &vao_id);
    glDeleteBuffers(1, &vbo_id);
    glDeleteTextures(1, &frame_tex);

    avformat_close_input(&data.fmt_ctx);
    data.clear();

    glfw::terminate();
    return 0;
}