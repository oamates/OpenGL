#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "blur_program.hpp"


void BlurProgram::ExtractUniforms()
{
    blurDirection.Assign(program);
    blurDirection.BindTo("blurDirection");
    source.Assign(program);
    source.BindTo("source");
    blurType.Assign(program);
    blurType.BindTo("blurType");
}

BlurProgram::BlurProgram()
{
}


BlurProgram::~BlurProgram()
{
}
