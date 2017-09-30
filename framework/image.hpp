#ifndef _texture_included_34875624975643765623801675657843657846387563487562354 
#define _texture_included_34875624975643765623801675657843657846387563487562354

#define GLEW_STATIC
#include <GL/glew.h>
#include <png.h>

namespace image {

namespace stbi {

GLuint texture2d(const char* file_name, int* channels = 0, GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR_MIPMAP_LINEAR, GLint wrap_mode = GL_REPEAT);
GLuint hdr2d(const char* file_name, int* channels = 0, GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR_MIPMAP_LINEAR, GLint wrap_mode = GL_REPEAT);

} // namespace stbi

namespace png {

//=======================================================================================================================================================================================================================
// Random texture generation
//=======================================================================================================================================================================================================================
GLuint random_rgb(GLuint size);
GLuint random_rgba(GLuint size);

//=======================================================================================================================================================================================================================
// GL_TEXTURE_2D loading function
//=======================================================================================================================================================================================================================
GLuint texture2d(const char* file_name, int* channels = 0, GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR_MIPMAP_LINEAR, GLint wrap_mode = GL_REPEAT, bool float_texture = false);

//=======================================================================================================================================================================================================================
// GL_TEXTURE_CUBE_MAP loading function
//=======================================================================================================================================================================================================================
GLuint cubemap(const char* file_names[6]);

//=======================================================================================================================================================================================================================
// GL_TEXTURE_2D_ARRAY loading function
//=======================================================================================================================================================================================================================
GLuint texture2d_array(const char* file_name_pattern, int layers);

//=======================================================================================================================================================================================================================
// stores pixelbuffer into a png file
// -- acceptable values for color_type are : PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA, PNG_COLOR_TYPE_GRAY_ALPHA
// -- combined with glReadPixels can be used to make screenshots
//=======================================================================================================================================================================================================================
bool write(const char * filename, int width, int height, unsigned char * rgbbuffer, int color_type = PNG_COLOR_TYPE_RGB);

} // namespace png




namespace dds {

//=======================================================================================================================================================================================================================
// Enough mips for 16K x 16K, which is the minumum required for OpenGL 4.x
//=======================================================================================================================================================================================================================
#define MAX_TEXTURE_MIPS 14

//=======================================================================================================================================================================================================================
// Each texture image data structure contains an array of MAX_TEXTURE_MIPS of these mipmap structures. The structure represents the mipmap data for all slices at that level.
//=======================================================================================================================================================================================================================
struct mip_t                                    // vglImageMipData 
{
    GLsizei width;                              // Width of this mipmap level
    GLsizei height;                             // Height of this mipmap level
    GLsizei depth;                              // Depth pof mipmap level
    GLsizeiptr stride;                          // Distance in bytes between mip levels in memory                                   // mipStride;
    GLvoid* data;                               // Pointer to data
};

//=======================================================================================================================================================================================================================
// The main image data structure. It contains all the parameters needed to place texture data into a texture object using OpenGL.
//=======================================================================================================================================================================================================================
struct image_t
{
    GLenum target;                              // texture target (1D, 2D, cubemap, array, etc.)
    GLenum internal_format;                     // recommended internal format (GL_RGBA32F, etc).
    GLenum format;                              // format in memory
    GLenum type;                                // type in memory (GL_RED, GL_RGB, etc.)
    GLenum swizzle[4];                          // swizzle for RGBA
    GLsizei levels;                             // number of present mipmap levels                                                  // mipLevels
    GLsizei slices;                             // number of slices (for arrays)
    GLsizeiptr stride;                          // distance in bytes between slices of an array texture                             // sliceStride
    GLsizeiptr size;                            // total amount of data allocated for texture                                       // totalDataSize;
    mip_t mips[MAX_TEXTURE_MIPS];               // actual mipmap data
};

void unload(image_t* image);
GLuint load_texture(const char* filename, image_t* image);
void load(const char* filename, image_t* image);

} // namespace dds



} // namespace texture

#endif  // _texture_included_34875624975643765623801675657843657846387563487562354