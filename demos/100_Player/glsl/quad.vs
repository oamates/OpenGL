#version 330 core

const vec4 quad[4] = vec4[4] 
(
    vec4(-1.0f,  1.0f, 0.0f, 0.0f),
    vec4(-1.0f, -1.0f, 0.0f, 1.0f),
    vec4( 1.0f,  1.0f, 1.0f, 0.0f),
    vec4( 1.0f, -1.0f, 1.0f, 1.0f)
);

out vec2 uv;

void main()
{
    vec4 vquad = quad[gl_VertexID];
    gl_Position = vec4(vquad.xy, 0.0f, 1.0f);
    uv = vquad.zw;
}
