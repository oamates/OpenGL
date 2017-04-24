#version 330 core

layout(location = 0) in vec4 value_in;

uniform mat4 projection_view_matrix;

const int n = 512;
const int m = 9;
const int mask = n - 1;
const vec2 center = vec2(0.5f * mask);

out vec3 position_ws;
out vec2 uv;

void main()
{
    uv = vec2(gl_VertexID & mask, gl_VertexID >> m) - center;

    position_ws = vec3(64.0 * uv, 0.5 * value_in.x + 512.0);
    vec4 position_ws4 = vec4(position_ws, 1.0f);

    gl_Position = projection_view_matrix * position_ws4;
}