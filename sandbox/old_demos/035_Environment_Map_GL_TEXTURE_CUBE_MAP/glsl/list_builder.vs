#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (std430, binding = 0) buffer shader_data
{
    vec4 shift3d[];
};

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_direction;
out vec3 view_direction;
out vec3 light_direction;
out vec2 texture_coords;
out float alpha;


void main(void)
{
    vec4 shift = shift3d[gl_InstanceID];

    position_ws = 356.0f * vec3(shift) + 35.65f * position;
    normal_direction = normal;
    view_direction = camera_ws - position_ws;
    light_direction = light_ws - position_ws;
    texture_coords = uv;
    alpha = shift.w;

    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}
