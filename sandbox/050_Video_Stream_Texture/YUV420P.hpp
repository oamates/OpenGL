#ifndef _YUV420P_player_included_3490534236450364503461068534056345631625096784
#define _YUV420P_player_included_3490534236450364503461068534056345631625096784

//  YUV420P Player
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------
//  This class implements a simple YUV420P renderer. This means that you need to feed planar YUV420 data to the setYPixels(), setUPixels() and setVPixels().
// First make sure to call setup() with the video width and height. We use these dimensions to allocate the Y, U and V textures. After calling setupyou call 
// the zset{Y,U,V}Pixels() everytime you have a new frame that you want to render. With the `draw()` function you draw the current frame to the screen.
// If you resize your viewport, make sure to  call `resize()` so we can adjust the projection matrix.
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <GL/glew.h> 
#include <glm/glm.hpp> 

#include "shader.hpp"

struct YUV420P_Player 
{
    YUV420P_Player();
    bool setup(int w, int h);
    void setYPixels(uint8_t* pixels, int stride);
    void setUPixels(uint8_t* pixels, int stride);
    void setVPixels(uint8_t* pixels, int stride);
    void draw(int x, int y, int w = 0, int h = 0);
    void resize(int winW, int winH);

    bool setupTextures();
    bool setupShader();
 
    int vid_w, vid_h, win_w, win_h;
    GLuint vao;
    GLuint y_tex, u_tex, v_tex;
    glsl_program yuv420p;
    GLint u_pos;
    bool textures_created;
    bool shader_created;
    uint8_t *y_pixels, *u_pixels, *v_pixels;
    glm::mat4 pm;
};

#endif // _YUV420P_player_included_3490534236450364503461068534056345631625096784
