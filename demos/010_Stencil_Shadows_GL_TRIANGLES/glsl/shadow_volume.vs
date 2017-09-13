#version 330 core

layout (location = 0) in vec3 position_in;                                             
layout (location = 1) in vec3 normal_in;                                             

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;

const float bias = 0.03125f;

out float dp;
out vec4 vl;
out vec4 vi;

void main()                                                                         
{
    vec3 p = position_in - bias * normal_in;
    vec3 l = p - light_ws;
    dp = dot(l, normal_in);

    vl = projection_view_matrix * vec4(p, 1.0f);
    vi = projection_view_matrix * vec4(l, 0.0f);
}
