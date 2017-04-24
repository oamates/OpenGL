#version 430 core

layout(binding = 0) uniform sampler2D ssao_image;

in vec4 position_ws;
in vec4 normal_ws;
in vec4 view_direction;

uniform mat4 view_matrix;


out vec4 fragment_color;

const float two_pi = 6.283185307179586476925286766559f;
const float one_over_root3 = 0.57735026918962576450914878050196f;
const float cube_size = 7.0;

//const vec4 light_position = vec4(cube_size, cube_size, cube_size, 1.0f);

uniform vec4 light_position;
const vec4 light_color = vec4(1.5f, 1.5f, 0.2f, 1.0f);
const float light_intensity = 15000.0f;



float mod289(float x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };
vec2 mod289(vec2 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };
vec3 mod289(vec3 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };
vec4 mod289(vec4 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; };


float permute(float x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); };
vec2 permute(vec2 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); };
vec3 permute(vec3 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); };
vec4 permute(vec4 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); };


float taylorInvSqrt(float r)
	{ return 1.79284291400159f - 0.85373472095314f * r; };
vec2 taylorInvSqrt(vec2 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; };
vec3 taylorInvSqrt(vec3 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; };
vec4 taylorInvSqrt(vec4 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; };


float snoise(vec2 v)
{
	const vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);		
	vec2 i  = floor(v + dot(v, C.yy));																			
	vec2 x0 = v - i + dot(i, C.xx);
	vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);													
  	vec4 x12 = x0.xyxy + C.xxzz;
  	x12.xy -= i1;
	i = mod289(i); 																								
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));
	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m * m;
	m = m * m;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;																		
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= taylorInvSqrt(a0 * a0 + h * h);																		
	vec3 g = vec3(a0.x  * x0.x  + h.x  * x0.y, a0.yz * x12.xz + h.yz * x12.yw);									
  	return 130.0 * dot(m, g);
};


float snoise(vec3 v)
{
	const vec2 C = vec2(1.0f / 6.0f, 1.0f / 3.0f);
	const vec4 D = vec4(0.0f, 0.5f, 1.0f, 2.0f);
	vec3 i = floor(v + dot(v, C.yyy));																			
	vec3 x0 = v - i + dot(i, C.xxx);
	vec3 g = step(x0.yzx, x0.xyz);																				
	vec3 l = 1.0 - g;
	vec3 i1 = min(g.xyz, l.zxy);
	vec3 i2 = max(g.xyz, l.zxy);
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy;																					
	vec3 x3 = x0 - D.yyy;																						
	i = mod289(i);																								
	vec4 p = permute(permute(permute(i.z + vec4(0.0f, i1.z, i2.z, 1.0f)) + i.y + vec4(0.0, i1.y, i2.y, 1.0f)) + i.x + vec4(0.0, i1.x, i2.x, 1.0f));
	float n_ = 0.142857142857; 																					
	vec3 ns = n_ * D.wyz - D.xzx;																				
	vec4 j = p - 49.0f * floor(p * ns.z * ns.z);  																
	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0f * x_);																				
	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0f - abs(x) - abs(y);
	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );
	vec4 s0 = floor(b0) * 2.0f + 1.0f;
	vec4 s1 = floor(b1) * 2.0f + 1.0f;
	vec4 sh = -step(h, vec4(0.0f));
	vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww ;
	vec3 p0 = vec3(a0.xy, h.x);
	vec3 p1 = vec3(a0.zw, h.y);
	vec3 p2 = vec3(a1.xy, h.z);
	vec3 p3 = vec3(a1.zw, h.w);
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));							
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;
	vec4 m = max(0.6f - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f);						
	m = m * m;
	return 42.0f * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}


vec4 grad4(float j, vec4 ip)
{
	const vec4 ones = vec4(1.0f, 1.0f, 1.0f, -1.0f);
	vec4 p,s;
	p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0f) * ip.z - 1.0f;
	p.w = 1.5f - dot(abs(p.xyz), ones.xyz);
	s = vec4(lessThan(p, vec4(0.0f)));
	p.xyz = p.xyz + (s.xyz * 2.0f - 1.0f) * s.www; 
	return p;
}


float snoise(vec4 v)
{
	const vec4 C = vec4(0.138196601125011f, 0.276393202250021f, 0.414589803375032f, -0.447213595499958f); 
	// vec4((5 - sqrt(5)) / 20 = G4, 2 * G4, 3 * G4, -1 + 4 * G4)
	// (sqrt(5) - 1)/4 = F4, used once below
	const float F4 = 0.309016994374947451f;

	// First corner
	vec4 i  = floor(v + dot(v, vec4(F4)));
	vec4 x0 = v - i + dot(i, C.xxxx);

	// Other corners

	// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
	
	vec4 i0;
	vec3 isX = step(x0.yzw, x0.xxx);
	vec3 isYZ = step(x0.zww, x0.yyz);
	// i0.x = dot(isX, vec3(1.0f));
	i0.x = isX.x + isX.y + isX.z;
	i0.yzw = 1.0 - isX;
	// i0.y += dot(isYZ.xy, vec2(1.0f));
	i0.y += isYZ.x + isYZ.y;
	i0.zw += 1.0f - isYZ.xy;
	i0.z += isYZ.z;
	i0.w += 1.0f - isYZ.z;

	// i0 now contains the unique values 0,1,2,3 in each channel
	vec4 i3 = clamp(i0, 0.0f, 1.0f);
	vec4 i2 = clamp(i0 - 1.0f, 0.0f, 1.0f);
	vec4 i1 = clamp(i0 - 2.0f, 0.0f, 1.0f);

	// x0 = x0 - 0.0 + 0.0 * C.xxxx
	// x1 = x0 - i1  + 1.0 * C.xxxx
	// x2 = x0 - i2  + 2.0 * C.xxxx
	// x3 = x0 - i3  + 3.0 * C.xxxx
	// x4 = x0 - 1.0 + 4.0 * C.xxxx
	vec4 x1 = x0 - i1 + C.xxxx;
	vec4 x2 = x0 - i2 + C.yyyy;
	vec4 x3 = x0 - i3 + C.zzzz;
	vec4 x4 = x0 + C.wwww;

	// Permutations
	i = mod289(i);
	float j0 = permute(permute(permute(permute(i.w) + i.z) + i.y) + i.x);
	vec4 j1 = permute(permute(permute(permute(i.w + vec4(i1.w, i2.w, i3.w, 1.0f))
                                                  + i.z + vec4(i1.z, i2.z, i3.z, 1.0f))
                                                  + i.y + vec4(i1.y, i2.y, i3.y, 1.0f))
                                                  + i.x + vec4(i1.x, i2.x, i3.x, 1.0f));

	// Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
	// 7 * 7 * 6 = 294, which is close to the ring size 17*17 = 289.
	vec4 ip = vec4(1.0f / 294.0f, 1.0f / 49.0f, 1.0f / 7.0f, 0.0f) ;

	vec4 p0 = grad4(j0,   ip);
	vec4 p1 = grad4(j1.x, ip);
	vec4 p2 = grad4(j1.y, ip);
	vec4 p3 = grad4(j1.z, ip);
	vec4 p4 = grad4(j1.w, ip);

	// Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;
	p4 *= taylorInvSqrt(dot(p4, p4));

	// Mix contributions from the five corners
	vec3 m0 = max(0.6f - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0f);
	vec2 m1 = max(0.6f - vec2(dot(x3, x3), dot(x4, x4)             ), 0.0f);
	m0 = m0 * m0;
	m1 = m1 * m1;
	return 49.0 * (dot(m0*m0, vec3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) + dot(m1 * m1, vec2(dot(p3, x3), dot(p4, x4))));
};


vec3 marble_color(vec3 position)
{
	const vec3 Color1 = vec3(0.2f, 0.1f, 0.4f);
	const vec3 Color2 = vec3(0.7f, 0.6f, 0.8f);
	const float base_freq = 0.2f;

	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);
	noisevec = noisevec / 8.0f;

	float intensity = abs(noisevec[0] - 0.20f) + abs(noisevec[1] - 0.100f) + 
					  abs(noisevec[2] - 0.05f) + abs(noisevec[3] - 0.025f);

	intensity = clamp(intensity * 1.4f, 0.0f, 1.0f);
	
	return mix(Color1, Color2, intensity);
};


vec3 marble_color2(vec3 position)
{
	const vec3 Color1 = vec3(0.8f, 0.8f, 0.8f);
	const vec3 Color2 = vec3(0.2f, 0.2f, 0.5f);      
	const float base_freq = 0.2f;

	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);
	noisevec = noisevec / 8.0f;

	float intensity = abs(noisevec[0] - 0.200f) + abs(noisevec[1] - 0.100f) + 
					  abs(noisevec[2] - 0.050f) + abs(noisevec[3] - 0.025f);
	float sineval = sin(position.y * 12.0f + intensity * 8.0f) * 0.5f + 0.5f;
	
	return mix(Color1, Color2, sineval);
};


vec3 granite(vec3 position)
{
	const vec3 Color1 = vec3(0.35f, 0.3f, 0.2f);
	const vec3 Color2 = vec3(0.7f, 0.7f, 0.7f);

	const float base_freq = 0.2f;

	
	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);

	noisevec = noisevec / 8.0f;

	float intensity = min(1.0f, noisevec[3] * 12.0f);	
	return mix(Color1, Color2, intensity);
};

const vec2 screen_dim = vec2(1920.0f, 1080.0f);

void main()
{
	vec4 v = normalize(view_direction);


    float distance = length(light_position - position_ws);

    vec4 l = (light_position - position_ws) / distance; 
    float lambert_cosine = clamp(dot(normal_ws, l), 0.0f, 1.0f);

	vec4 r = reflect(l, normal_ws);
    float cos_alpha = clamp(dot(r, v), 0.0f, 1.0f) * lambert_cosine;
	

	float ambient_factor = 1.0f;//0.5f + 0.5f * texture(ssao_image, gl_FragCoord.xy / screen_dim).r;
//	float ambient_factor = texture(ssao_image, gl_FragCoord.xy / screen_dim).r;

	vec4 ambient_color = ambient_factor * vec4(marble_color(position_ws.xyz), 1.0f) / 7.0f;         

    vec4 material_diffuse_color = mix(ambient_color, vec4(marble_color(normal_ws.xyz), 1.0f) / 7.0f, 0.7f);

    vec4 material_specular_color = normalize(ambient_color) * light_color;

    float diffuse_distance_factor = 10.0f / (distance * distance);
    float specular_distance_factor = 15.0f / (distance * distance);

	fragment_color = ambient_color +
                     + material_diffuse_color * light_intensity * light_color * lambert_cosine * diffuse_distance_factor
                     + material_specular_color * light_intensity * light_color * pow(cos_alpha, 20) * specular_distance_factor;

//	fragment_color = vec4(vec3(ambient_factor), 1.0f);

};



