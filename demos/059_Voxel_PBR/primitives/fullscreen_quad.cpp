#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "fullscreen_quad.hpp"

#include <oglplus/vertex_attrib.hpp>
#include <oglplus/context.hpp>

void FullscreenQuad::Load()
{
    using namespace oglplus;

    if (loaded)
        return;

    static const std::array<float, 20> fsQuadVertexBufferData =                 // data for fs quad
    {
         1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 0.0f, 0.0f,
    };
    
    static const std::array<unsigned int, 6> indexData = { 0, 1, 2, 2, 1, 3 };  // data for element buffer array
    
    fsQuadVertexArray.Bind();                                                   // bind vao for full screen quad
    fsQuadVertexBuffer.Bind(Buffer::Target::Array);
    Buffer::Data(Buffer::Target::Array, fsQuadVertexBufferData);                // set up attrib points
    
    VertexArrayAttrib(VertexAttribSlot(0)).Enable().Pointer(3, DataType::Float, false, 5 * sizeof(float), reinterpret_cast<const GLvoid *>(0));
    VertexArrayAttrib(VertexAttribSlot(1)).Enable().Pointer(2, DataType::Float, false, 5 * sizeof(float), reinterpret_cast<const GLvoid *>(12));
    
    fsQuadElementBuffer.Bind(Buffer::Target::ElementArray);                     // bind and fill element array
    Buffer::Data(Buffer::Target::ElementArray, indexData);
    NoVertexArray().Bind();                                                     // unbind vao
    loaded = true;                                                              // mesh loaded
}

void FullscreenQuad::DrawElements() const
{
    static oglplus::Context gl;
    fsQuadVertexArray.Bind();
    gl.DrawElements(oglplus::PrimitiveType::Triangles, 6, oglplus::DataType::UnsignedInt);
}