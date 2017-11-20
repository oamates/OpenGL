#version 400 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;

uniform float time;
uniform float scale;
uniform vec4 shift_rotor[16];

out vec3 position;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;

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

    position = shift + scale * rm * position_in;
    normal = rm * normal_in;
    tangent_x = rm * tangent_x_in;
    tangent_y = rm * tangent_y_in;
}