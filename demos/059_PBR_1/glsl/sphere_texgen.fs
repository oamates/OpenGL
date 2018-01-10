#version 330 core

in vec2 uv;

uniform sampler2D texture_in;

const float pi = 3.14159265359f;
out vec3 color;

void main()
{
    float q = sin(pi * uv.y);
    float inv_q = 1.0f / q;
    color = textureGrad(texture_in, uv, inv_q * dFdx(uv), dFdy(uv)).rgb;
}
