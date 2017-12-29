#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[3];

in vec3 normal_ws[];
in vec3 tangent_x_ws[];
in vec3 tangent_y_ws[];
in vec3 light[];
in vec3 view[];
in vec2 uv[];

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec3 g_normal_ws;
out vec3 g_tangent_x_ws;
out vec3 g_tangent_y_ws;
out vec3 g_light;
out vec3 g_view;
out vec2 g_uv;

const mat4 face_matrix[6] = mat4[6]
(
    mat4( 0.0,  0.0, -1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
         -1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 0.0,  0.0,  1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0,  0.0, -1.0,  0.0,
          0.0,  1.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0, -1.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4(-1.0,  0.0,  0.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0,  1.0,  0.0,
          0.0,  0.0,  0.0,  1.0)
);

void main()
{
    for(gl_Layer = 0; gl_Layer != 6; ++gl_Layer)
    {
        mat4 transform_matrix = projection_matrix * face_matrix[gl_Layer] * view_matrix;

        for(int i = 0; i != 3; ++i)
        {
            gl_Position = transform_matrix * gl_in[i].gl_Position;
            g_normal_ws = normal_ws[i];
            g_tangent_x = tangent_x[i];
            g_tangent_y = tangent_y[i];
            g_light = light[i];
            g_view = view[i];
            g_uv = uv[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}