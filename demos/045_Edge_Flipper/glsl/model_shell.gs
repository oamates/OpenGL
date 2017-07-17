#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 position[];
in vec3 normal[];

uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out float defect;

const float inv_pi = 0.31830988618;

void main()
{
	//    vec3 n = normalize(cross(position[1] - position[0], position[2] - position[0]));

    vec3 ab = normalize(position[1] - position[0]);
    vec3 bc = normalize(position[2] - position[1]);
    vec3 ca = normalize(position[0] - position[2]);



    float cosines[3];

    cosines[0] = inv_pi * acos(-dot(ca, ab));
    cosines[1] = inv_pi * acos(-dot(ab, bc));
    cosines[2] = inv_pi * acos(-dot(bc, ca));    	


    for(int i = 0; i < 3; ++i)
    {
        position_ws = position[i];
        normal_ws = normal[i];
        defect = (cosines[i] > 0.984375) ? 1.0 : 0.0;
        gl_Position = projection_view_matrix * vec4(position[i], 1.0f);        
        EmitVertex();
    }
}