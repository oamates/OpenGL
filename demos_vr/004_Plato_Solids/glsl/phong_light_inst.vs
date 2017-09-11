#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float time;
uniform float solid_scale;
uniform int buffer_base;

struct motion3d_t
{
    vec4 shift;
    vec4 rotor;
};

layout (std430, binding = 0) buffer shader_data
{
    motion3d_t data[];
};

out vec3 view;
out vec3 light;
out vec3 tangent_x;
out vec3 tangent_y;
out vec3 normal;
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
    int index = buffer_base + gl_InstanceID;
    vec4 shift_vector = data[index].shift;
    vec3 rotation_axis = vec3(data[index].rotor);
    float angular_rate = data[index].rotor.w;
    mat3 rotation = rotation_matrix(rotation_axis, angular_rate * time);

    vec4 position_ws = shift_vector + vec4(solid_scale * rotation * position_in, 1.0f);

    view = vec3(camera_ws - position_in);
    light = vec3(light_ws - position_in);

    tangent_x = rotation * tangent_x_in;
    tangent_y = rotation * tangent_y_in;
    normal = rotation * normal_in;
    
    gl_Position = projection_view_matrix * position_ws;
    uv = uv_in;
}