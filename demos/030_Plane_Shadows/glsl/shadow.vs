#version 330

uniform mat4 projection_view_model_matrix;

layout (location = 0) in vec4 position;

void main(void)
{
    gl_Position = projection_view_model_matrix * position;
}
