#version 410 core

in vec3 vertNormal;

out vec4 FragmentColor;

void main(void)
{
	vec3 color = vertNormal;
	if (!all(equal(color, abs(color))))
		color = vec3(1.0) - abs(color);
	FragmentColor = vec4(color, 1.0);
}