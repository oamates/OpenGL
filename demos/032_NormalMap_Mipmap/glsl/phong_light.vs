#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float time;
uniform float solid_scale;

uniform vec4 shift_rotor[16];

out vec3 view;
out vec3 light;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;
out vec2 uv;

mat3 rotation_matrix(vec3 axis, float angle)
{
    float sn = sin(angle);
    float cs = cos(angle);

    vec3 axis_cs = (1.0f - cs) * axis;
    vec3 axis_sn = sn * axis;
    
    return mat3(axis_cs.x * axis + vec3(        cs,  axis_sn.z, -axis_sn.y), 
                axis_cs.y * axis + vec3(-axis_sn.z,         cs,  axis_sn.x),
                axis_cs.z * axis + vec3( axis_sn.y, -axis_sn.x,         cs));
}

void main()
{
    int index = 2 * gl_InstanceID;

    vec3 shift = shift_rotor[index + 0].xyz;
    vec4 rotor = shift_rotor[index + 1];
    mat3 rm = rotation_matrix(rotor.xyz, rotor.w * time);

    vec3 position_ws = shift + solid_scale * rm * position_in;

    view = camera_ws - position_ws;
    light = light_ws - position_ws;

    normal = rotation_matrix * normal_in;
    tangent_x = rotation_matrix * tangent_x_in;
    tangent_y = rotation_matrix * tangent_y_in;

    gl_Position = projection_matrix * view_matrix * vec4(position_ws, 1.0);
    uv = uv_in;
}


