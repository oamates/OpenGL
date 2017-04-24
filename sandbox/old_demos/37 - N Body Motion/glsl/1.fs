#version 430 core

in vec2 txcoord;

layout(location = 0) out vec4 FragColor;

void main() 
{
   float s = (1.0f / (1.0f + 15.0f * dot(txcoord, txcoord)) - 1.0f / 16.0f);
   FragColor = s * vec4(0.3f, 0.3f, 1.0f, 1.0f);
};
