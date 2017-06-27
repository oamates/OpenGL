#version 330 core

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

void main(void)
{
    gl_Position = vec4(uvs[gl_VertexID], 0.0f, 1.0f);
}
