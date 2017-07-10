#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 position[];
in vec3 normal[];
in vec3 color_mark[];

uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out float defect;
out vec3 manifold_mark;

void main()
{
    vec3 n = normalize(cross(position[1] - position[0], position[2] - position[0]));

    for(int i = 0; i < 3; ++i)
    {
        position_ws = position[i];
        normal_ws = normal[i];
        defect = 0.5 + 0.5 * dot(normal[i], n);
        manifold_mark = color_mark[i];
        gl_Position = projection_view_matrix * vec4(position[i], 1.0f);        
        EmitVertex();
    }
}