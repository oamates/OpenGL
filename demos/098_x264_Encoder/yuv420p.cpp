#include <glm/gtc/matrix_transform.hpp> 

#include "yuv420p.hpp"
#include "log.hpp"

yuv420p_player::yuv420p_player() : vao_id(0),
    vid_w(0), vid_h(0), win_w(0), win_h(0),
    y_texture(0), u_texture(0), v_texture(0),
    y_pixels(0), u_pixels(0), v_pixels(0)
{ }
 
yuv420p_player::~yuv420p_player()
{
    if (y_pixels) free(y_pixels);
    if (u_pixels) free(u_pixels);
    if (v_pixels) free(v_pixels);
}


void yuv420p_player::setup(int vidW, int vidH)
{
    vid_w = vidW;
    vid_h = vidH;
 
    y_pixels = (uint8_t*) malloc(vid_w * vid_h * sizeof(uint8_t));
    u_pixels = (uint8_t*) malloc(int((vid_w * 0.5) * (vid_h * 0.5)) * sizeof(uint8_t));
    v_pixels = (uint8_t*) malloc(int((vid_w * 0.5) * (vid_h * 0.5)) * sizeof(uint8_t));

    glGenTextures(1, &y_texture);
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w, vid_h, 0, GL_RED, GL_UNSIGNED_BYTE, 0); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenTextures(1, &u_texture);
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w/2, vid_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenTextures(1, &v_texture);
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w/2, vid_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenVertexArrays(1, &vao_id);

    yuv420p_converter.link(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/yuv420p_converter.vs"),
                           glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/yuv420p_converter.fs"));

    yuv420p_converter.enable();

    yuv420p_converter["y_tex"] = 0;
    yuv420p_converter["u_tex"] = 1;
    yuv420p_converter["v_tex"] = 2;
 
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    resize(viewport[2], viewport[3]);

}
 
void yuv420p_player::draw(int x, int y, int w, int h)
{
    if(w == 0) w = vid_w;
    if(h == 0) h = vid_h;
 
    glBindVertexArray(vao_id);
    yuv420p_converter.enable();
    yuv420p_converter["draw_pos"] = glm::vec4((float) x, (float) y, (float) w, (float) h);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
 
void yuv420p_player::resize(int winW, int winH)
{
    win_w = winW;
    win_h = winH;
    glm::mat4 projection_view_matrix = glm::ortho(0.0f, (float) win_w, (float) win_h, 0.0f, 0.0f, 100.0f);
    yuv420p_converter["projection_view_matrix"] = projection_view_matrix;
}
 
void yuv420p_player::setYPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, y_texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w, vid_h, GL_RED, GL_UNSIGNED_BYTE, pixels);
}
 
void yuv420p_player::setUPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, u_texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}
 
void yuv420p_player::setVPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, v_texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}