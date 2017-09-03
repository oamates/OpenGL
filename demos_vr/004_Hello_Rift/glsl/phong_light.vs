#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent_x;
layout(location = 3) in vec3 tangent_y;
layout(location = 4) in vec2 uv;

uniform mat4 projection_view_matrix;
uniform vec4 camera_ws;
uniform vec4 light_ws;
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


out vec3 view_direction;
out vec3 light_direction;
out vec3 normal_direction;
out vec3 tangent_x_direction;
out vec3 tangent_y_direction;
out vec2 texture_coord;

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
    vec4 shift_vector = data[index].shift;
    vec3 rotation_axis = vec3(data[index].rotor);
    float angular_rate = data[index].rotor.w;

    mat3 rotation_matrix = compute_rotation_matrix(rotation_axis, angular_rate * time);
    vec4 position_ws = shift_vector + vec4(solid_scale * rotation_matrix * vertex, 1.0f);

    view_direction = vec3(camera_ws - position_ws);
    light_direction = vec3(light_ws - position_ws);

    normal_direction    = rotation_matrix * normal;
    tangent_x_direction = normalize(rotation_matrix * tangent_x);
    tangent_y_direction = normalize(rotation_matrix * tangent_y);

	gl_Position = projection_view_matrix * position_ws;
	texture_coord = uv;
}


