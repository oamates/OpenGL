#version 330 core

in vec3 position_ws;

out vec4 FragmentColor;

in float vertexID;

void main()
{

	vec4 rnd_color = vec4(abs(cos(0.475677 * vertexID)),
					      abs(cos(0.343377 * vertexID)),
					      abs(cos(0.665531 * vertexID)),
					      1.0f);

	FragmentColor = rnd_color;
};

