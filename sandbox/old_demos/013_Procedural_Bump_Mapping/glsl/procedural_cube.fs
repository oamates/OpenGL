#version 400

uniform float time;

in vec3 position;
out vec4 FragmentColor;



//==============================================================================================================================================================
// Helper functions
//==============================================================================================================================================================
vec2 mod289(vec2 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec3 mod289(vec3 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec4 mod289(vec4 x)
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }

float permute(float x) 
	{ return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec3 permute(vec3 x) 
	{ return mod((34.0f * x + 1.0f) * x, 289.0f); }
vec4 permute(vec4 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); }
vec4 taylorInvSqrt(vec4 r) 
	{ return 1.792843f - 0.853735f * r; }


float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return x*x*x*(x*(x*6 - 15) + 10);
}

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
}

//==============================================================================================================================================================
//  Plain cellular noise
//==============================================================================================================================================================
vec2 cellular(vec2 P) 
{
	const float K = 0.142857142857f;										// 1.0f / 7.0f
	const float Ko = 0.428571428571f;										// 3.0f / 7.0f
	const float jitter = 1.0f;												// Less gives more regular pattern

	// Permutation polynomial: (34x^2 + x) mod 289. Standard 3x3 search window for good F1 and F2 values
	vec2 Pi = mod(floor(P), 289.0f);
 	vec2 Pf = fract(P);
	vec3 oi = vec3(-1.0f, 0.0f, 1.0f);
	vec3 of = vec3(-0.5f, 0.5f, 1.5f);
	vec3 px = permute(Pi.x + oi);
	vec3 p = permute(px.x + Pi.y + oi);										// p11, p12, p13
	vec3 ox = fract(p*K) - Ko;
	vec3 oy = mod(floor(p*K), 7.0f) * K - Ko;
	vec3 dx = Pf.x + 0.5 + jitter * ox;
	vec3 dy = Pf.y - of + jitter * oy;
	vec3 d1 = dx * dx + dy * dy;											// d11, d12 and d13, squared
	p = permute(px.y + Pi.y + oi);											// p21, p22, p23
	ox = fract(p * K) - Ko;
	oy = mod(floor(p * K), 7.0f) * K - Ko;
	dx = Pf.x - 0.5 + jitter * ox;
	dy = Pf.y - of + jitter * oy;
	vec3 d2 = dx * dx + dy * dy;											// d21, d22 and d23, squared
	p = permute(px.z + Pi.y + oi);											// p31, p32, p33
	ox = fract(p * K) - Ko;
	oy = mod(floor(p * K), 7.0f) * K - Ko;
	dx = Pf.x - 1.5 + jitter * ox;
	dy = Pf.y - of + jitter * oy;
	vec3 d3 = dx * dx + dy * dy;											// d31, d32 and d33, squared

	// Sort out the two smallest distances (F1, F2)
	vec3 d1a = min(d1, d2);
	d2 = max(d1, d2);														// Swap to keep candidates for F2
	d2 = min(d2, d3);														// neither F1 nor F2 are now in d3
	d1 = min(d1a, d2);														// F1 is now in d1
	d2 = max(d1a, d2);														// Swap to keep candidates for F2
	d1.xy = (d1.x < d1.y) ? d1.xy : d1.yx;									// Swap if smaller
	d1.xz = (d1.x < d1.z) ? d1.xz : d1.zx;									// F1 is in d1.x
	d1.yz = min(d1.yz, d2.yz);												// F2 is now not in d2.yz
	d1.y = min(d1.y, d1.z);													// nor in d1.z
	d1.y = min(d1.y, d2.x);													// F2 is in d1.y, we're done.
	return sqrt(d1.xy);
}

//==============================================================================================================================================================
//  Plain cellular noise returning the 2D vectors to each of the two closest points in a vec4
//==============================================================================================================================================================
void cellular(in vec2 P, out vec2 F, out vec4 d1d2) 
{
	const float K = 0.142857142857f;										// 1.0f / 7.0f
	const float Ko = 0.428571428571f;										// 3.0f / 7.0f
	const float jitter = 1.0f;												// Less gives more regular pattern

	// Standard 3x3 search window for good F1 and F2 values.
	vec2 Pi = mod(floor(P), 289.0f);
 	vec2 Pf = fract(P);
	vec3 oi = vec3(-1.0f, 0.0f, 1.0f);
	vec3 of = vec3(-0.5f, 0.5f, 1.5f);
	vec3 px = permute(Pi.x + oi);
	vec3 p = permute(px.x + Pi.y + oi);										// p11, p12, p13
	vec3 ox = fract(p * K) - Ko;
	vec3 oy = mod(floor(p * K), 7.0f) * K - Ko;
	vec3 d1x = Pf.x + 0.5f + jitter * ox;                                      
	vec3 d1y = Pf.y - of + jitter * oy;
	vec3 d1 = d1x * d1x + d1y * d1y;										// d11, d12 and d13, squared
	p = permute(px.y + Pi.y + oi);											// p21, p22, p23
	ox = fract(p * K) - Ko;
	oy = mod(floor(p * K), 7.0f) * K - Ko;
	vec3 d2x = Pf.x - 0.5 + jitter * ox;
	vec3 d2y = Pf.y - of + jitter * oy;
	vec3 d2 = d2x * d2x + d2y * d2y;										// d21, d22 and d23, squared
	p = permute(px.z + Pi.y + oi);											// p31, p32, p33
	ox = fract(p * K) - Ko;
	oy = mod(floor(p * K), 7.0) * K - Ko;
	vec3 d3x = Pf.x - 1.5 + jitter * ox;
	vec3 d3y = Pf.y - of + jitter * oy;
	vec3 d3 = d3x * d3x + d3y * d3y;										// d31, d32 and d33, squared

	// Sort out the two smallest distances (F1, F2). While also swapping dx and dy accordingly
	vec3 comp3 = step(d2, d1);
	vec3 d1a = mix(d1, d2, comp3);
	vec3 d1xa = mix(d1x, d2x, comp3);
	vec3 d1ya = mix(d1y, d2y, comp3);
	d2 = mix(d2, d1, comp3);												// Swap to keep candidates for F2
	d2x = mix(d2x, d1x, comp3);
	d2y = mix(d2y, d1y, comp3);
	comp3 = step(d3, d2);
	d2 = mix(d2, d3, comp3);												// neither F1 nor F2 are now in d3
	d2x = mix(d2x, d3x, comp3);
	d2y = mix(d2y, d3y, comp3);
	comp3 = step(d2, d1a);
	d1 = mix(d1a, d2, comp3);												// F1 is now in d1
	d1x = mix(d1xa, d2x, comp3);
	d1y = mix(d1ya, d2y, comp3);
	d2 = mix(d2, d1a, comp3);												// Swap to keep candidates for F2
	d2x = mix(d2x, d1xa, comp3);
	d2y = mix(d2y, d1ya, comp3);
	float comp1 = step(d1.y, d1.x);
	d1.xy = mix(d1.xy, d1.yx, comp1);										// Swap if smaller
	d1x.xy = mix(d1x.xy, d1x.yx, comp1);
	d1y.xy = mix(d1y.xy, d1y.yx, comp1);
	comp1 = step(d1.z, d1.x);
	d1.xz = mix(d1.xz, d1.zx, comp1);										// F1 is in d1.x
	d1x.xz = mix(d1x.xz, d1x.zx, comp1);
	d1y.xz = mix(d1y.xz, d1y.zx, comp1);
	vec2 comp2 = step(d2.yz, d1.yz);
	d1.yz = mix(d1.yz, d2.yz, comp2);										// F2 is now not in d2.yz
	d1x.yz = mix(d1x.yz, d2x.yz, comp2);
	d1y.yz = mix(d1y.yz, d2y.yz, comp2);
	comp1 = step(d1.z, d1.y);
	d1.y = mix(d1.y, d1.z, comp1);											// nor in  d1.z
	d1x.y = mix(d1x.y, d1x.z, comp1);
	d1y.y = mix(d1y.y, d1y.z, comp1);
	comp1 = step(d2.x, d1.y);
	d1.y = mix(d1.y, d2.x, comp1);											// F2 is in d1.y, we're done.
	d1x.y = mix(d1x.y, d2x.x, comp1);
	d1y.y = mix(d1y.y, d2y.x, comp1);
	F = sqrt(d1.xy);
	d1d2 = vec4(d1x.x, d1y.x, d1x.y, d1y.y);
}

//==============================================================================================================================================================
// Simplex noise
//==============================================================================================================================================================

float simplex_noise (vec2 v)
{
	const vec4 C = vec4 (0.211324865405187f,								// (3.0f - sqrt(3.0f)) / 6.0f
						 0.366025403784439f,								//  0.5f * (sqrt(3.0f) - 1.0f)
						-0.577350269189626f,								// -1.0f + 2.0f * C.x
						 0.024390243902439f);								//  1.0f / 41.0f
	// First corner
	vec2 i = floor (v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);
	// Other corners
	vec2 i1 = (x0.x > x0.y) ? vec2 (1.0f, 0.0f) : vec2 (0.0f, 1.0f);
	vec4 x12 = x0 . xyxy + C. xxzz ;
	x12.xy -= i1 ;
	// Permutations
	i = mod289 (i);															// Avoid truncation effects in permutation
	vec3 p = permute(permute(i.y + vec3(0.0f, i1.y, 1.0f)) + i.x + vec3(0.0f, i1 .x , 1.0f));
	vec3 m = max (0.5f - vec3(dot(x0, x0) , dot(x12.xy, x12.xy), dot(x12.zw , x12.zw)), 0.0f);
	m = m*m;
	m = m *m;
	// Gradients
	vec3 x = 2.0 * fract (p * C. www ) - 1.0;
	vec3 h = abs (x) - 0.5;
	vec3 a0 = x - floor (x + 0.5) ;
	// Normalise gradients implicitly by scaling m
	m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
	// Compute final noise value at P
	vec3 g = vec3(a0.x * x0.x + h.x * x0.y, a0.yz * x12.xz + h.yz * x12.yw);
	return 130.0f * dot(m, g);
}

//==============================================================================================================================================================
// Simplified cellular noise
//==============================================================================================================================================================

vec2 cellular2x2 (vec2 P)
{
	const float K = 1.0f / 7.0f;
	const float K2 = 0.5f / 7.0f;
	const float jitter = 0.8f;										// jitter 1.0 makes F1 wrong more often
	vec2 Pi = mod (floor(P), 289.0f);
	vec2 Pf = fract (P);
	vec4 Pfx = Pf.x + vec4(-0.5f, -1.5f, -0.5f, -1.5f);
	vec4 Pfy = Pf.y + vec4(-0.5f, -0.5f, -1.5f, -1.5f);
	vec4 p = permute(Pi.x + vec4(0.0f, 1.0f, 0.0f, 1.0f));
	p = permute (p + Pi .y + vec4 (0.0f, 0.0f, 1.0f, 1.0f));
	vec4 ox = mod (p, 7.0f) * K + K2 ;
	vec4 oy = mod (floor(p * K), 7.0f) * K + K2;
	vec4 dx = Pfx + jitter * ox;
	vec4 dy = Pfy + jitter * oy;
	vec4 d = dx * dx + dy * dy;										// distances squared. cheat and pick only F1 for the return value
	d.xy = min (d.xy, d.zw);
	d.x = min (d.x, d.y);
	return d.xx;													// F1 duplicated , F2 not computed
}


float fractalsum(vec3 pos, float initial_frequency, float frequency_multiplier, int iterations)
{
	float value = 0.0f;
	float f = initial_frequency;
	for (int i = 0; i < iterations; ++i)
	{
		value += snoise(vec3(pos * f)) / f;
		f *= frequency_multiplier;
	};
	return value;
}

float turbulence(vec3 pos, float initial_frequency, float frequency_multiplier, int iterations)
{
	float value = 0.0f;
	float f = initial_frequency;
	for (int i = 0; i < iterations; ++i)
	{
		value += abs(snoise(vec3(pos * f))) / f;
		f *= frequency_multiplier;
	};
	return value;
}

//==============================================================================================================================================================
// True cellular noise
//==============================================================================================================================================================

vec2 cellular(vec3 P) 
{
	const float K = 0.142857142857f; // 1/7
	const float Ko = 0.428571428571f; // 1/2-K/2
	const float K2 = 0.020408163265306f; // 1/(7*7)
	const float Kz = 0.166666666667f; // 1/6
	const float Kzo = 0.416666666667f; // 1/2-1/6*2
	const float jitter = 1.0f; // smaller jitter gives more regular pattern

	vec3 Pi = mod(floor(P), 289.0);
 	vec3 Pf = fract(P) - 0.5;

	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);

	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);

	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);

	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);

	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod(floor(p11*K), 7.0)*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod(floor(p12*K), 7.0)*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;

	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod(floor(p13*K), 7.0)*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;

	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod(floor(p21*K), 7.0)*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;

	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod(floor(p22*K), 7.0)*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;

	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod(floor(p23*K), 7.0)*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;

	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod(floor(p31*K), 7.0)*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;

	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod(floor(p32*K), 7.0)*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;

	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod(floor(p33*K), 7.0)*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;

	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;

	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;

	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;

	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;

	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;

	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;

	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;

	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;

	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;

	vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
	vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
	vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
	vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
	vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
	vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
	vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
	vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
	vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

	// Sort out the two smallest distances (F1, F2)
	vec3 d1a = min(d11, d12);
	d12 = max(d11, d12);
	d11 = min(d1a, d13); // Smallest now not in d12 or d13
	d13 = max(d1a, d13);
	d12 = min(d12, d13); // 2nd smallest now not in d13
	vec3 d2a = min(d21, d22);
	d22 = max(d21, d22);
	d21 = min(d2a, d23); // Smallest now not in d22 or d23
	d23 = max(d2a, d23);
	d22 = min(d22, d23); // 2nd smallest now not in d23
	vec3 d3a = min(d31, d32);
	d32 = max(d31, d32);
	d31 = min(d3a, d33); // Smallest now not in d32 or d33
	d33 = max(d3a, d33);
	d32 = min(d32, d33); // 2nd smallest now not in d33
	vec3 da = min(d11, d21);
	d21 = max(d11, d21);
	d11 = min(da, d31); // Smallest now in d11
	d31 = max(da, d31); // 2nd smallest now not in d31
	d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
	d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
	d12 = min(d12, d21); // 2nd smallest now not in d21
	d12 = min(d12, d22); // nor in d22
	d12 = min(d12, d31); // nor in d31
	d12 = min(d12, d32); // nor in d32
	d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
	d11.y = min(d11.y,d12.z); // Only two more to go
	d11.y = min(d11.y,d11.z); // Done! (Phew!)
	return sqrt(d11.xy); // F1, F2
}

//==============================================================================================================================================================
// SRD NOISE
//==============================================================================================================================================================

vec2 grad2(vec2 p, float rot)												// Gradient mapping with an extra rotation.
{                                                                           // Map from a line to a diamond such that a shift maps to a rotation.
	const float K = 0.0243902439f;											// 1/41
	float u = permute(permute(p.x) + p.y) * K + rot;						// Rotate by shift
	u = 4.0f * fract(u) - 2.0f;
	return vec2(abs(u) - 1.0f, abs(abs(u + 1.0f) - 2.0f) - 1.0f);
}

float srdnoise(in vec2 P, in float rot, out vec2 grad)
{
	const float F2 = 0.366025403f;											// Helper constants
	const float G2 = 0.211324865f;
	const float K = 0.0243902439f;											// 1/41
	vec2 Ps = P + dot(P, vec2(F2));											// Transform input point to the skewed simplex grid
	vec2 Pi = floor(Ps);													// Round down to simplex origin
	vec2 P0 = Pi - dot(Pi, vec2(G2));										// Transform simplex origin back to (x,y) system
	vec2 v0 = P - P0;														// Find (x,y) offsets from simplex origin to first corner
	vec2 i1 = (v0.x > v0.y) ? vec2(1.0f, 0.0f) : vec2 (0.0f, 1.0f);			// Pick (+x, +y) or (+y, +x) increment sequence
	vec2 v1 = v0 - i1 + G2;													// Determine the offsets for the other two corners
	vec2 v2 = v0 - 1.0f + 2.0f * G2;
	Pi = mod(Pi, 289.0f);													// Wrap coordinates at 289 to avoid float precision problems
	vec3 t = max(0.5f - vec3(dot(v0,v0), dot(v1,v1), dot(v2,v2)), 0.0f);	// Calculate the circularly symmetric part of each noise wiggle
	vec3 t2 = t*t;
	vec3 t4 = t2*t2;
	vec2 g0 = grad2(Pi, rot);												// Calculate the gradients for the three corners
	vec2 g1 = grad2(Pi + i1, rot);
	vec2 g2 = grad2(Pi + 1.0f, rot);
	vec3 gv = vec3(dot(g0, v0), dot(g1, v1), dot(g2, v2));					// Compute noise contributions from each corner
	vec3 n = t4 * gv;														// Circular kernel times linear ramp
	vec3 temp = t2 * t * gv;												// Compute partial derivatives in x and y
	vec3 gradx = temp * vec3(v0.x, v1.x, v2.x);
	vec3 grady = temp * vec3(v0.y, v1.y, v2.y);
	grad.x = -8.0f * (gradx.x + gradx.y + gradx.z);
	grad.y = -8.0f * (grady.x + grady.y + grady.z);
	grad.x += dot(t4, vec3(g0.x, g1.x, g2.x));
	grad.y += dot(t4, vec3(g0.y, g1.y, g2.y));
	grad *= 40.0f;
	return 40.0f * (n.x + n.y + n.z);										// Add contributions from the three corners and return
}


//==============================================================================================================================================================
// glsl subroutines
//==============================================================================================================================================================

subroutine vec3 procedural_color(vec3 position);
subroutine uniform procedural_color color_func;

subroutine(procedural_color) vec3 gradient_noise_color(vec3 position)
{
	const vec3 base_color = vec3(0.95f, 0.75f, 0.15f);
	float n = 0.5f + 0.5f * snoise(vec3(10.0f * position));
	return n * base_color;	
}

subroutine(procedural_color) vec3 turbulence_color(vec3 position)
{
	const vec3 base_color = vec3(0.7f, 0.9f, 0.15f);
	float n = turbulence(position, 5.0f, 2.0f, 5);
	return n * base_color;
}

subroutine(procedural_color) vec3 fractal_sum_color(vec3 position)
{
	const vec3 base_color = vec3(0.7f, 0.9f, 0.15f);
	float n = 0.5f + 0.5f * fractalsum(position, 5.0f, 2.0f, 5);
	return n * base_color;
}

subroutine(procedural_color) vec3 cellular_noise_color(vec3 position)
{
	const vec3 base_color = vec3(0.6f, 1.1f, 0.5f);
	vec2 F = cellular(10.0f * position);
	float n = 0.1f + F.y - F.x;
	return n * base_color;
}

subroutine(procedural_color) vec3 flow_noise_color(vec3 position)
{
	vec2 g1, g2;
	vec2 p = 10.0f * position.xy;
	float n1 = srdnoise(p * 0.5f, 0.2f * time, g1);
	float n2 = srdnoise(p * 2.0f + g1 * 0.5f, 0.51f * time, g2);
	float n3 = srdnoise(p * 4.0f + g1 * 0.5f + g2 * 0.25f, 0.77f * time, g2);
	return vec3(0.4f, 0.5f, 0.6f) + vec3(n1 + 0.75f * n2 + 0.5f * n3);
}

subroutine(procedural_color) vec3 moving_cellular_color(vec3 position)
{
	const vec3 base_color = vec3(1.1f, 0.2f, 0.13f);
	vec3 tex_coord = position * 10.0f
      + 0.5f * vec3(snoise(position + vec3( 0.0f,   0.0f, time)),
					snoise(position + vec3(43.0f,  17.0f, time)), 
					snoise(position + vec3( 0.0f, -43.0f, time - 17.0f)));
	vec2 F = cellular(tex_coord);
	float n = 0.1f + F.y - F.x;
	return n * base_color;
}

subroutine(procedural_color) vec3 moving_spots_color(vec3 position)
{
	vec3 tex_coord = position * 4.0 - vec3(0.0, 0.0, time);
	vec2 F = cellular(tex_coord);
	float s = fwidth(F.x);
	float n1 = smoothstep(0.4f - s, 0.4f + s, F.x);
	float n2 = smoothstep(0.5f - s, 0.5f + s, F.x);
	return vec3(n1, n2, n2);
}

subroutine(procedural_color) vec3 star_plasma_color(vec3 position)
{
	const vec3 base_color0 = vec3(0.95f, 0.1f, 0.05f);
	const vec3 base_color1 = vec3(0.95f, 0.85f, 0.10f);
	vec3 tex_coord = position * 10.0f
      + 5.0f * vec3(snoise(position + vec3( 0.0f,   0.0f, time)),
					snoise(position + vec3(43.0f,  17.0f, time)), 
					snoise(position + vec3( 0.0f, -43.0f, time - 17.0f)));
	float n = 0.5f + 0.5f * snoise(vec3(tex_coord));
	return mix(base_color0, base_color1, n);	
}

subroutine(procedural_color) vec3 unknown_color(vec3 position)
{
	vec3 tex_coord = position + 0.1f * vec3(snoise(position + vec3(0.0, 0.0, time)),
    						           	    snoise(position + vec3(43.0, 17.0, time)),
                                            snoise(position + vec3(-17.0, -43.0, time)));
	float n      = snoise(tex_coord - vec3(0.0, 0.0, time)) + 
		0.50000f * snoise(tex_coord *  2.0f - vec3(0.0f, 0.0f, time * 1.4f)) +  
		0.25000f * snoise(tex_coord *  4.0f - vec3(0.0f, 0.0f, time * 2.0f)) +
		0.12500f * snoise(tex_coord *  8.0f - vec3(0.0f, 0.0f, time * 2.8f)) +
		0.06250f * snoise(tex_coord * 16.0f - vec3(0.0f, 0.0f, time * 4.0f)) +
		0.03125f * snoise(tex_coord * 32.0f - vec3(0.0f, 0.0f, time * 5.6f)); 

	n = n * 0.7;
	return vec3(1.0, 0.5, 0.0) + vec3(n, n, n);
}


subroutine(procedural_color) vec3 marble_color(vec3 position)
{
	const vec3 marble_color0 = vec3(0.444f, 0.231f, 0.041f);
	const vec3 marble_color1 = vec3(0.095f, 0.085f, 0.010f);
	const vec3 marble_color2 = vec3(0.444f, 0.331f, 0.011f);

//	position.x *= 6.0f;
	float rnd0 = snoise(position * 4.0f);
	float rnd1 = snoise(position * 6.0f);
	float rnd2 = snoise(position * 8.0f);
	

	float is_marble0 = smootherstep(0.5f, 1.0f, rnd0);
	float is_marble1 = smootherstep(0.5f, 1.0f, rnd1);
	float is_marble2 = smootherstep(0.5f, 1.0f, rnd2);

	vec3 color =
				 is_marble0 * marble_color0 +
				 + is_marble1 * marble_color1 +
				 + is_marble2 * marble_color2;

	return color;
}

subroutine(procedural_color) vec3 chessboard_color(vec3 position)
{
	vec3 I = floor(position * 7.999f);
	ivec3 II = ivec3(I);
	if (((II.x + II.y + II.z) & 1) == 1) return vec3(0.463f, 0.216f, 0.012f);
	return vec3(0.984f, 0.882f, 0.610f);
}

subroutine(procedural_color) vec3 experimental_color(vec3 position)
{
	vec3 I = floor(position * 7.999f);
	ivec3 II = ivec3(I);
	if (((II.x + II.y + II.z) & 1) == 1) return vec3(0.463f, 0.216f, 0.012f);
	return vec3(0.984f, 0.882f, 0.610f);
}

void main()
{
	FragmentColor = vec4(color_func(position), 1.0f);
}

