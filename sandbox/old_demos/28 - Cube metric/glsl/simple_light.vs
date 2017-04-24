#version 430 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent_x;
layout(location = 3) in vec3 tangent_y;
layout(location = 4) in vec2 uv;

layout(binding = 0) uniform modelpositions
{
	vec4 model_positions[1000];
};

uniform float global_time;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec4 position_ws;
out vec3 view_direction;
out vec3 normal_direction;
out vec3 tangent_x_direction;
out vec3 tangent_y_direction;

out vec2 texture_coord;


vec3 rand3d (int n)
{
	return normalize(vec3(sin(224662.77 + 15.1 * n), cos(22.73921 + 912.3 * n), sin(1278.9353 + 5243.57 * n)));
};

mat3 rotationMatrix(int n, float t)
{
    vec3 axis = rand3d (n);
    float s = sin(t);
    float c = cos(t);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c          );
};

void main()
{
	mat3 rotation_matrix = rotationMatrix(gl_InstanceID, global_time);

    position_ws = model_positions[gl_InstanceID] + vec4(rotation_matrix * position_ms, 0.0f);
	vec4 camera_ws = view_matrix[3];

    view_direction = vec3(camera_ws - position_ws);

    normal_direction    = rotation_matrix * normal;
    tangent_x_direction = normalize(rotation_matrix * tangent_x);
    tangent_y_direction = normalize(rotation_matrix * tangent_y);

	gl_Position = projection_matrix * view_matrix * position_ws;

	texture_coord = uv;
};


