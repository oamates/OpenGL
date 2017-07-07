#version 330 core

uniform sampler3D udf_tex;

in vec3 uvw;

out vec4 FragmentColor;

void main()
{
	float d = texture(udf_tex, uvw).x;

	float q = exp(-16.0 * d);
    FragmentColor = vec4(q, q, 0.0, 1.0);
}
