#version 430 core

uniform float time;

in float density;

out vec4 FragmentColor;


void main()
{
	
	vec3 point_color;
	
	if ((density >= -0.05f) && (density <= 0.05f))
		point_color = vec3(1.0f);
	else
		point_color = vec3(0.0f);

    FragmentColor = vec4(point_color, 1.0f);

};
