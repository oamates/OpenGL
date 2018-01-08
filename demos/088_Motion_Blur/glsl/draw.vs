#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 0) in vec3 normal_in;
layout(location = 0) in vec2 uv_in;

uniform mat4 projection_view_matrix;

layout (std140) uniform model_matrices {
   mat4 model_matrix[512];
};

const vec3 light_ws = vec3(0.0, 0.0, 0.0);

out vec3 normal_ws;
out vec3 light;
out vec3 color;
out vec2 uv;

void main()
{
    mat4 model_matrix0 = model_matrix[gl_InstanceID];
    vec4 position_ws4 = model_matrix0 * vec4(position_in, 1.0f);
    normal_ws = mat3(model_matrix0) * normal_in;
    light = light_ws - position_ws4.xyz;
    uv = uv_in;
    color = abs(normalize(position_ws4.yxz));
    gl_Position = projection_view_matrix * position_ws4;
}
