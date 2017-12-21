#version 330 core

in vec4 vertTexCoord;
in vec2 vertChecker;

uniform sampler2D LightTex;

out vec4 FragmentColor;

void main()
{
	vec2 coord = vertTexCoord.st/vertTexCoord.q;
	vec4 t  = texture(LightTex, coord * 0.5 + 0.5);
	float i = (1 + int(vertChecker.x + 10) % 2 + int(vertChecker.y + 10) % 2) % 2;

	vec3 color = vec3(0.1, 0.1, 0.1);
	color += t.rgb * (1.0 - t.a);
	color *= mix(vec3(0.9, 0.9, 1.0), vec3(0.4, 0.4, 0.9), i);

	FragmentColor = vec4(color, 1.0);
}
