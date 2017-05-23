#version 330 core

const vec2 ndcs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

uniform vec2 focal_scale;

out vec2 uv;
out vec3 view;

void main()
{
    vec2 ndc = ndcs[gl_VertexID];
    view = vec3(focal_scale * ndc, -1.0f);
    uv = 0.5f + 0.5f * ndc;
    gl_Position = vec4(ndc, 0.0f, 1.0f);
}