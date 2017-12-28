#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform mat4 model_matrix;
uniform mat4 view_matrix;

out vec3 vertNormal;
out float vd;

void main()
{
    mat4 view_model_matrix = view_matrix * model_matrix;
    gl_Position = view_model_matrix * vec4(position_in, 1.0f);
    vertNormal = mat3(view_model_matrix) * normal_in;
    vd = vertNormal.z;
}
