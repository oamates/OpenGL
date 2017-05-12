#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position_in;
layout(location = 1) in vec3 color_in;

layout(location = 0) out vec3 color;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
    gl_Position = vec4(position_in, 0.0, 1.0);
    color = color_in;
}