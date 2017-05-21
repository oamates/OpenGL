#version 330 core

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

uniform int draw_mode = 1;

in vec2 TexCoord;

layout (location = 0) out vec4 FragmentColor;

void main(void)
{
	vec4 color = 1.0 * texture(texture0, TexCoord);
	float ao = texture(texture1, TexCoord).r;
	ao = pow(ao, 1.37);

	if (draw_mode == 0)
		FragmentColor = vec4(color.rgb * ao, color.a);
	else
		FragmentColor = vec4(vec3(ao), color.a);
}