#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

in vec4 position_ws[];
in vec3 axis_x[];
in vec3 axis_y[];
in vec3 axis_z[];

uniform mat4 projection_view_matrix;
uniform vec3 camera_z;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float time;

const mat4x3 simplex_ms = mat4x3
(
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
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
    7, 2, 1, 4,   // FACE -X
    7, 0, 2, 5,   // FACE -Y
    7, 1, 0, 6,   // FACE -Z
    3, 5, 6, 0,   // FACE  X
    3, 6, 4, 1,   // FACE  Y
    3, 4, 5, 2    // FACE  Z
);

out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;
out vec3 view;
out vec3 light;
out vec2 uv;

const float cube_size = 1.0;

void main()
{
    vec3 rotation_axis = axis_z[0];
    vec3 shift = position_ws[0].xyz;

    float angle = position_ws[0].w * time;
    float cs = cos(angle);
    float sn = sin(angle);

    vec3 axis_X = cs * axis_x[0] + (1 - cs) * dot(axis_z[0], axis_x[0]) * axis_z[0] + sn * cross(axis_z[0], axis_x[0]);
    vec3 axis_Y = cs * axis_y[0] + (1 - cs) * dot(axis_z[0], axis_y[0]) * axis_z[0] + sn * cross(axis_z[0], axis_y[0]);

    float pX = dot(camera_z, axis_X);
    float pY = dot(camera_z, axis_Y);

    if (pX < 0.0) { axis_X = -axis_X; pX = -pX; }
    if (pY < 0.0) { axis_Y = -axis_Y; pY = -pY; }

    vec3 axis_Z = cross(axis_X, axis_Y);
    float pZ = dot(camera_z, axis_Z);

    if (pZ < 0.0)
    {
        axis_Z = -axis_Z;
        vec3 t = axis_X;
        axis_X = axis_Y;
        axis_Y = t;
        pZ = -pZ;
    }

    if ((pY < pX) && (pY < pZ))
    {
        vec3 t = axis_X;
        axis_X = axis_Y;
        axis_Y = axis_Z;
        axis_Z = t;
    }
    else if ((pZ < pX) && (pZ < pY))
    {
        vec3 t = axis_X;
        axis_X = axis_Z;
        axis_Z = axis_Y;
        axis_Y = t;
    }

    mat3 model_matrix = mat3(axis_X, axis_Y, axis_Z);

    mat4x3 simplex_ws = model_matrix * simplex_ms; 

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

    vec3 normals_ws[6] = vec3[](-axis_X, -axis_Y, -axis_Z, axis_X, axis_Y, axis_Z);
    vec3 tangents_x[6] = vec3[]( axis_Z,  axis_X,  axis_Y, axis_Y, axis_Z, axis_X);
    vec3 tangents_y[6] = vec3[]( axis_Y,  axis_Z,  axis_X, axis_Z, axis_X, axis_Y);

    int index = 0;    

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            vec3 position = positions[faces[index]];
            view = camera_ws - position;
            light = light_ws - position;
            uv = square[j];
            normal = normals_ws[i];
            tangent_x = tangents_x[i];
            tangent_y = tangents_y[i];
            gl_Position = projection_view_matrix * vec4(position, 1.0f);
            EmitVertex();
            ++index;
        }
        EndPrimitive();
    }
}




