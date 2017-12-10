#version 330 core

out vec3 view;

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

uniform mat3 camera_matrix;
 
void main()
{
    vec2 q = uvs[gl_VertexID];
    gl_Position = vec4(q, 0.0f, 1.0f);
    view = camera_matrix * vec3(q, 1.0);
}
