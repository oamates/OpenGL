#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent_x;
layout(location = 3) in vec3 tangent_y;
layout(location = 4) in vec2 uv;



struct motion3d_t
{
    vec4 shift;
    vec4 rotor;
};

layout (std430, binding = 0) buffer shader_data
{
    motion3d_t data[];
};

uniform int buffer_base;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform float time;

out vec4 position_ms;
out vec4 position_ws;
out vec3 view_direction;
out vec3 normal_ws;
out vec3 tangent_x_ws;
out vec3 tangent_y_ws;
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
	position_ms = vec4(position, 1.0f);

    int index = buffer_base + gl_InstanceID;
    vec4 shift_vector = data[index].shift;
    vec3 rotation_axis = vec3(data[index].rotor);
    float angular_rate = data[index].rotor.w;
    mat3 rotation_matrix = compute_rotation_matrix(rotation_axis, angular_rate * time);
    position_ws = shift_vector + vec4(rotation_matrix * position, 1.0f);

	vec4 camera_ws = view_matrix[3];

    view_direction = vec3(camera_ws - position_ws);

    normal_ws    = rotation_matrix * normal;
    tangent_x_ws = rotation_matrix * tangent_x;
    tangent_y_ws = rotation_matrix * tangent_y;

	gl_Position = projection_matrix * view_matrix * position_ws;

	texture_coord = uv;
};


