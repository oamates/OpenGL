#ifndef _player_included_134730389406134109278560184103489650235610238562516286
#define _player_included_134730389406134109278560184103489650235610238562516286

#include "decoder.hpp"
#include "shader.hpp"

struct player_t
{
    player_control_t decoder;

    AVFrame* currentFrame = 0;
    Player* player = 0;

    glsl_program_t yuv420p_converter;
    GLuint texture_y, texture_u, texture_v;
    GLuint vao_id, vbo_id;
    bool textures_initialized;

    // ========================================================================= //

    player_t() : textures_initialized(false), texture_y(0), texture_u(0), texture_v(0)
    {
        yuv420p_converter.link(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/yuv420p_conv.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/yuv420p_conv.fs"));

        yuv420p_converter.enable();

        glUniform1i(yuv420p_converter["y_channel"], 0);
        glUniform1i(yuv420p_converter["u_channel"], 1);
        glUniform1i(yuv420p_converter["v_channel"], 2);

        yuv420p_converter.disable();

        float uvs[] =
        {
                -1.0f, -1.0f,
                 1.0f, -1.0f,
                 1.0f,  1.0f,
                -1.0f,  1.0f
        };

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);        
    }

    ~player_t()
    {
        glDeleteTextures(1, &texture_y);
        glDeleteTextures(1, &texture_u);
        glDeleteTextures(1, &texture_v);
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    }

    GLuint single_channel_texture(GLsizei width, GLsizei height)
    {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return texture_id;
    }    

    void play() 
        { decoder.play(); }

    void pause()
        { decoder.pause(); }

    void set_frame(unsigned int index);
        { decoder.set_frame(index); }

    void next_frame();
        { decoder.next_frame(); }    

    frame_handle_t current_frame()
        { return decoder.current_frame(); }

    void set_source(const char* video)
    {
        if (video == decoder.source()) return;

        if (decoder.state() != playback_state_t::NOT_LOADED) 
            decoder.deinit();

        decoder.init(value, this);
    }

    const char* source() const
        { return decoder.source(); }

    int getDuration() const 
        { return decoder.duration; }

    int getFrameCount() const
        { return decoder.frame_count; }

    unsigned int getFramePisition() const 
        { return decoder.framePosition(); }

    playback_state_t getState() const
        { return decoder.state(); }

    void lock()
        { decoder.lock.lock(); }
    void unlock()
        { decoder.lock.unlock(); }

    void render()
    {
        lock();

        frame_handle_t handle = current_frame();
        current_frame = handle.frame;

        if (!textures_initialized)
        {
            texture_y = single_channel_texture(current_frame->width, current_frame->height);
            texture_u = single_channel_texture(current_frame->linesize[1], (current_frame->linesize[1] * current_frame->height) / current_frame->width);
            texture_v = single_channel_texture(current_frame->linesize[2], (current_frame->linesize[2] * current_frame->height) / current_frame->width);
            textures_initialized = true;
        }

        glActiveTexture(GL_TEXTURE0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int video_width  = current_frame->width;                                         // Set viewport to preserve video aspect
        int video_height = current_frame->height;

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_y);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_RED, GL_UNSIGNED_BYTE, current_frame->data[0]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture_u);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_frame->linesize[1], (current_frame->linesize[1] * video_height) / video_width, GL_RED, GL_UNSIGNED_BYTE, current_frame->data[1]);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture_v);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_frame->linesize[2], (current_frame->linesize[2] * video_height) / video_width, GL_RED, GL_UNSIGNED_BYTE, current_frame->data[2]);

        yuv420p_converter.enable();

        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        yuv420p_converter.disable();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        unlock();
    }

};

#endif // _player_included_134730389406134109278560184103489650235610238562516286