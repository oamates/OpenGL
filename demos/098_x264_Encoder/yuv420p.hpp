//=======================================================================================================================================================================================================================
// Simple YUV420P Player : 
//=======================================================================================================================================================================================================================
// --- call setup() with the video width and height
// --- call zset{Y,U,V}Pixels() everytime there is a new frame to render
// --- call draw() function you draw the current frame to the screen
// --- to resize the viewport, call resize() to adjust the projection matrix
//=======================================================================================================================================================================================================================

#ifndef _yuv420p_player_9887823453465104715424353247865345683465346592346734295
#define _yuv420p_player_9887823453465104715424353247865345683465346592346734295

#include <cstdint>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"
 
struct yuv420p_player
{
 
    yuv420p_player();
    ~yuv420p_player();

    void setup(int w, int h);

    void setYPixels(uint8_t* pixels, int stride);
    void setUPixels(uint8_t* pixels, int stride);
    void setVPixels(uint8_t* pixels, int stride);

    void draw(int x, int y, int w = 0, int h = 0);
    void resize(int winW, int winH);
 
    bool setupTextures();
    bool setupShader();
 
    GLuint vao_id;
    int vid_w, vid_h, win_w, win_h;

    GLuint y_texture, u_texture, v_texture;
    uint8_t *y_pixels, *u_pixels, *v_pixels;

    glsl_program_t yuv420p_converter;
};

#endif // _yuv420p_player_9887823453465104715424353247865345683465346592346734295