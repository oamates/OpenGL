#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float solid_scale;
uniform float time;
uniform int buffer_base;

struct motion3d_t
{
    vec3 shift;
    float hue;
    vec3 axis;
    float angular_velocity;
};

layout (std430, binding = 0) buffer shader_data
{
    motion3d_t data[];
};

out vec3 position_ws;
out vec3 normal_ws;
out vec3 tangent_x;
out vec3 tangent_y;
out vec2 uv;
out vec3 view;
out vec3 light;
out float hue;

mat3 compute_rotation_matrix(vec3 axis, float angle)
{
    // Rodrigues' formula for rotation in 3-space

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
    mat3 rotation_matrix = compute_rotation_matrix(data[index].axis, 0.25f * data[index].angular_velocity * time);
    position_ws = data[index].shift + solid_scale * rotation_matrix * position_in;

    view = camera_ws - position_ws;
    light = light_ws - position_ws;

    normal_ws = rotation_matrix * normal_in;
    tangent_x = normalize(rotation_matrix * tangent_x_in);
    tangent_y = normalize(rotation_matrix * tangent_y_in);

    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
    uv = uv_in;
    hue = data[index].hue;
}



