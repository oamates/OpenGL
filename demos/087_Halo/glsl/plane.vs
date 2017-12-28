#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform vec3 light_ws;

out vec3 vertNormal;
out vec3 vertLight;

void main()
{
    gl_Position = projection_matrix * view_matrix * vec4(position_in, 1.0f);
    vertNormal = normal_in;
    vertLight = light_ws - position_in;
}
