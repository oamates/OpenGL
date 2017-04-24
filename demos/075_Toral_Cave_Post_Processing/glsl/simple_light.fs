#version 330 core

uniform sampler2D ssao_image;

in vec3 position_ws;
in vec3 normal_ws;
in vec3 view;

out vec4 FragmentColor;

const int MAX_LIGHT_SOURCES = 8;
uniform vec3 light_position[MAX_LIGHT_SOURCES];
uniform vec4 light_color[MAX_LIGHT_SOURCES];								// the last component will measure light intensity

float distance[MAX_LIGHT_SOURCES]; 

const float Ns = 40.0f;

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
// Simplex 3d noise function
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
    vec3 x2 = x0 - i2 + C.yyy;                                              // 2.0f * C.x = 1.0f / 3.0f = C.y
    vec3 x3 = x0 - D.yyy;                                                   // -1.0f + 3.0f * C.x = -0.5f = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute( permute(permute(i.z + vec4(0.0f, i1.z, i2.z, 1.0f)) + i.y + vec4(0.0f, i1.y, i2.y, 1.0f)) + i.x + vec4(0.0f, i1.x, i2.x, 1.0f));

    // Gradients: 7x7 points over a square, mapped onto an octahedron. The ring size 17 * 17 = 289 is close to a multiple of 49 (49 * 6 = 294)
    float n_ = 0.142857142857;                                              // 1.0f / 7.0f
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0f * floor(p * ns.z * ns.z);                            // mod(p, 49)
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0f * x_ );                                        // mod(j, N)
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

vec3 marble(vec3 position)
{
    const vec3 marble_color0 = vec3(0.1114f, 0.1431f, 0.0126f);
    const vec3 marble_color1 = vec3(0.9147f, 0.6987f, 0.3947f);

    float n = 0.0f;
    float frequency = 1.87f;
    float weight = 0.77f;
    float frequency_factor = 2.07;
    float weight_factor = 0.47;

    for (int i = 0; i < 4; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.45);

    return mix(marble_color0, marble_color1, n);
}

vec3 marble_color(vec3 position)
{
    const vec3 marble_color0 = vec3(0.125f, 0.065f, 0.057f);
    const vec3 marble_color1 = vec3(0.344f, 0.231f, 0.111f);
    const vec3 marble_color2 = vec3(0.444f, 0.331f, 0.081f);
    const vec3 marble_color3 = vec3(0.344f, 0.411f, 0.071f);

    float n = 0.0f;
    float frequency = 1.17f;
    float frequency_factor = 2.34;
    float weight_factor = 0.67;
    float weight = 1.0f;

    for (int i = 0; i < 6; ++i)
    {
        n += weight * snoise(vec3(position * frequency));
        frequency *= frequency_factor;
        weight *= weight_factor;
    };

    n = (1.0f - weight_factor) * abs(n);
    n = pow(n, 0.85);

    vec3 color = (n >= 0.6667) ? mix(marble_color2, marble_color3, 3.0f * (n - 0.666667f)) : 
                 (n >= 0.3333) ? mix(marble_color1, marble_color2, 3.0f * (n - 0.333333f)) :
                                 mix(marble_color0, marble_color1, 3.0f * (n - 0.000000f)) ;

    return 1.55f * color;
}


void main()
{
    vec2 texture_size = textureSize(ssao_image, 0);
	vec3 diffuse_color = marble_color(0.000725 * position_ws);
	vec3 color = 0.0675f * diffuse_color;									// ambient component
    vec3 specular_color = mix(diffuse_color, vec3(1.0f), 0.5);

    float luminosity = dot(diffuse_color, vec3(0.299, 0.587, 0.114));

    float Lx = dFdx(luminosity); vec3 Tx = normalize(dFdx(position_ws));
    float Ly = dFdy(luminosity); vec3 Ty = normalize(dFdy(position_ws));

    vec3 surface_gradient = 3.25f * (Lx * cross(normal_ws, Tx) + Ly * cross(Ty, normal_ws));
    vec3 n = normalize(normal_ws + surface_gradient);


	for (int i = 0; i < MAX_LIGHT_SOURCES; ++i)
	{
    	vec3 light_direction = light_position[i] - position_ws;
    	distance[i] = length(light_direction);
	    vec3 l = light_direction / distance[i];
	    vec3 v = normalize(view);

	    float cos_theta = dot(n, l);

		float occlusion_factor = 0.1 + 0.9 * texture(ssao_image, gl_FragCoord.xy / texture_size).r;

	    if (cos_theta > 0.0f) 
    	{
            float decay_factor = 11.0 / (0.00875f + 0.00875f * distance[i]);
        	color += cos_theta * diffuse_color * light_color[i].rgb * occlusion_factor * decay_factor;

        	// Phong lighting
			// vec3 r = reflect(-l, n);
			// float cos_alpha = max(dot(v, r), 0.0f);
			// float exponent = 0.25f * specular_exponent();
        
        	// Blinn - Phong lighting
        	vec3 h = normalize(l + v);
        	float cos_alpha = max(dot(h, n), 0.0f);

        	color += pow(cos_alpha, Ns) * occlusion_factor * specular_color * light_color[i].rgb * decay_factor;
        }
    }

    color = pow(color, vec3(1.0 / 2.2));
    FragmentColor = vec4(color, 1.0f);     
}



