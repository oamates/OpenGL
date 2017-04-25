#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;


struct light_t
{
    vec4 position;
    vec4 color;    
};

const int NR_LIGHTS = 32;

layout (std140) uniform lights_block
{
    light_t light[NR_LIGHTS];
};

out vec3 color;

void main()
{
    vec3 shift = light[gl_InstanceID].position.xyz;
    color = light[gl_InstanceID].color.rgb;
    gl_Position = projection_view_matrix * vec4(shift + 0.25f * position_in, 1.0f);
}