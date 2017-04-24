#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 24) out; 
 
uniform mat4 projection_matrix;
uniform mat4 view_matrix;

const mat4 simplex_ms = mat4
(
    vec4(-1.0f, -1.0f,  1.0f, 0.0f),
    vec4( 1.0f, -1.0f, -1.0f, 0.0f),
    vec4(-1.0f,  1.0f, -1.0f, 0.0f),
    vec4( 1.0f,  1.0f,  1.0f, 0.0f)
);

const vec2[4] square = vec2[](vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f));

const int faces[24] = int[]
(
    7, 1, 0, 6, 
    1, 4, 6, 3, 
    3, 5, 6, 0, 
    4, 2, 3, 5, 
    1, 7, 4, 2, 
    0, 5, 7, 2
);


in mat4 model_matrix[];

out vec4 position;
out vec4 color;
out vec4 normal;
out vec4 tangent_x;
out vec4 tangent_y;
out vec4 view_direction;
out vec2 texture_coord;

void main()
{
    mat4 frame = model_matrix[0];
    mat4 simplex_ws = frame * simplex_ms;  

    vec4 shift = frame[3];
    vec4 view_point = view_matrix[3];

    vec4 vertices[] = vec4[]
    (
        shift + simplex_ws[0], shift + simplex_ws[1], shift + simplex_ws[2], shift + simplex_ws[3],
        shift - simplex_ws[0], shift - simplex_ws[1], shift - simplex_ws[2], shift - simplex_ws[3]
    );

    vec4 normals[6]    = vec4[](-frame[2], frame[2],-frame[0], frame[0],-frame[1], frame[1]);
    vec4 tangents_x[6] = vec4[]( frame[1], frame[0], frame[2], frame[1], frame[0], frame[2]);
    vec4 tangents_y[6] = vec4[]( frame[0], frame[1], frame[1], frame[2], frame[2], frame[0]);

    int index = 0;    

    vec4 c = normalize(shift);
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            color = abs(c);
            position = vertices[faces[index]];
            view_direction = view_point - position;
            gl_Position = projection_matrix * view_matrix * position;
            texture_coord = square[j];
            normal = normals[i];
            tangent_x = tangents_x[i];
            tangent_y = tangents_y[i];
            EmitVertex();
            ++index;
        }
        EndPrimitive();
    }
}




