#version 330 core

layout (location = 0) in vec3 position_in;                                             
layout (location = 1) in vec3 normal_in;                                             

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;
const float bias = 0.0425f;


invariant out vec3 position_ws;
invariant out vec3 normal_ws;
invariant out float dp;
invariant out vec4 vl;
invariant out vec4 vi;

                                                                                    
void main()                                                                         
{                                                                                   
    vec4 position_ws4 = model_matrix * vec4(position_in, 1.0f);
    normal_ws = mat3(model_matrix) * normal_in;
    position_ws = vec3(position_ws4);

    vec3 l = normalize(position_ws - light_ws);
    dp = dot(l, normal_ws);

    vl = projection_view_matrix * vec4(position_ws + bias * l, 1.0f);
    vi = projection_view_matrix * vec4(l, 0.0f);
}
