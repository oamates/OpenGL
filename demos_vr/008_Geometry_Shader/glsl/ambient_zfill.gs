#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

const int ovrEye_Count = 2;
uniform mat4 projection_view_matrix[ovrEye_Count];

in vec3 position_ws[];
in vec3 normal_ws[];

out vec3 position;
out vec3 normal;

void main()
{
    for (int vp = 0; vp < ovrEye_Count; ++vp)
    {
        gl_ViewportIndex = vp;
        for(int j = 0; j < 3; ++j)
        {
            position = position_ws[j];
            normal = normal_ws[j];
            gl_Position = projection_view_matrix[vp] * vec4(position, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
}


