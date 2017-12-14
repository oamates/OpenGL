#pragma once

#include <oglplus/vertex_array.hpp>
#include <oglplus/buffer.hpp>
#include "../scene/mesh.hpp"

struct FullscreenQuad : public MeshDrawer
{
    FullscreenQuad() {}
    ~FullscreenQuad() {}

    void Load() override;
    void DrawElements() const override;

    oglplus::VertexArray fsQuadVertexArray;             // The full screen quad vertex array object
    oglplus::Buffer fsQuadVertexBuffer;                 // The full screen quad vertex buffer
    oglplus::Buffer fsQuadElementBuffer;                // The full screen quad element index buffer
};