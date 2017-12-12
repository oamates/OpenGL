#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 color;
uniform vec3 camera_ws;

out vec4 FragmentColor;


const float pi = 3.14159265359f;

void main()
{		
    
    FragmentColor = vec4(color, 1.0f);
}
