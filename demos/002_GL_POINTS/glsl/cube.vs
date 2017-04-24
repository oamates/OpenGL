#version 330 core

layout(location = 0) in vec3 position_in;  
layout(location = 1) in vec3 normal_in;    

uniform mat4 projection_view_matrix;
uniform float box_size;
uniform float time;

out vec3 color;

vec2 hash(vec2 n)
    { return fract(cos(n) * 753.5453123); }

void main()
{

    vec2 h = hash(vec2(gl_InstanceID, gl_VertexID));

    float cs = cos(0.125f * time + h.x);
    float sn = sin(0.125f * time - h.y);
    vec3 p = position_in * cs + normal_in * sn;

    const float d = 32.0f;
    float s = pow(dot(pow(abs(p), vec3(d)), vec3(1.0f)), 1.0 / d);

    float m = 1.03125f * box_size / s; 

    vec3 position_ws = m * p;
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);

    color = vec3(0.85, 0.65, 0.25) + 0.45 * p;
}