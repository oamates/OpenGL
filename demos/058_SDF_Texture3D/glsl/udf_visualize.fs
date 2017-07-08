#version 330 core

uniform usampler3D udf_tex;

in vec3 uvw;

out vec4 FragmentColor;

const float INTEGRAL_SCALE = 268435456.0;
const float INV_INT_SCALE = 1.0 / INTEGRAL_SCALE;

void main()
{
	uint d = texture(udf_tex, uvw).x;

    float q = INV_INT_SCALE * float(d);
    if (q > 0.05) discard;

	float w = exp(-8.0 * abs(q));


    FragmentColor = vec4(w, w, w, 1.0);
}
