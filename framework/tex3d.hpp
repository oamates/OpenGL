#ifndef _tex3d_included_3246158736480756134807560831475634108756341097560834156
#define _tex3d_included_3246158736480756134807560831475634108756341097560834156

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

struct tex3d_header_t
{
    GLenum target;
    GLenum internal_format;
    GLenum format;
    GLenum type;
    glm::ivec3 size;
    GLuint data_size;    
};

struct bbox_t
{
    glm::dvec3 center;
    glm::dvec3 size;
};

struct sdf_header_t
{
    tex3d_header_t tex3d_header;
    bbox_t bbox;
};

struct texture3d_t
{
    GLuint id;
    glm::ivec3 size;
    GLenum texture_unit;
    GLenum internal_format;

    texture3d_t () : id(0) {}

    texture3d_t(GLenum texture_unit, const char* file_name)
        : texture_unit(texture_unit)
    {
        tex3d_header_t header;

        FILE* f = fopen(file_name, "rb");
        assert(f);
        
        fread(&header, sizeof(tex3d_header_t), 1, f);

        size = header.size;
        internal_format = header.internal_format;
        void* texture_data = malloc(header.data_size);

        fread(texture_data, header.data_size, 1, f);
        fclose(f);

        glActiveTexture(texture_unit);
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexImage3D(GL_TEXTURE_3D, 0, internal_format, size.x, size.y, size.z, 0, header.format, header.type, texture_data);

        free(texture_data);
    }

    texture3d_t& operator = (texture3d_t&& rhs)
    {
        std::swap(id, rhs.id);
        size = rhs.size;
        texture_unit = rhs.texture_unit;
        internal_format = rhs.internal_format;
        return *this;
    }

    texture3d_t(texture3d_t&& rhs)
    {
        id = rhs.id;
        size = rhs.size;
        texture_unit = rhs.texture_unit;
        internal_format = rhs.internal_format;
        rhs.id = 0;
    }

    texture3d_t(GLenum texture_unit, tex3d_header_t& header, const GLvoid* data)
        : size(header.size),
          texture_unit(texture_unit),
          internal_format(header.internal_format)
    {
        glActiveTexture(texture_unit);
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexImage3D(GL_TEXTURE_3D, 0, internal_format, size.x, size.y, size.z, 0, header.format, header.type, data);
    }

    texture3d_t(const glm::ivec3& size, GLenum texture_unit, GLenum internal_format)
        : size(size), 
          texture_unit(texture_unit), 
          internal_format(internal_format)
    {
        glActiveTexture(texture_unit);
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_3D, id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage3D(GL_TEXTURE_3D, 1, internal_format, size.x, size.y, size.z);
    }

    void bind_as_image(GLuint image_unit, GLenum access = GL_READ_WRITE)
        { glBindImageTexture(image_unit, id, 0, GL_TRUE, 0, access, internal_format); }

    GLuint data_size()
        { return size.x * size.y * size.z * sizeof(GLuint); }

    void dump_info()
    {
        debug_msg("Texture %u information :: ", id);
        debug_msg("\tinternal_format = %x.", internal_format);
        debug_msg("\tsize = (%u, %u, %u).", size.x, size.y, size.z);
    }

    static texture3d_t load_sdf(GLenum texture_unit, const char* file_name, bbox_t& bbox)
    {
        texture3d_t texture;
        texture.texture_unit = texture_unit;

        tex3d_header_t header;

        FILE* f = fopen(file_name, "rb");
        assert(f);
        
        fread(&header, sizeof(tex3d_header_t), 1, f);

        texture.size = header.size;
        texture.internal_format = header.internal_format;
        void* texture_data = malloc(header.data_size);

        fread(&bbox, sizeof(bbox_t), 1, f);
        fread(texture_data, header.data_size, 1, f);
        fclose(f);

        glActiveTexture(texture_unit);
        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_3D, texture.id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexImage3D(GL_TEXTURE_3D, 0, texture.internal_format, texture.size.x, texture.size.y, texture.size.z, 0, header.format, header.type, texture_data);

        free(texture_data);
        return texture;
    }


    ~texture3d_t()
        { if (id) glDeleteTextures(1, &id); }
};

#endif // _tex3d_included_3246158736480756134807560831475634108756341097560834156