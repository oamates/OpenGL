#include <memory>
#include <random>

#include "glsl_noise.hpp"

namespace glsl_noise {

static int perm[256] = {151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
                        247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32, 57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
                         74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122, 60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
                         65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
                         52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
                        119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
                        218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241, 81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
                        184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180};


//=======================================================================================================================================================================================================================
// permutation_texture - creates and loads a 2D texture for a combined index permutation and gradient lookup table
// this texture is used for 2D and 3D noise, both classic and simplex
//=======================================================================================================================================================================================================================
GLuint permutation_texture()
{
    int grad3[16][3] = {{ 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1},
                        { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
                        { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
                        { 1, 0,-1}, {-1, 0,-1}, { 0,-1, 1}, { 0, 1, 1}};

    char* pixels = (char*) malloc(256 * 256 * 4);
    int offset = 0;
    for(int i = 0; i < 256; i++) for(int j = 0; j < 256; j++)
    {
        char value = perm[(j + perm[i]) & 0xFF];
        pixels[offset + 0x0] = grad3[value & 0x0F][0] * 64 + 64;   // Gradient x
        pixels[offset + 0x1] = grad3[value & 0x0F][1] * 64 + 64;   // Gradient y
        pixels[offset + 0x2] = grad3[value & 0x0F][2] * 64 + 64;   // Gradient z
        pixels[offset + 0x3] = value;                              // Permuted index
        offset += 4;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);                        // GL_NEAREST is a must for the generator to work
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(pixels);
    return tex_id;
}

//=======================================================================================================================================================================================================================
// gradient_texture - creates and loads a 2D texture for a 4D gradient lookup table
// used for 4D noise only
//=======================================================================================================================================================================================================================
GLuint gradient_texture()
{
    int grad4[32][4]= {{ 0, 1, 1, 1}, { 0, 1, 1,-1}, { 0, 1,-1, 1}, { 0, 1,-1,-1}, // 32 tesseract edges
                       { 0,-1, 1, 1}, { 0,-1, 1,-1}, { 0,-1,-1, 1}, { 0,-1,-1,-1},
                       { 1, 0, 1, 1}, { 1, 0, 1,-1}, { 1, 0,-1, 1}, { 1, 0,-1,-1},
                       {-1, 0, 1, 1}, {-1, 0, 1,-1}, {-1, 0,-1, 1}, {-1, 0,-1,-1},
                       { 1, 1, 0, 1}, { 1, 1, 0,-1}, { 1,-1, 0, 1}, { 1,-1, 0,-1},
                       {-1, 1, 0, 1}, {-1, 1, 0,-1}, {-1,-1, 0, 1}, {-1,-1, 0,-1},
                       { 1, 1, 1, 0}, { 1, 1,-1, 0}, { 1,-1, 1, 0}, { 1,-1,-1, 0},
                       {-1, 1, 1, 0}, {-1, 1,-1, 0}, {-1,-1, 1, 0}, {-1,-1,-1, 0}};

    char* pixels = (char*) malloc(256 * 256 * 4);
    int offset = 0;
    for(int i = 0; i < 256; i++) for(int j = 0; j < 256; j++)
    {
        char value = perm[(j + perm[i]) & 0xFF];
        pixels[offset + 0x0] = grad4[value & 0x1F][0] * 64 + 64; // Gradient x
        pixels[offset + 0x1] = grad4[value & 0x1F][1] * 64 + 64; // Gradient y
        pixels[offset + 0x2] = grad4[value & 0x1F][2] * 64 + 64; // Gradient z
        pixels[offset + 0x3] = grad4[value & 0x1F][3] * 64 + 64; // Gradient w
        offset += 2;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);                        // GL_NEAREST is a must for the generator to work
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(pixels);
    return tex_id;
}


//=======================================================================================================================================================================================================================
// random RG texture with G channel a copy of red channel with integral shift
// to reduce the number of texture fetches in glsl noise functions
//=======================================================================================================================================================================================================================
GLuint randomRG_shift_tex256x256(glm::ivec2 shift)
{
    std::random_device rd;
    std::mt19937 engine(rd());

    unsigned char* pixels = (unsigned char*) malloc (256 * 256 * 2);

    unsigned int index = 0;
    for (int y = 0; y < 256; y++) for (int x = 0; x < 256; x++)
    {
        pixels[index] = engine();
        index += 2;
    }

    index = 0;
    for (int y = shift.y; y < 256 + shift.y; y++) for (int x = shift.x; x < 256 + shift.x; x++)
    {
        unsigned int index_s = ((y & 0xFF) << 8) | (x & 0xFF);
        pixels[2 * index_s + 1] = pixels[index];
        index += 2;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, 256, 256, 0, GL_RG, GL_UNSIGNED_BYTE, pixels);                            // GL_LINEAR is a must for the generator to work
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(pixels);
    return tex_id;
}


GLuint randomRGBA_shift_tex256x256(glm::ivec2 shift)
{
    std::random_device rd;
    std::mt19937 engine(rd());

    unsigned char* pixels = (unsigned char*) malloc (256 * 256 * 4);

    unsigned int index = 0;
    for (int y = 0; y < 256; y++) for (int x = 0; x < 256; x++)
    {
        unsigned int value = engine();
        pixels[index] = value & 0xFF;
        pixels[index + 2] = (value >> 8) & 0xFF;
        index += 4;
    }

    index = 0;
    for (int y = shift.y; y < 256 + shift.y; y++) for (int x = shift.x; x < 256 + shift.x; x++)
    {
        unsigned int index_s = ((y & 0xFF) << 8) | (x & 0xFF);
        pixels[4 * index_s + 1] = pixels[index];
        pixels[4 * index_s + 3] = pixels[index + 2];
        index += 4;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);                        // GL_LINEAR is a must for the generator to work
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(pixels);
    return tex_id;

}


} // namespace glsl_noise

