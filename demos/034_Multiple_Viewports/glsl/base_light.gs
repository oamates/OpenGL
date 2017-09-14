#version 410 core

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix[4];
uniform vec3 camera_ws[4];
uniform vec3 light_ws;

in VS_GS_VERTEX
{
    vec3 position;
    vec3 normal;
    vec3 tangent_x;
    vec3 tangent_y;
    vec2 uv;
} vertex_in[];

out GS_FS_VERTEX
{
    vec3 view_direction;
    vec3 light_direction;
    vec3 normal_direction;
    vec3 tangent_x_direction;
    vec3 tangent_y_direction;
    vec2 texture_coord;
} vertex_out;

void main()
{
    for (int i = 0; i < 3; i++)
    {
        gl_ViewportIndex = gl_InvocationID;
        vertex_out.view_direction = camera_ws[gl_InvocationID] - vertex_in[i].position;
        vertex_out.light_direction = light_ws - vertex_in[i].position;
        vertex_out.normal_direction    = vertex_in[i].normal;
        vertex_out.tangent_x_direction = vertex_in[i].tangent_x;
        vertex_out.tangent_y_direction = vertex_in[i].tangent_y;        
        vertex_out.texture_coord = vertex_in[i].uv;
        gl_Position = projection_matrix * view_matrix[gl_InvocationID] * vec4(vertex_in[i].position, 1.0f);
        EmitVertex();
    }
}
