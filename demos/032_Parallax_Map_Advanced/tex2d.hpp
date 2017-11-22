#ifndef TEX2D_HPP
#define TEX2D_HPP

#include <GL/glew.h>

struct tex2d_t
{
    GLuint id;

    void loadFromFile(const char* file_name) {};
    void setSmooth(bool smooth) {};
    void setRepeated(bool repeated) {};

};

#endif
