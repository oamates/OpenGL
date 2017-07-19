#version 330 core

uniform sampler3D udf_tex;

in vec3 uvw;

out vec4 FragmentColor;

void main()
{
	float q = texture(udf_tex, uvw).x;

    if (q > 0.05) discard;

	float w = exp(-4.0 * abs(q));


    FragmentColor = vec4(w, w, w, 1.0);
}
