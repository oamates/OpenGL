#version 330 core

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightColor;

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 light_color;

void main()
{           
    FragmentColor = vec4(light_color, 1.0);
    float brightness = dot(FragmentColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = FragmentColor;
}