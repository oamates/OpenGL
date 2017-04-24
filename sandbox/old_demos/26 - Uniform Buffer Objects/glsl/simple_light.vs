#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent_x;
layout(location = 3) in vec3 tangent_y;
layout(location = 4) in vec2 uv;

uniform mat4 model_matrix[343];

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec4 position_ws;
out vec3 view_direction;
out vec3 normal_direction;
out vec3 tangent_x_direction;
out vec3 tangent_y_direction;

out vec2 texture_coord;

flat out int instance_id;


void main()
{
    position_ws = model_matrix[gl_InstanceID] * vec4(vertex, 1.0f);
	vec4 camera_ws = view_matrix[3];

    view_direction = vec3(camera_ws - position_ws);

	mat3 rotation_matrix = mat3(model_matrix[gl_InstanceID]);
    normal_direction    = rotation_matrix * normal;
    tangent_x_direction = normalize(rotation_matrix * tangent_x);
    tangent_y_direction = normalize(rotation_matrix * tangent_y);

	gl_Position = projection_matrix * view_matrix * position_ws;

	texture_coord = uv;
    instance_id = gl_InstanceID;
};


