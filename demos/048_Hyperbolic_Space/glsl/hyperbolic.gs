#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 position[3];
in vec3 barycentric_coordinates[3];

out vec3 texture_coordinates;

const mat4 projection_matrix = mat4(vec4(1.154701, 0.000000,  0.000000,  0.000000), 
                                    vec4(0.000000, 1.732051,  0.000000,  0.000000), 
                                    vec4(0.000000, 0.000000, -1.000000, -1.000000), 
                                    vec4(0.000000, 0.000000, -0.200000,  0.000000));


uniform mat4 view_matrix;

void main()
{
    for (unsigned int i = 0; i < 3; ++i)
    {
        vec4 pos = position[i];
        vec4 hyperbolic_position = view_matrix * pos;
        hyperbolic_position.w = 1;
        gl_Position = projection_matrix * hyperbolic_position;
        texture_coordinates = barycentric_coordinates[i];
	    EmitVertex();
    }
    EndPrimitive();
}