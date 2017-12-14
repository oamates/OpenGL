#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "geometry_program.hpp"

void GeometryProgram::ExtractUniforms()
{
    using namespace oglplus;
    // assign program
    material.diffuse.Assign(program);
    material.specular.Assign(program);
    material.shininess.Assign(program);
    material.emissive.Assign(program);
    material.useNormalsMap.Assign(program);
    alphaCutoff.Assign(program);
    matrices.normal.Assign(program);
    matrices.modelViewProjection.Assign(program);
    // bind to uniform name
    material.diffuse.BindTo("material.diffuse");
    material.specular.BindTo("material.specular");
    material.emissive.BindTo("material.emissive");
    material.shininess.BindTo("material.shininess");
    material.useNormalsMap.BindTo("material.useNormalsMap");
    alphaCutoff.BindTo("alphaCutoff");
    matrices.normal.BindTo("matrices.normal");
    matrices.modelViewProjection.BindTo("matrices.modelViewProjection");
}
GeometryProgram::~GeometryProgram()
{
}