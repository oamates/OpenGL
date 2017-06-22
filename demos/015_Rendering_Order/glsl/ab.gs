#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

in vec3 position_ws[];
in vec3 axis_x[];
in vec3 axis_y[];
in vec3 axis_z[];

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

const mat4x3 simplex_ms = mat4x3
(
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f,  1.0f)
);

const vec2[4] square = vec2[]
(
    vec2(0.0f, 0.0f), 
    vec2(1.0f, 0.0f), 
    vec2(0.0f, 1.0f), 
    vec2(1.0f, 1.0f)
);

const int faces[24] = int[]
(
    1, 4, 6, 3,
    4, 2, 3, 5,
    3, 5, 6, 0,
    7, 2, 0, 5,
    1, 7, 6, 0,
    4, 2, 1, 7
);

out vec3 position;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;
out vec3 view;
out vec3 light;
out vec2 uv;

const float cube_size = 0.5f;

void main()
{
    mat3 frame = mat3(axis_x[0], axis_y[0], axis_z[0]);

    mat4x3 simplex_ws = frame * simplex_ms; 
    vec3 shift = position_ws[0];

    vec3 positions[8] = vec3[]
    (
        shift + cube_size * simplex_ws[0], 
        shift + cube_size * simplex_ws[1], 
        shift + cube_size * simplex_ws[2], 
        shift + cube_size * simplex_ws[3],
        shift - cube_size * simplex_ws[0], 
        shift - cube_size * simplex_ws[1], 
        shift - cube_size * simplex_ws[2], 
        shift - cube_size * simplex_ws[3]
    );

    vec3 normals[6]    = vec3[](frame[0], frame[1], frame[2],-frame[0],-frame[1],-frame[2]);
    vec3 tangents_x[6] = vec3[](frame[1], frame[2], frame[0], frame[2], frame[0], frame[1]);
    vec3 tangents_y[6] = vec3[](frame[2], frame[0], frame[1], frame[1], frame[2], frame[0]);

    int index = 0;    

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            position = positions[faces[index]];
            view = camera_ws - position;
            light = light_ws - position;
            gl_Position = projection_view_matrix * vec4(position, 1.0f);
            uv = square[j];
            normal = normals[i];
            tangent_x = tangents_x[i];
            tangent_y = tangents_y[i];
            EmitVertex();
            ++index;
        }
        EndPrimitive();
    }
}




