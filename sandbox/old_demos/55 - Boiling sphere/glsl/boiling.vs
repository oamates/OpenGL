#version 330                                                                        

layout(location = 0) in vec3 spherical_point;


uniform float time;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 position;

//==============================================================================================================================================================
// Helper functions
//==============================================================================================================================================================
vec2 mod289(vec2 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };
vec3 mod289(vec3 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };
vec4 mod289(vec4 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };

float permute(float x) 
	{ return mod((34.0f * x + 1.0f) * x, 289.0f); };
vec3 permute(vec3 x) 
	{ return mod((34.0f * x + 1.0f) * x, 289.0f); };
vec4 permute(vec4 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); };
vec4 taylorInvSqrt(vec4 r) 
	{ return 1.792843f - 0.853735f * r; };

//==============================================================================================================================================================
// Classic Perlin noise
//==============================================================================================================================================================
float snoise(vec3 v)
{
	const vec2 C = vec2(1.0f / 6.0f, 1.0f/3.0f);
	const vec4 D = vec4(0.0f, 0.5f, 1.0f, 2.0f);

	// First corner
	vec3 i = floor(v + dot(v, C.yyy));
	vec3 x0 = v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0f - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy;												// 2.0f * C.x = 1.0f / 3.0f = C.y
	vec3 x3 = x0 - D.yyy;													// -1.0f + 3.0f * C.x = -0.5f = -D.y

	// Permutations
	i = mod289(i);
  	vec4 p = permute( permute(permute(i.z + vec4(0.0f, i1.z, i2.z, 1.0f)) + i.y + vec4(0.0f, i1.y, i2.y, 1.0f)) + i.x + vec4(0.0f, i1.x, i2.x, 1.0f));

	// Gradients: 7x7 points over a square, mapped onto an octahedron. The ring size 17 * 17 = 289 is close to a multiple of 49 (49 * 6 = 294)
	float n_ = 0.142857142857;												// 1.0f / 7.0f
	vec3 ns = n_ * D.wyz - D.xzx;
	vec4 j = p - 49.0f * floor(p * ns.z * ns.z);							// mod(p, 49)
	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0f * x_ );										// mod(j, N)
	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0f - abs(x) - abs(y);
	vec4 b0 = vec4(x.xy, y.xy);
	vec4 b1 = vec4(x.zw, y.zw);
	vec4 s0 = floor(b0) * 2.0f + 1.0f;
	vec4 s1 = floor(b1) * 2.0f + 1.0f;
	vec4 sh = -step(h, vec4(0.0f));
	vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww ;
	vec3 p0 = vec3(a0.xy, h.x);
	vec3 p1 = vec3(a0.zw, h.y);
	vec3 p2 = vec3(a1.xy, h.z);
	vec3 p3 = vec3(a1.zw, h.w);

	// Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.51f - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0f);
	m = m * m;
	return 93.0f * dot(m * m, vec4( dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
};


void main()
{

//	vec3 tex_coord = spherical_point + 0.1f * vec3(snoise(spherical_point + vec3(0.0, 0.0, time)),
//    						           	      snoise(spherical_point + vec3(43.0, 17.0, time)),
//                                              snoise(spherical_point + vec3(-17.0, -43.0, time)));

	vec3 tex_coord = spherical_point + 0.04f * vec3(snoise(spherical_point * vec3( 0.43f,  0.17f, 0.37f) * time),
    						           	           snoise(spherical_point * vec3( 0.16f,  0.23f, 0.61f) * time),
                                                   snoise(spherical_point * vec3(-0.14f, -0.71f, 0.51f) * time));

	
	float radius = 1.0f + 0.007f * snoise(2.0f * tex_coord) + 0.003f * snoise(4.0f * tex_coord) + 0.001f * snoise(16.0f * tex_coord);
	position = 30.0f * radius * spherical_point;
	gl_Position = projection_matrix * view_matrix * vec4(position, 1.0f);
};