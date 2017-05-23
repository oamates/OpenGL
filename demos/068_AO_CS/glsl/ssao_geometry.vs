#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;

out vec4 normal_cs;

uniform mat4 view_model_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_vm_matrix;

void main()
{
    vec4 position_cs4 = view_model_matrix * vec4(position_in, 1.0f);
    gl_Position = projection_matrix * position_cs4;
    normal_cs = vec4(normal_vm_matrix * normal_in, position_cs4.z);
}