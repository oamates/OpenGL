#version 330 core

uniform sampler2D scene_texture;

in vec2 uv;

out vec4 FragmentColor;

void main ()
{
	FragmentColor = texture(scene_texture, uv);
}
