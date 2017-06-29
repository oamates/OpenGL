#version 430

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

layout (std430, binding = 0) buffer shader_data
{
    vec4 shift3d[];
};

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 view;
out vec3 light;
out vec2 uv;
out float alpha;


void main(void)
{
    vec4 shift = shift3d[gl_InstanceID];

    position_ws = 8.0f * shift.xyz + position_in;
    normal_ws = normal_in;
    view = camera_ws - position_ws;
    light = light_ws - position_ws;
    uv = uv_in;
    alpha = shift.w;

    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}
