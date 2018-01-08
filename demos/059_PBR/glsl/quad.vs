#version 330 core

out vec2 uv;

const vec2 ndcs[4] = vec2[4]
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

void main()
{
    vec2 ndc = ndcs[gl_VertexID];
    uv = 0.5f + 0.5f * ndc;
    gl_Position = vec4(ndc, 0.0f, 1.0f);
}
