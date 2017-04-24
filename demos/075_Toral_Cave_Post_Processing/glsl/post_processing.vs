#version 330 core                                                                        

// Night Vision post-processing filter

layout (location = 0) in vec2 position; 
out vec2 uv;

void main()
{          
    gl_Position = vec4(position, 0.0f, 1.0f);
    uv = (position + vec2(1.0f)) / 2.0f;
}

