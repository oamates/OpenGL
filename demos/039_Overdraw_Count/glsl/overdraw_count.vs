#version 430 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec3 uvw_in;

uniform mat4 projection_view_matrix;
uniform float base_size;

void main()
{
    vec3 shift = base_size * vec3(float((gl_InstanceID >> 0) & 15) - 7.5f,
                                  float((gl_InstanceID >> 4) & 15) - 7.5f,
                                  float((gl_InstanceID >> 8) & 15) - 7.5f);

    vec4 position_ws = vec4(shift + position_in, 1.0f);
    gl_Position = projection_view_matrix * position_ws;
}
