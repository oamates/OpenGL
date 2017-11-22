#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

out vec3 view_ts;
out vec3 light_ts;
out vec2 uv;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;

uniform vec3 light_ws;
uniform vec3 camera_ws;

void main()
{
    vec4 position_ws4 = model_matrix * vec4(position_in, 1.0);
    vec3 position_ws = vec3(position_ws4);

    vec3 T = normalize(mat3(model_matrix) * tangent_x_in);
    vec3 B = normalize(mat3(model_matrix) * tangent_y_in);
    vec3 N = normalize(mat3(model_matrix) * normal_in);

    mat3 TBN = transpose(mat3(T, B, N));

    view_ts = TBN * (camera_ws - position_ws);
    light_ts = TBN * (light_ws - position_ws);

    uv = uv_in;
    gl_Position = projection_view_matrix * position_ws4;
}
