#version 400 core

in int gl_VertexID;

const vec2 rectangle[] = vec2[4]
(
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f)
);

out vec2 uv;
                                                                                    
void main()                                                                         
{
    uv = rectangle[gl_VertexID];
    gl_Position = vec4(2.0f * uv - 1.0f, 0.0f, 1.0f);                                                                                   
}