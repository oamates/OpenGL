#version 400 core

layout (lines) in;
layout (triangle_strip, max_vertices = 130) out; 
 
uniform mat4 projection_view_matrix;
uniform float radius;

in vec3 position_ms[];
in vec3 tangent_ms[];
in vec3 normal_ms[];
in vec3 binormal_ms[];

out vec3 position;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;

const int N = 64;
const float two_pi = 6.283185307179586476925286766559; 


void main()
{

    for (int i = 0; i <= N; ++i)
    {
        float phi = two_pi * i / N;
        float cs = cos(phi);
        float sn = sin(phi);

        normal = cs * normal_ms[1] + sn * binormal_ms[1];
        position = position_ms[1] + radius * normal;
        tangent_x = tangent_ms[1];
        tangent_y = -sn * normal_ms[1] + cs * binormal_ms[1];
        gl_Position = projection_view_matrix * vec4(position, 1.0f);
        EmitVertex();

        normal = cs * normal_ms[0] + sn * binormal_ms[0];
        position = position_ms[0] + radius * normal;
        tangent_x = tangent_ms[0];
        tangent_y = -sn * normal_ms[0] + cs * binormal_ms[0];
        gl_Position = projection_view_matrix * vec4(position, 1.0f);
        EmitVertex();

    }
    EndPrimitive();
}




