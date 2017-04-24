#version 400                                                                        

layout(location = 0) in vec3 position_in;

out vec3 position;

uniform vec3 shift;
uniform mat4 projection_view_matrix;

void main()
{
    position = position_in;
    gl_Position = projection_view_matrix * vec4(50.0f * shift + 40.0f * position_in, 1.0f);
}