#version 330 core

out vec2 uv;

const vec2 uvs[4] = vec2[4] 
(
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f)
);

void main(void)
{
    uv = uvs[gl_VertexID];
    gl_Position = vec4(2.0 * uv - 1.0, 0.0f, 1.0f);
}