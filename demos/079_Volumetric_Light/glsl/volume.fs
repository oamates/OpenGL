#version 330 core

in vec4 geomTexCoord;

uniform sampler2D LightTex;
uniform int SampleCount;

out vec4 fragColor;

void main()
{
	vec2 coord = geomTexCoord.st / geomTexCoord.q;
	float depth = geomTexCoord.z;

	if(depth < 0.0)
        discard;

	vec4 t  = texture(LightTex, coord * 0.5 + 0.5);
	if(t.a == 0.0) discard;
	float alpha = 10.0*(1.0-t.a)/SampleCount;
	alpha *= (t.r+t.g+t.b)*0.3333;
	alpha /= sqrt(depth);
	fragColor = vec4(t.rgb, alpha);
}
