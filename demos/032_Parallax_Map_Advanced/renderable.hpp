#ifndef RENDERABLE_HPP_INCLUDED
#define RENDERABLE_HPP_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "camera.hpp"


struct Renderable
{
    Renderable() {}
    virtual ~Renderable() {}

    /* Draws in the current active OpenGL context. Assumes the shader is already binded. */
    virtual void draw(const glsl_program_t& program, Camera const& camera) const
    {
        do_draw(program, camera);
    }

    /* Does the actual drawing, once Renderable::draw() checked the shader is ready. */
    virtual void do_draw (const glsl_program_t& program, Camera const& camera) const = 0;
};

#endif // RENDERABLE_HPP_INCLUDED
