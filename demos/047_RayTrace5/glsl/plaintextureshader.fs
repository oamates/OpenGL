#version 410 core

in vec2 uv;

uniform sampler3D texUnit;
out vec4 color;

void main(void)
{
    color = texture(texUnit, vec3(uv, 1.0f));
}
