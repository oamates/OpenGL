#version 330 core

layout(location = 0) in vec4 value_in;

uniform mat4 projection_view_matrix;

const int m = 11;
const int mask = (1 << m) - 1;
const int n = 1 << m;

out vec3 position_ws;
out vec2 uv;

void main()
{
    uv = vec2(gl_VertexID & mask, gl_VertexID >> m) / float(n);

    position_ws = vec3(4096.0 * (uv - vec2(0.5)), 0.007625 * value_in.x);
    vec4 position_ws4 = vec4(position_ws, 1.0f);

    gl_Position = projection_view_matrix * position_ws4;
}