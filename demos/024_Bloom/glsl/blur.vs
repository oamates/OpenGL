#version 330 core

out vec2 uv;

const vec2 quad[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);
 
void main()
{
    vec2 vquad = quad[gl_VertexID];
    gl_Position = vec4(vquad, 0.0f, 1.0f);
    uv = 0.5f + 0.5f * vquad;
}