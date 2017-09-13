#version 330 core

layout (location = 0) in vec3 position_in;                                             
layout (location = 1) in vec3 normal_in;                                             

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;
const float bias = 0.0425f;


out vec3 position_ws;
out vec3 normal_ws;
out float dp;
out vec4 vl;
out vec4 vi;

                                                                                    
void main()                                                                         
{                                                                                   
    position_ws = position_in;
    normal_ws = normal_in;

    vec3 l = normalize(position_in - light_ws);
    dp = dot(l, normal_ws);

    vl = projection_view_matrix * vec4(position_in + bias * l, 1.0f);
    vi = projection_view_matrix * vec4(l, 0.0f);
}
