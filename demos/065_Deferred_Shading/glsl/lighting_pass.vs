#version 330 core

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

out vec2 uv;

void main()
{
    vec2 q = uvs[gl_VertexID];
    uv = 0.5 + 0.5 * q;
    gl_Position = vec4(q, 0.0f, 1.0f);
}