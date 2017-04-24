#version 330                                                                        

layout(binding = 4) uniform samplerCube triangle_image0;
layout(binding = 5) uniform samplerCube triangle_image1;
layout(binding = 6) uniform samplerCube triangle_image2;
layout(binding = 7) uniform samplerCube triangle_image3;

in vec3 colorf;
in vec3 texcoordf;

out vec4 FragmentColor;


uniform float time;
flat in int instanceID;


vec4 texget(vec3 tc, int instance_id)
{
	int q = (int(72185 * sin(72185 * instance_id))) & 0x3; 
	if (q == 0) return texture(triangle_image0, tc);
	if (q == 1) return texture(triangle_image1, tc);
	if (q == 2) return texture(triangle_image2, tc);
	return texture(triangle_image3, tc);	
}


void main()
{
	vec4 v = texget(texcoordf, instanceID);

	float t = (time + 50 * instanceID);
	float cs = cos(t);
	float sn = sin(t);

	v = vec4(v.x * cs - v.z * sn, v.y, v.x * sn + v.z * cs, v.w);
	v = vec4(v.x * cs - v.y * sn, v.x * sn + v.y * cs, v.z, v.w);
	v = vec4(v.x, v.y * cs - v.z * sn, v.y * sn + v.z * cs, v.w);

//	FragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	FragmentColor = abs(v);
}