#version 330 core

const vec2 uvs[4] = vec2[4] 
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

uniform mat4 camera_matrix;
uniform vec2 focal_scale;

out vec2 uv;
out vec3 view;
flat out vec3 position;

void main()
{
    uv = uvs[gl_VertexID];
    position = vec3(camera_matrix[3]);
    view = vec3(camera_matrix * vec4(focal_scale * uv, -1.0f, 0.0f));
    gl_Position = vec4(uv, 0.0f, 1.0f);
}