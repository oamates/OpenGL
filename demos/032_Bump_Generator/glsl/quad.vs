#version 330 core

uniform vec4 quad;
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
    vec2 screen_xy = mix(quad.xy, quad.zw, uv.xy);
    gl_Position = vec4(screen_xy, 0.0f, 1.0f);
}