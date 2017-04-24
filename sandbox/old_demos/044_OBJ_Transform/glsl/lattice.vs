#version 330 core

layout(location = 0) in vec3 position;

uniform float scale;
uniform float inv_bound;

uniform mat4 projection_view_matrix;

out vec3 uvw;

void main()
{
    uvw = inv_bound * position;
    gl_Position = projection_view_matrix * vec4(scale * position, 1.0f);
}

