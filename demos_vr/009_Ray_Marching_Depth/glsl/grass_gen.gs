#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 7) out; 
 
uniform mat4 projection_view_matrix;

in vec3 axis_x[];
in vec3 axis_z[];

const vec3 position_ms[7] = vec3[7]
(
    vec3(1.0),
    vec3(1.0),
    vec3(1.0),
    vec3(1.0),
    vec3(1.0),
    vec3(1.0),
    vec3(1.0)
);

void main()
{
    vec3 axis_y = cross(axis_z[0], axis_x[0]);
    mat3 model_matrix = mat3(axis_x[0], axis_y, axis_z[0]);

    for (int i = 0; i < 7; ++i)
    {
        vec3 position_ws = model_matrix * position_ms[i];
        gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
        EmitVertex();
    }
    EndPrimitive();
}