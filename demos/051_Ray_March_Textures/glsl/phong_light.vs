#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
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

out vec3 position_ms;
out vec3 camera_ms;
out vec3 position;
out vec3 normal;
out vec3 tangent_x;
out vec3 tangent_y;
out vec2 uv;
//out int instance_id;


void main()
{
    position_ms = position_in;
    int index = buffer_base + gl_InstanceID;
    vec3 shift = vec3(data[index].shift);
    vec3 rotation_axis = vec3(data[index].rotor);
    float rotation_angle = data[index].rotor.w * time;
    float cs = cos(rotation_angle);
    float sn = sin(rotation_angle);

    // Rodrigues' rotation formula
    position = shift + solid_scale * (cs * position_in + sn * cross(rotation_axis, position_in) + (1.0f - cs) * dot(rotation_axis, position_in) * rotation_axis); 
    normal = cs * normal_in + sn * cross(rotation_axis, normal_in) + (1.0f - cs) * dot(rotation_axis, normal_in) * rotation_axis; 
    tangent_x = cs * tangent_x_in + sn * cross(rotation_axis, tangent_x_in) + (1.0f - cs) * dot(rotation_axis, tangent_x_in) * rotation_axis; 
    tangent_y = cs * tangent_y_in + sn * cross(rotation_axis, tangent_y_in) + (1.0f - cs) * dot(rotation_axis, tangent_y_in) * rotation_axis; 


    //vec3 camera = (camera_ws - shift) / solid_scale;
    vec3 camera = camera_ws;
    camera_ms = cs * camera - sn * cross(rotation_axis, camera) + (1.0f - cs) * dot(rotation_axis, camera) * rotation_axis; 

    gl_Position = projection_view_matrix * vec4(position, 1.0f);
    uv = uv_in;
}


