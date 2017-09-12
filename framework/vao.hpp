#ifndef __vao_included_43026861862561872462458746024956097046805382424868922784
#define __vao_included_43026861862561872462458746024956097046805382424868922784

#include <cstdio>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "log.hpp"

//=======================================================================================================================================================================================================================
// OpenGL index buffer object
//=======================================================================================================================================================================================================================
struct ibo_t
{
    GLuint id;
    GLenum type;
    GLenum mode;
    GLuint size;
    GLuint pri;

    ibo_t () : id(0), pri(-1) {};

    ibo_t& operator = (ibo_t&& other)
    {
        std::swap(id, other.id);
        type = other.type;
        mode = other.mode;
        size = other.size;
        pri = other.pri;
        return *this;
    }

    ibo_t(ibo_t&& other)
    {
        id = other.id;
        type = other.type;
        mode = other.mode;
        size = other.size;
        pri = other.pri;
        other.id = 0;
    }

    ibo_t(const ibo_t& other) = delete;
    ibo_t& operator = (const ibo_t&) = delete;

    template<typename index_t> struct info { static const GLenum type; };

    template<typename index_t> void init (GLenum mode, const index_t* data, GLuint size)
    {
        ibo_t::mode = mode;
        ibo_t::size = size;
        ibo_t::type = info<index_t>::type;
        ibo_t::pri  = info<index_t>::pri;

        glGenBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(index_t), data, GL_STATIC_DRAW);
    }

    template<typename index_t> void init (GLenum mode, const std::vector<index_t>& data)
        { init(mode, data.data(), data.size()); }


    void render()
        { glDrawElements(mode, size, type, 0); }

    // to be able to render the same data as GL_POINTS or GL_PATCHES
    void render(GLenum override_mode)
        { glDrawElements(override_mode, size, type, 0); }

    void render(GLsizei count, const GLvoid* offset)
        { glDrawElements(mode, count, type, offset); }

    void instanced_render(GLsizei primcount)
        { glDrawElementsInstanced(mode, size, type, 0, primcount); }

    ~ibo_t()
        { if (id) glDeleteBuffers(1, &id); }

    static void* map(GLenum access = GL_WRITE_ONLY)
        { return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, access); }

    static void unmap()
        { glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); }

};

template<> struct ibo_t::info<GLuint>   { static const GLenum type = GL_UNSIGNED_INT;   static const GLuint pri = 0xFFFFFFFF; };
template<> struct ibo_t::info<GLushort> { static const GLenum type = GL_UNSIGNED_SHORT; static const GLuint pri = 0xFFFF; };
template<> struct ibo_t::info<GLubyte>  { static const GLenum type = GL_UNSIGNED_BYTE;  static const GLuint pri = 0xFF; };

//=======================================================================================================================================================================================================================
// OpenGL vertex buffer object
//=======================================================================================================================================================================================================================
struct vbo_t
{
    GLuint id;
    GLuint layout;
    GLuint size;

    vbo_t () : id(0) {};

    vbo_t& operator = (vbo_t&& other)
    {
        std::swap(id, other.id);
        layout = other.layout;
        size = other.size;
        return *this;
    }

    vbo_t(vbo_t&& other)
    {
        id = other.id;
        layout = other.layout;
        size = other.size;
        other.id = 0;
    }

    vbo_t(const vbo_t& other) = delete;
    vbo_t& operator = (const vbo_t&) = delete;

    template <typename vertex_t> vbo_t (const vertex_t* data, size_t size)
        { init(data, size); }

    template <typename vertex_t> vbo_t (const std::vector<vertex_t>& vertices)
        { init(vertices.data(), vertices.size()); }

    template <typename vertex_t> void init (const vertex_t* data, size_t size)
    {
        vbo_t::size = size;
        vbo_t::layout = vertex_t::layout;
    
        glGenBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(vertex_t), data, GL_STATIC_DRAW);

        float* offset = 0;
        for(GLuint attribute = 0; attribute < vertex_t::attributes; ++attribute)
        {
            glEnableVertexAttribArray(attribute);
            glVertexAttribPointer(attribute, vertex_t::dimensions[attribute], GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset);
            offset += vertex_t::dimensions[attribute];
        }
    }

    template <typename vertex_t> void init (const std::vector<vertex_t>& data)
        { init(data.data(), data.size()); }

    static void* map(GLenum access = GL_WRITE_ONLY)
        { return glMapBuffer(GL_ARRAY_BUFFER, access); }

    static void unmap()
        { glUnmapBuffer(GL_ARRAY_BUFFER); }

    void render(GLenum mode)
        { glDrawArrays(mode, 0, size); }

    void instanced_render(GLenum mode, GLsizei primcount)
        { glDrawArraysInstanced(mode, 0, size, primcount); }

    ~vbo_t()
        { if (id) glDeleteBuffers(1, &id); }
};

//=======================================================================================================================================================================================================================
// OpenGL vertex array object
//=======================================================================================================================================================================================================================
struct vao_t
{
    GLuint id;
    vbo_t vbo;
    ibo_t ibo;

    struct header_t
    {
        //================================================================================================================================================================================================================
        // VBO description
        //================================================================================================================================================================================================================
        GLuint layout;
        GLuint vbo_size;

        //================================================================================================================================================================================================================
        // IBO description
        //================================================================================================================================================================================================================
        GLenum type;
        GLenum mode;
        GLuint ibo_size;
    };

    vao_t() : id(0) {};

    vao_t(const char* file_name)
        { init(file_name); }

    vao_t(vao_t&& other) : id(other.id), vbo(std::move(other.vbo)), ibo(std::move(other.ibo)) 
        { other.id = 0; }

    vao_t& operator = (vao_t&& other)
    {
        std::swap(id, other.id);
        vbo = std::move(other.vbo);
        ibo = std::move(other.ibo);
        return *this;
    }

    vao_t(const vao_t& other) = delete;
    vao_t& operator = (const vao_t&) = delete;

    void init(const char* file_name)
    {
        //================================================================================================================================================================================================================
        // read buffer params
        //================================================================================================================================================================================================================
        header_t header;

        FILE* f = fopen(file_name, "rb");
        fread (&header, sizeof(header_t), 1, f);

        glGenVertexArrays(1, &id);
        glBindVertexArray(id);
        
        //================================================================================================================================================================================================================
        // calculate attribute stride size in memory
        //================================================================================================================================================================================================================
        vbo.size = header.vbo_size;
        vbo.layout = header.layout;
        glGenBuffers(1, &vbo.id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
        GLsizei stride = 0;        
        for(GLuint layout = header.layout; layout; layout >>= 4)
            stride += layout & 0xF;
        stride *= sizeof(GLfloat);
        
        //================================================================================================================================================================================================================
        // set up vertex attributes layout in buffer
        //================================================================================================================================================================================================================
        GLuint attr_id = 0;
        float* offset = 0;
        
        for(GLuint layout = header.layout; layout; layout >>= 4)
        {
            glEnableVertexAttribArray(attr_id);
            GLuint attr_size = layout & 0xF;
            glVertexAttribPointer(attr_id, attr_size, GL_FLOAT, GL_FALSE, stride, offset);
            offset += attr_size;
            attr_id++;
        }
        
        GLvoid* buf_ptr;
        //================================================================================================================================================================================================================
        // map attribute buffer to memory and read file data directly to the address provided by OpenGL
        //================================================================================================================================================================================================================
        glBufferData(GL_ARRAY_BUFFER, header.vbo_size * stride, 0, GL_STATIC_DRAW);
        buf_ptr = vbo_t::map(GL_WRITE_ONLY);
        fread(buf_ptr, stride, header.vbo_size, f);
        vbo_t::unmap();
        
        //================================================================================================================================================================================================================
        // map index buffer to memory and read file data directly to it
        //================================================================================================================================================================================================================
        ibo.size = header.ibo_size;
        ibo.mode = header.mode;
        ibo.type = header.type;
        unsigned int index_size = (header.type == GL_UNSIGNED_INT) ? sizeof(GLuint) : (header.type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLubyte);    
        glGenBuffers(1, &ibo.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size * header.ibo_size, 0, GL_STATIC_DRAW);

        buf_ptr = ibo_t::map(GL_WRITE_ONLY);
        fread(buf_ptr, index_size, header.ibo_size, f);
        ibo_t::unmap();
                
        fclose(f);
    }

    template <typename vertex_t, typename index_t> 
    vao_t(GLenum mode, const vertex_t* vertex_data, unsigned int vertex_count, const index_t* index_data, unsigned int index_count)
        { init(mode, vertex_data, vertex_count, index_data, index_count); }

    template <typename vertex_t, typename index_t> 
    void init(GLenum mode, const vertex_t* vertex_data, unsigned int vertex_count, const index_t* index_data, unsigned int index_count)
    {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);    
        vbo.init<vertex_t>(vertex_data, vertex_count);
        ibo.init<index_t>(mode, index_data, index_count);    
    }
    
    template <typename vertex_t, typename index_t> 
    void init(GLenum mode, const std::vector<vertex_t>& vertex_data, const std::vector<index_t>& index_data)
        { init(mode, vertex_data.data(), vertex_data.size(), index_data.data(), index_data.size()); }
    
    template <typename vertex_t, typename index_t> 
    static void store(const char* file_name, GLenum mode, const vertex_t* vertex_data, unsigned int vertex_count, const index_t* index_data, unsigned int index_count)
    {
        FILE* f = fopen(file_name, "wb");
    
        header_t header;
        header.layout = vertex_t::layout;
        header.vbo_size = vertex_count;
        header.type = ibo_t::info<index_t>::type;
        header.mode = mode;
        header.ibo_size = index_count;

        fwrite(&header, sizeof(header_t), 1, f);
        fwrite(vertex_data, sizeof(vertex_t), vertex_count, f);
        fwrite(index_data, sizeof(index_t), index_count, f);
        fclose(f);
    }
    
    ~vao_t()
        { if (id) glDeleteVertexArrays(1, &id); };

    void bind()
        { glBindVertexArray(id); }

    void render()
    {
        printf("vao::render:: id = %u, ibo_id = %u, ibo_type = %u, ibo_mode = %u, ibo_size = %u, ibo_pri = %u\n", 
            id, ibo.id, ibo.type, ibo.mode, ibo.size, ibo.pri); 
        fflush(stdout);
        bind();
        ibo.render();
    }

    // to be able to render the same data as GL_POINTS or GL_PATCHES
    void render(GLenum override_mode)
    {
        bind();
        ibo.render(override_mode);
    }

    void render(GLsizei count, const GLvoid* offset)
    {
        bind();
        ibo.render(count, offset);
    }

    void instanced_render(GLsizei primcount)
    {
        bind();
        ibo.instanced_render(primcount);
    }

    void render_points()
    {
        bind();
        vbo.render(GL_POINTS);
    }

};

//=======================================================================================================================================================================================================================
// Auxiliary vertex array object used for stencil shadows:
//  - uses the same attribute buffer as the main VAO reading position attribute only
//  - has its own index buffer carrying face adjacency information
//  - should be used in pair with geometry shader accepting triangles_adjacency input primitive
//  - possible ibo modes are GL_TRIANGLES_ADJACENCY and GL_TRIANGLE_STRIP_ADJACENCY
//  - in case of strips primitive restart must be enabled and primitive restart index set to -1
//=======================================================================================================================================================================================================================

struct adjacency_vao_t
{
    GLuint id;
    ibo_t ibo;
     adjacency_vao_t() : id(0) {};
    ~adjacency_vao_t() 
        { glDeleteVertexArrays(1, &id); }

    void render()
        { glBindVertexArray(id); ibo.render(); }

    void render(GLsizei count, const GLvoid* offset)
        { glBindVertexArray(id); ibo.render(count, offset); }

    void instanced_render(GLsizei primcount)
        { glBindVertexArray(id); ibo.instanced_render(primcount); }

};

#endif // __vao_included_43026861862561872462458746024956097046805382424868922784
