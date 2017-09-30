//=======================================================================================================================================================================================================================
//  Texture loading functions implementation
//=======================================================================================================================================================================================================================

#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "image.hpp"

namespace image {

namespace stbi {

#include "image/stb_image.h"

GLuint texture2d(const char* file_name, int* channels, GLint mag_filter, GLint min_filter, GLint wrap_mode)
{
    GLuint texture_id;

    int width, height, bpp; // bytes per pixel

    unsigned char* data = stbi_load(file_name, &width, &height, &bpp, 0);

    if (!data)
    {
        debug_msg("stbi :: failed to load image : %s", file_name);
        return 0;
    }
    
    GLenum format = (bpp == 1) ? GL_RED :
                    (bpp == 2) ? GL_RG : 
                    (bpp == 3) ? GL_RGB : GL_RGBA;
    if (channels) *channels = bpp;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR) 
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture_id;
}

GLuint hdr2d(const char* file_name, int* channels, GLint mag_filter, GLint min_filter, GLint wrap_mode)
{
    GLuint texture_id;    
    int width, height, bpp; // bytes per pixel

    float *data = stbi_loadf(file_name, &width, &height, &bpp, 0);
    if (!data)
    {
        debug_msg("stbi :: failed to load HDR image : %s", file_name);
        return 0;
    }

    GLint internal_format;
    GLenum format;

    if (bpp == 1)
    {
        internal_format = GL_R16F;
        format = GL_RED;
    }
    else if (bpp == 2)
    {
        internal_format = GL_RG16F;
        format = GL_RG;
    }
    else if (bpp == 3)
    {
        internal_format = GL_RGB16F;
        format = GL_RGB;
    }
    else
    {
        internal_format = GL_RGBA16F;
        format = GL_RGBA;
    }

    if (channels) *channels = bpp;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_FLOAT, data); // note how we specify the texture's data value to be float
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    if (min_filter == GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture_id;
}

} // namespace stbi

namespace tga {

void write(const char * file_name, int width, int height, unsigned char* pixelbuffer) // RGBA assumed
{
    FILE * fp = fopen (file_name, "wb");
    char header[18] = {0};
    header[2] = 2;
    header[12] = width & 0xff;
    header[13] = width >> 8;
    header[14] = height & 0xff;
    header[15] = height >> 8;
    header[16] = 32;

    fwrite (header, 1, 18, fp);    
    fwrite (pixelbuffer, 4, width * height, fp);
    fclose(fp);
}    

} 
namespace png {

//===================================================================================================================================================================================================================
// random texture generation
//===================================================================================================================================================================================================================

GLuint random_rgb(GLuint size)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);    
    glBindTexture(GL_TEXTURE_2D, texture_id);

    unsigned int data_size = size * size * sizeof(glm::vec3);
    glm::vec3* random_data = (glm::vec3 *) malloc(data_size);

    for (unsigned int i = 0 ; i < size * size ; i++) 
        random_data[i] = glm::vec3(glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f));
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, glm::value_ptr(random_data[0]));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
    glGenerateMipmap(GL_TEXTURE_2D);
    free(random_data);
    return texture_id;
}

GLuint random_rgba(GLuint size)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);    
    glBindTexture(GL_TEXTURE_2D, texture_id);

    unsigned int data_size = size * size * sizeof(glm::vec4);
    glm::vec4 * random_data = (glm::vec4 *) malloc(data_size);

    for (unsigned int i = 0 ; i < size * size ; i++)
        random_data[i] = glm::vec4(glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f));
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(random_data[0]));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
    glGenerateMipmap(GL_TEXTURE_2D);
    free(random_data);
    return texture_id;
}

//===================================================================================================================================================================================================================
// creates GL_TEXTURE_CUBE_MAP from six png files
//===================================================================================================================================================================================================================

GLuint cubemap(const char* file_names[6])
{
    debug_msg("Loading Cubemap PNG texture.");

    static const char * face_name[] = 
    {
        "GL_TEXTURE_CUBE_MAP_POSITIVE_X",
        "GL_TEXTURE_CUBE_MAP_NEGATIVE_X",
        "GL_TEXTURE_CUBE_MAP_POSITIVE_Y",
        "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y",
        "GL_TEXTURE_CUBE_MAP_POSITIVE_Z",
        "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z"
    };

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    int bit_depth, color_type;                                                                                          
    png_uint_32 width, height;
    png_byte * image_data;
    png_byte ** row_pointers;
    int rowbytes;
    GLint format;


    for (int i = 0; i < 6; ++i)
    {

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (setjmp(png_jmpbuf(png_ptr))) 
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
            debug_msg("Error from libpng.");
            return 0;
        }
        //===========================================================================================================================================================================================================
        // open the file and read its 8-byte header to make sure the format is png
        //===========================================================================================================================================================================================================
        const char * file_name = file_names[i];
        GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        debug_msg("Loading %s face from %s", face_name[i], file_name);

        png_byte header[8];
        FILE * file = fopen(file_name, "rb");
        if ((0 == file) || (8 != fread(header, 1, 8, file)) || png_sig_cmp(header, 0, 8))
        { 
            fclose(file);
            debug_msg("Failed to open %s file or invalid png format.", file_name);
            return 0;
        }
        
        //===========================================================================================================================================================================================================
        // set up reading struct, image info struct and error callback point
        //===========================================================================================================================================================================================================
        png_init_io(png_ptr, file);                                                                                         
        png_set_sig_bytes(png_ptr, 8);                                                                                      
        png_read_info(png_ptr, info_ptr);                                                                                   
        
        //===========================================================================================================================================================================================================
        // calculate the space needed and allocate it, row size in bytes should be aligned to the next 4-byte boundary as required (by default) by glTexImage2d 
        //===========================================================================================================================================================================================================
        if (0 == i)
        {
            png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);                                 
            if ((bit_depth != 8) || (width != height) || ((PNG_COLOR_TYPE_RGB != color_type) && (PNG_COLOR_TYPE_RGB_ALPHA != color_type)))
            {
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Invalid bit depth (%d) or color type (%d) or image is not rectangular (%d x %d).", bit_depth, color_type, width, height);
                return 0;
            }
            png_read_update_info(png_ptr, info_ptr);                                                                            

            format = (PNG_COLOR_TYPE_RGB == color_type) ? GL_RGB : GL_RGBA;
            rowbytes = (png_get_rowbytes(png_ptr, info_ptr) + 3) & 0xFFFFFFFC;                                              
            image_data = (png_byte *) malloc(rowbytes * height * sizeof(png_byte));                     
            row_pointers = (png_byte **) malloc(height * sizeof(png_byte *));                                       
        
            if ((0 == image_data) || (0 == row_pointers))
            {
                free(image_data);
                free(row_pointers);
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Error: could not allocate memory for png image data or row pointers\n");
                return 0;
            }
            for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;                 
        }
        else
        {
            int bit_depth0, color_type0;                                                                                            
            png_uint_32 width0, height0;
            png_get_IHDR(png_ptr, info_ptr, &width0, &height0, &bit_depth0, &color_type0, 0, 0, 0);
            png_read_update_info(png_ptr, info_ptr);                                                                            

            if ((bit_depth0 != bit_depth) || (color_type0 != color_type) || (width0 != width) || (height0 != height))
            {
                free(image_data);
                free(row_pointers);
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Image bit depth, color type, width or height mismatch for images %s and %s", file_names[i], file_names[0]);
                return 0;
            }
        }
        //===========================================================================================================================================================================================================
        // read the image data and fill the corresponding cubemap face
        //===========================================================================================================================================================================================================
        png_read_image(png_ptr, row_pointers);                                                                              
        glTexImage2D(face, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
        fclose(file);

        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    }

    free(image_data);
    free(row_pointers);

    debug_msg("Cubemap texture with id = %d successfully created.", texture_id);
    return texture_id;
}

//===================================================================================================================================================================================================================
// creates GL_TEXTURE_CUBE_MAP from six png files
//===================================================================================================================================================================================================================

GLuint texture2d_array(const char* file_name_pattern, int layers)
{
    debug_msg("PNG :: Loading TEXTURE2D_ARRAY texture.");

    char file_name[256];

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    int bit_depth, color_type;                                                                                          
    png_uint_32 width, height;
    png_byte * image_data;
    png_byte ** row_pointers;
    int rowbytes;
    GLint format, internal_format;

    for (int i = 0; i < layers; ++i)
    {

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (setjmp(png_jmpbuf(png_ptr))) 
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
            debug_msg("Error from libpng.");
            return 0;
        }
        //===========================================================================================================================================================================================================
        // open the file and read its 8-byte header to make sure the format is png
        //===========================================================================================================================================================================================================
        sprintf(file_name, file_name_pattern, i);
        debug_msg("Loading layer #%u from file %s", i, file_name);

        png_byte header[8];
        FILE * file = fopen(file_name, "rb");
        if ((0 == file) || (8 != fread(header, 1, 8, file)) || png_sig_cmp(header, 0, 8))
        { 
            fclose(file);
            debug_msg("Failed to open %s file or invalid png format.", file_name);
            return 0;
        }
        
        //===========================================================================================================================================================================================================
        // set up reading struct, image info struct and error callback point
        //===========================================================================================================================================================================================================
        png_init_io(png_ptr, file);                                                                                         
        png_set_sig_bytes(png_ptr, 8);                                                                                      
        png_read_info(png_ptr, info_ptr);                                                                                   
        
        //===========================================================================================================================================================================================================
        // calculate the space needed and allocate it, row size in bytes should be aligned to the next 4-byte boundary as required (by default) by glTexImage2d 
        //===========================================================================================================================================================================================================
        if (0 == i)
        {
            png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);                                 
            if ((bit_depth != 8) || ((PNG_COLOR_TYPE_RGB != color_type) && (PNG_COLOR_TYPE_RGB_ALPHA != color_type)))
            {
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Invalid bit depth (%d) or color type (%d).", bit_depth, color_type);
                return 0;
            }
            png_read_update_info(png_ptr, info_ptr);                                                                            

            if (PNG_COLOR_TYPE_RGB == color_type)
            {
                internal_format = GL_RGB8;
                format = GL_RGB;
            }
            else
            {
                internal_format = GL_RGBA8;
                format = GL_RGBA;
            }

            rowbytes = (png_get_rowbytes(png_ptr, info_ptr) + 3) & 0xFFFFFFFC;                                              
            image_data = (png_byte *) malloc(rowbytes * height * sizeof(png_byte));                     
            row_pointers = (png_byte **) malloc(height * sizeof(png_byte *));                                       
        
            if ((0 == image_data) || (0 == row_pointers))
            {
                free(image_data);
                free(row_pointers);
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Error: could not allocate memory for png image data or row pointers\n");
                return 0;
            }
            for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;

            int l = 0;
            int m = (width > height) ? width : height;
            while (m)
            {
                m >>= 1; 
                ++l; 
            }
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, l, internal_format, width, height, layers);
        }
        else
        {
            int bit_depth0, color_type0;                                                                                            
            png_uint_32 width0, height0;
            png_get_IHDR(png_ptr, info_ptr, &width0, &height0, &bit_depth0, &color_type0, 0, 0, 0);
            png_read_update_info(png_ptr, info_ptr);                                                                            

            if ((bit_depth0 != bit_depth) || (color_type0 != color_type) || (width0 != width) || (height0 != height))
            {
                free(image_data);
                free(row_pointers);
                png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                fclose(file);
                glDeleteTextures(1, &texture_id);
                debug_msg("Image bit depth, color type or size (%u x %u) for image %s", file_name, width0, height0);
                debug_msg("Previous image size is (%u x %u)", width, height);
                fflush(stdout);
                return 0;
            }
        }
        //===========================================================================================================================================================================================================
        // read the image data and fill the corresponding cubemap face
        //===========================================================================================================================================================================================================
        png_read_image(png_ptr, row_pointers);                                                                              
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, image_data); 

        fclose(file);

        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    free(image_data);
    free(row_pointers);

    debug_msg("GL_TEXTURE_2D_ARRAY texture with id = %d successfully created.", texture_id);
    return texture_id;
}

//===================================================================================================================================================================================================================
// creates GL_TEXTURE_2D from a png file
//===================================================================================================================================================================================================================
GLuint texture2d(const char* file_name, int* channels, GLint mag_filter, GLint min_filter, GLint wrap_mode, bool float_texture)
{
    debug_msg("Loading PNG texture : %s", file_name);

    //===============================================================================================================================================================================================================
    // open the file and read its 8-byte header to make sure the format is png
    //===============================================================================================================================================================================================================
    png_byte header[8];
    FILE * file = fopen(file_name, "rb");
    if ((0 == file) || (8 != fread(header, 1, 8, file)) || png_sig_cmp(header, 0, 8))
    { 
        fclose(file);
        debug_msg("Failed to open %s or invalid png format.", file_name);
        return 0;
    }

    //===============================================================================================================================================================================================================
    // set up reading struct, image info struct and error callback point
    //===============================================================================================================================================================================================================
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (setjmp(png_jmpbuf(png_ptr))) 
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        debug_msg("Error from libpng.");
        return 0;
    }

    png_init_io(png_ptr, file);                                                                                         
    png_set_sig_bytes(png_ptr, 8);                                                                                      
    png_read_info(png_ptr, info_ptr);                                                                                   
    int bit_depth, color_type;                                                                                          
    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);                                 

    if ((color_type == PNG_COLOR_TYPE_PALETTE) || ((color_type == PNG_COLOR_TYPE_GRAY) && (bit_depth < 8)) || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);

    // Tell libpng to strip 16 bits/color files down to 8 bits/color (png_set_scale_16 uses accurate scaling)
    if (bit_depth == 16) png_set_scale_16(png_ptr);

    GLint format, internal_format;

    switch (color_type)
    {
        case PNG_COLOR_TYPE_GRAY: debug_msg("Image color type : PNG_COLOR_TYPE_GRAY");
            format = GL_RED;
            internal_format = float_texture ? GL_R32F : GL_RED;             
            if (channels) 
                *channels = 1; 
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA : debug_msg("Image color type : PNG_COLOR_TYPE_GRAY_ALPHA");
            format = GL_RG;
            internal_format = float_texture ? GL_RG32F : GL_RG;             
            if (channels) 
                *channels = 2; 
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA : debug_msg("Image color type : PNG_COLOR_TYPE_RGB_ALPHA");
            format = GL_RGBA;
            internal_format = float_texture ? GL_RGBA32F : GL_RGBA;
            if (channels) 
                *channels = 4; 
            break;
        default : // we have either an RGB image or a paletted image, that was converted to RGB
            debug_msg("Image color type : %s", (color_type == PNG_COLOR_TYPE_RGB ? "PNG_COLOR_TYPE_RGB" : "PNG_COLOR_TYPE_PALETTE"));
            format = GL_RGB;
            internal_format = float_texture ? GL_RGB32F : GL_RGB;
            if (channels) 
                *channels = 3; 
    }

    png_read_update_info(png_ptr, info_ptr);                                                                            

    //===============================================================================================================================================================================================================
    // calculate the space needed and allocate it, row size in bytes should be aligned to the next 4-byte boundary as required (by default) by glTexImage2d 
    //===============================================================================================================================================================================================================
    int rowbytes = (png_get_rowbytes(png_ptr, info_ptr) + 3) & 0xFFFFFFFC;                                              
    png_byte * image_data = (png_byte *) malloc(rowbytes * height * sizeof(png_byte));                      
    png_byte ** row_pointers = (png_byte **) malloc(height * sizeof(png_byte *));                                       

    if ((0 == image_data) || (0 == row_pointers))
    {
        free(image_data);
        free(row_pointers);
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        fclose(file);
        debug_msg("Error: could not allocate memory for PNG image data or row pointers\n");
        return 0;
    }
    
    for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;                 

    //===============================================================================================================================================================================================================
    // finally, read the image data, create a texture and supply it with the data
    //===============================================================================================================================================================================================================
    png_read_image(png_ptr, row_pointers);                                                                              

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
    if (min_filter == GL_LINEAR_MIPMAP_LINEAR) glGenerateMipmap(GL_TEXTURE_2D);

    //===============================================================================================================================================================================================================
    // clean up
    //===============================================================================================================================================================================================================
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    free(image_data);
    free(row_pointers);
    fclose(file);
    debug_msg("Texture successfully loaded : id = %u", texture_id);
    return texture_id;
}

//===================================================================================================================================================================================================================
// stores pixelbuffer into a png file
//===================================================================================================================================================================================================================
bool write(const char * file_name, int width, int height, unsigned char* pixelbuffer, int color_type)
{
    assert(color_type != PNG_COLOR_TYPE_PALETTE);
    FILE * fp = fopen (file_name, "wb");
    if (!fp)
    {
        debug_msg("Could not open %s for writing png image.", file_name);
        return false;
    }

    png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct (png_ptr);

    if (setjmp (png_jmpbuf (png_ptr)))
    {
        png_destroy_write_struct (&png_ptr, &info_ptr);
        fclose (fp);
        return false;
    }

    png_set_IHDR (png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_bytepp row_pointers = (png_bytepp) malloc (height * sizeof (png_bytep));
    
    int pixel_size = (color_type == PNG_COLOR_TYPE_GRAY) ? 1 : 
                     (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ? 2 :
                     (color_type == PNG_COLOR_TYPE_RGB) ? 3 : 4;

    for (int y = 0; y < height; ++y) row_pointers[y] = pixelbuffer + y * width * pixel_size;

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

    png_destroy_write_struct (&png_ptr, &info_ptr);
    fclose (fp);
    debug_msg("PNG File %s successfully stored.", file_name);
    return true;
}

} // namespace png
} // namespace texture
    