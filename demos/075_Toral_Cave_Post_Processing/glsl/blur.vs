#version 330 core                                                                        

layout (location = 0) in vec2 position; 

out vec2 uv;

void main()
{          
    gl_Position = vec4(position, 0.0f, 1.0f);
    uv = 0.5f * position + vec2(0.5f);
}
