#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

struct motion3d_t
{
    vec4 shift;
    vec4 rotor;
};

layout (std430, binding = 0) buffer shader_data
{
    motion3d_t data[];
};

uniform mat4 projection_view_matrix;
uniform int buffer_base;
uniform float time;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 tangent_x_ws;
out vec3 tangent_y_ws;
out vec2 uv;

mat3 compute_rotation_matrix(vec3 axis, float angle)
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
    int index = buffer_base + gl_InstanceID;
    vec3 shift_vector = vec3(data[index].shift);
    vec3 rotation_axis = vec3(data[index].rotor);
    float angular_rate = data[index].rotor.w;
    mat3 rotation_matrix = compute_rotation_matrix(rotation_axis, angular_rate * time);
    position_ws = shift_vector + rotation_matrix * position_in;

    normal_ws    = rotation_matrix * normal_in;
    tangent_x_ws = rotation_matrix * tangent_x_in;
    tangent_y_ws = rotation_matrix * tangent_y_in;

    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);

    uv = uv_in;
}


