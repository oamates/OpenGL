#version 330 core

layout (location = 0) out vec4 normal_ws_out;

uniform vec3 camera_ws;
in vec3 position_ws;
in vec3 normal_ws;

void main()
{    
    normal_ws_out = vec4(normal_ws, length(position_ws - camera_ws));
}