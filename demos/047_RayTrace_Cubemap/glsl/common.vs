#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 model_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out gl_PerVertex
{
   vec4 gl_Position;
};

out vec3 normal_ws;
out vec3 tangent_x_ws;
out vec3 tangent_y_ws;
out vec3 light;
out vec3 view;
out vec2 uv;

void main()
{
	gl_Position = model_matrix * vec4(position_in, 1.0f);

    light = light_ws - gl_Position.xyz;
    view = camera_ws - gl_Position.xyz;
    normal_ws = mat3(model_matrix) * normal_in;
    tangent_x_ws = mat3(model_matrix) * tangent_x_in;
    tangent_y_ws = mat3(model_matrix) * tangent_y_in;
    uv = uv_in;
}
