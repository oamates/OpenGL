#ifndef RENDERABLE_HPP_INCLUDED
#define RENDERABLE_HPP_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "camera.hpp"


struct Renderable
{
    Renderable() {}
    virtual ~Renderable() {}
    virtual void render(const glsl_program_t& program, Camera const& camera) const = 0;
};

#endif // RENDERABLE_HPP_INCLUDED
