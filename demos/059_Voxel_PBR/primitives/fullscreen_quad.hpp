#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../scene/mesh.hpp"

#include "vertex.hpp"

const vertex_pt2_t fullscreen_quad[4] = 
{
    vertex_pt2_t(glm::vec3( 1.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f)),
    vertex_pt2_t(glm::vec3(-1.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f)),
    vertex_pt2_t(glm::vec3( 1.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f)),
    vertex_pt2_t(glm::vec3(-1.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f))
};

const GLuint quad_indices[] = { 0, 1, 2, 2, 1, 3 };


struct FullscreenQuad : public MeshDrawer
{
    GLuint vao_id, vbo_id, ibo_id;

    FullscreenQuad() : vao_id(0), vbo_id(0), ibo_id(0) {}

    ~FullscreenQuad()
    {
        if (vao_id != 0)
        {
            glDeleteBuffers(1, &ibo_id);
            glDeleteBuffers(1, &vbo_id);
            glDeleteVertexArrays(1, &vao_id);
        }
    }

    void Load() override
    {
        if (loaded) return;

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_quad), fullscreen_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pt2_t), (const GLvoid*) offsetof(vertex_pt2_t, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_pt2_t), (const GLvoid*) offsetof(vertex_pt2_t, uv));

        glGenBuffers(1, &ibo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices    , GL_STATIC_DRAW);

        glBindVertexArray(0);
        loaded = true;
    }

    void DrawElements() const override
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

};