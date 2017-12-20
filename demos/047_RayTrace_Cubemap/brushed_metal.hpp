#ifndef __brushed_metal_8723568346836123085762783560782356827356408235602612346
#define __brushed_metal_8723568346836123085762783560782356827356408235602612346

#include <cassert>
#include <cstdlib>
#include <cmath>

static void _make_pixel(uint8_t* b, uint8_t* e, GLsizei w, GLsizei h, GLint x, GLint y, double /*c*/, uint8_t r, uint8_t g)
{
    while(x < 0) x += w;
    while(y < 0) y += h;
    if (x >= w) x %= w;
    if (y >= h) y %= h;
    uint8_t* p = b + (y * w + x) * 3;
    uint8_t* pr = p;
    uint8_t* pg = p + 1;
    uint8_t* pb = p + 2;

    *pr = r;
    *pg = g;
    *pb = (*pb + 8) % 0x100;
}

static void _make_scratch(uint8_t* b, uint8_t *e, GLsizei w, GLsizei h, GLint x, GLint y, GLint dx, GLint dy)
{
    if ((dx == 0) && (dy == 0)) return;

    double dd = std::sqrt(double(dx * dx + dy * dy));

    uint8_t r = uint8_t((dy / dd) * 0xFF);
    uint8_t g = uint8_t((dx / dd) * 0xFF);

    if (dx > dy)
    {
        if (dx >= 0)
        {
            for(GLint i = 0; i < dx; ++i)
            {
                double c = double(i) / dx;
                GLint j = GLint(dy * c);
                _make_pixel(b, e, w, h, x + i, y + j, c, r, g);
            }
        }
        else
        {
            for (GLint i = 0; i > dx; --i)
            {
                double c = double(i) / dx;
                GLint j = GLint(dy * c);
                _make_pixel(b, e, w, h, x + i, y + j, c, r, g);
            }
        }
    }
    else
    {
        if (dy >= 0)
        {
            for(GLint j = 0; j < dy; ++j)
            {
                double c = double(j) / dy;
                GLint i = GLint(dx * c);
                _make_pixel(b, e, w, h, x + i, y + j, c, r, g);
            }
        }
        else
        {
            for(GLint j = 0; j > dy; --j)
            {
                double c = double(j) / dy;
                GLint i = GLint(dx * c);
                _make_pixel(b, e, w, h, x + i, y + j, c, r, g);
            }
        }
    }
}

uint8_t* brushed_metal_texture(GLsizei width, GLsizei height, int n_scratches, int s_disp_min, int s_disp_max, int t_disp_min, int t_disp_max)
{
    GLuint size = 3 * width * height;
    uint8_t* rgb_data = (uint8_t*) malloc(size);
    uint8_t* p = rgb_data;
    uint8_t* e = rgb_data + size;
    while(n_scratches--)
    {
        const GLuint n_segments = 1 + std::rand() % 4;
        GLint x = std::rand() % width;
        GLint y = std::rand() % height;
        for(GLuint seg = 0; seg < n_segments; ++seg)
        {
            GLint dx = s_disp_min + (std::rand() % (s_disp_max - s_disp_min + 1));
            GLint dy = t_disp_min + (std::rand() % (t_disp_max - t_disp_min + 1));

            _make_scratch(p, e, width, height, x, y, dx, dy);
            x += dx;
            y += dy;
        }
    }
}

#endif // __brushed_metal_8723568346836123085762783560782356827356408235602612346