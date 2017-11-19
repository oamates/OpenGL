#version 400 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;

out vec3 position;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;

void main()
{
    position = position_in;
    normal = normal_in;
    tangent_x = tangent_x_in;
    tangent_y = tangent_y_in;
}