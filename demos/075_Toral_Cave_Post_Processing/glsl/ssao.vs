#version 330 core                                                                        

layout (location = 0) in vec2 position; 

uniform vec2 scale_xy;

out vec2 uv;
out vec2 view_ray;

void main()
{          
    gl_Position = vec4(position, 0.0f, 1.0f);
    uv = 0.5f * position + vec2(0.5f);
    view_ray = scale_xy * position;
}
