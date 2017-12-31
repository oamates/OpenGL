#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

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

void main()
{
    for(int i = 0; i != 3; ++i)
    {
        gl_Position = projection_matrix * view_matrix * gl_in[i].gl_Position;
        g_normal_ws = normal_ws[i];
        g_tangent_x_ws = tangent_x_ws[i];
        g_tangent_y_ws = tangent_y_ws[i];
        g_light = light[i];
        g_view = view[i];
        g_uv = uv[i];
        EmitVertex();
    }
    EndPrimitive();
}
