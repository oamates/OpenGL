#version 330

layout(location = 0) in vec2 uv_in;  

out vec2 uv;  


void main() 
{  
    uv = 0.5 + 0.5 * uv_in;
    gl_Position = vec4(uv_in, 0.0, 1.0);
}