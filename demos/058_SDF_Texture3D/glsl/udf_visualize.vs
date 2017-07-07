#version 330 core

uniform mat4 projection_view_matrix;

uniform int mask;
uniform int shift;

out vec3 uvw;

const float scale = 0.5f;

void main()
{
    int id = gl_VertexID;

    ivec3 Pi = ivec3(gl_VertexID & mask,
                    (gl_VertexID >> shift) & mask,
                    (gl_VertexID >> (shift + shift)) & mask);

    uvw = vec3(Pi) / vec3(mask);

    vec3 position = scale * (uvw - 0.5);

    gl_Position = projection_view_matrix * vec4(position, 1.0f);
}