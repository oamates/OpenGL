#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 view;
out vec3 light;
out vec3 tangent_x;
out vec3 tangent_y;
out vec3 normal;
out vec2 uv;

void main()
{
    vec4 position_ws = vec4(position_in, 1.0f);

    view = vec3(camera_ws - position_in);
    light = vec3(light_ws - position_in);
    tangent_x = tangent_x_in;
    tangent_y = tangent_y_in;
    normal = normal_in;
    
    gl_Position = projection_view_matrix * position_ws;
    uv = uv_in;
}


