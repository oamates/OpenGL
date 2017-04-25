#version 430 core

layout (location = 0) in vec2 position; 

out vec2 texture_coord;

void main()
{          
    gl_Position = vec4(position, 0.0f, 1.0f);
    texture_coord = (position + vec2(1.0f)) / 2.0f;
}
