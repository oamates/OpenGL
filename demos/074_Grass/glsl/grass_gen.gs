#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 7) out; 
 
uniform mat4 projection_view_matrix;

in vec3 position_ws[];
in vec3 axis_x[];
in vec3 axis_z[];
in float height[];

const float w = 0.0057;

const vec3 position_ms[7] = vec3[7]
(
    vec3(       -w, 0.0, 0.000),
    vec3(        w, 0.0, 0.000),
    vec3(-0.95 * w, 0.0, 0.057),
    vec3( 0.95 * w, 0.0, 0.057),
    vec3(-0.75 * w, 0.0, 0.124),
    vec3( 0.75 * w, 0.0, 0.124),
    vec3(      0.0, 0.0, 0.207)
);

out vec3 position;
out vec3 normal;

void main()
{
    vec3 axis_y = cross(axis_z[0], axis_x[0]);
    mat3 model_matrix = mat3(axis_x[0], axis_y, axis_z[0]);

    for (int i = 0; i < 7; ++i)
    {
        position = position_ws[0] + height[0] * (model_matrix * position_ms[i]);
        normal = axis_y;
        gl_Position = projection_view_matrix * vec4(position, 1.0f);
        EmitVertex();
    }
    EndPrimitive();
}