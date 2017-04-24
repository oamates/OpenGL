#version 330                                                                        

layout (location = 0) in vec2 position; 

uniform vec2 scale_xy;

out vec2 texture_coord;
out vec2 view_ray;

void main()
{          
    gl_Position = vec4(position, 0.0f, 1.0f);
    texture_coord = (position + vec2(1.0f)) / 2.0f;
    view_ray = scale_xy * position;
}
