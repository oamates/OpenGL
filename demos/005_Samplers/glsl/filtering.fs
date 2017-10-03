#version 400 core

#extension GL_ARB_texture_query_lod : enable
#extension GL_EXT_gpu_shader4 : enable

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform sampler2D nearest_mode_tex;
uniform sampler2D linear_mode_tex;
uniform sampler2D mipmap_mode_tex;
uniform sampler2D anisotropic_mode_tex;

uniform vec2 texture_dim = vec2(1920.0, 1080.0);

out vec4 FragmentColor;

const float pi = 3.14159265359f;

//==============================================================================================================================================================
// FILTER FUNCTIONS
//==============================================================================================================================================================
//#define FILTER_SHARPNESS 2.0
uniform float sharpness;

float box_filter(float r2)
{
    return 1.0f;
}

float gauss_filter(float r2)
{
    float alpha = sharpness;
    return exp(-alpha * r2);
}

float tri_filter(float r2)
{
    float alpha = sharpness;
    float r = sqrt(r2);
    return max(0.0f, 1.0f - r / alpha);
}

float sinc(float x)
{
    float y = pi * x;
    return sin(y) / y;
}

float lanczos_filter(float r2)
{
    if (r2 == 0.0f) return 1;
    float r = sqrt(r2);
    return sinc(r) * sinc(r / 1.3);
}

float cr_filter(float r2)                   // catmull-rom filter
{
    float r = sqrt(r2);
    return (r >= 2.0f) ? 0.0f : 
           (r  < 1.0f) ? (3.0f * r * r2 - 5.0f * r2 + 2.0f) 
                       : (-r * r2 + 5.0f * r2 - 8.0f * r + 4.0f);
}

float quadratic_filter(float r2)
{
    float a = sharpness;
    return 1.0f - r2 / (a * a);
}

float cubic_filter(float r2)
{
    float a = sharpness;
    float r = sqrt(r2);
    return 1.0f - 3.0f * r2 / (a * a) + 2.0f * r * r2 / (a * a * a);
}

float filter_func(float q)
{
	return gauss_filter(q);
}

//==============================================================================================================================================================
// texture filtering subroutines
//==============================================================================================================================================================

subroutine vec3 texture_filter_func(vec2 uv);
subroutine uniform texture_filter_func texture_filter;

subroutine(texture_filter_func) vec3 nearest_filter_HW(vec2 uv)
{
    return texture(nearest_mode_tex, uv).rgb;
}

subroutine(texture_filter_func) vec3 linear_filter_HW(vec2 uv)
{
    return texture(linear_mode_tex, uv).rgb;
}

subroutine(texture_filter_func) vec3 mipmap_filter_HW(vec2 uv)
{
    return texture(mipmap_mode_tex, uv).rgb;
}

subroutine(texture_filter_func) vec3 anisotropic_filter_HW(vec2 uv)
{
    return texture(anisotropic_mode_tex, uv).rgb;
}






//========= PARAMETERS FOR THE RENDERING OF THE TUNNEL/PLANE ==========
#define SPEED 0.00
#define ZOOM 0.07
//=====================================================================

uniform int frame;
uniform float time;
uniform sampler2D tex0;



//==============================================================================================================================================================
// TEXTURE FILTERING (EWA) PARAMETERS 
//==============================================================================================================================================================
#define MAX_ECCENTRICITY 16
#define NUM_PROBES 6
#define FILTER_WIDTH 1.0
#define TEXELS_PER_PIXEL 1.0
#define USE_GL4 1
#define RENDER_SCENE 1
#define SPLIT_SCREEN 1
#define USE_HARDWARE_LOD 1
#define TEXEL_LIMIT 128

/**
* FILTERING MODES:
* 1: Hardware
* 2: EWA
* 3: EWA 2-tex
* 4: EWA 4-tex
* 5: Approximate EWA
* 6: Approximate Spatial EWA
* 7: Approximate Temporal EWA
* 8: Display hardware mip-map selection deviation
* 9: Display annisotropy levels
*/

//==============================================================================================================================================================
// EWA ( reference / 2-tex / 4-tex) 
// EWA filter :: Adapted from an ANSI C implementation from Matt Pharr
//==============================================================================================================================================================

vec4 ewaFilter(sampler2D tex0, vec2 p0, vec2 du, vec2 dv, float lod, int psize)
{
	int scale = psize >> int(lod);
	vec4 foo = texture(tex0, p0);
	
	// don't bother with elliptical filtering if the scale is very small
	if(scale < 2)
		return foo;

	p0 -= vec2(0.5f) / scale;
	vec2 p = scale * p0;

	float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

	// compute ellipse coefficients 
    // A*x*x + B*x*y + C*y*y = F.
    float A = vx * vx + vy * vy + 1.0f;
    float B = -2.0f * (ux * vx + uy * vy);
    float C = ux * ux + uy * uy + 1.0f;
    float F = A * C - 0.25f * B * B;

	// Compute the ellipse's (u,v) bounding box in texture space
	float bbox_du = 2.0f / (-B * B + 4.0 * C * A) * sqrt(C * (-B * B + 4.0 * C * A) * F);
	float bbox_dv = 2.0f / (-B * B + 4.0 * C * A) * sqrt(A * (-B * B + 4.0 * C * A) * F);

#if 1
	// Clamp the ellipse so that the bbox includes at most TEXEL_LIMIT texels.
	// This is necessary in order to bound the run-time, since the ellipse can be arbitrarily large
	// Note that here we are actually clamping the bbox directly instead of the ellipse.
	// Non real-time GPU renderers can skip this step.
	if(bbox_du * bbox_dv > TEXEL_LIMIT)
	{
		float ll = sqrt(bbox_du * bbox_dv / TEXEL_LIMIT);
		bbox_du /= ll;
		bbox_dv /= ll;
	}
#endif

	//the ellipse bbox			    
    int u0 = int(floor(p.s - bbox_du));
    int u1 = int(ceil (p.s + bbox_du));
    int v0 = int(floor(p.t - bbox_dv));
    int v1 = int(ceil (p.t + bbox_dv));

    // Heckbert MS thesis, p. 59; scan over the bounding box of the ellipse
    // and incrementally update the value of Axx + Bxy + Cyy; when this
    // value, q, is less than F, we're inside the ellipse so we filter away..
    vec4 num = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float den = 0;
    float ddq = 2.0f * A;
    float U = u0 - p.s;

#if (FILTERING_MODE != 4)
	
	for (int v = v0; v <= v1; ++v)
    {
		float V = v - p.t;
		float dq = A * (2 * U + 1) + B * V;
		float q = (C * V + B * U) * V + A * U * U;
#if (FILTERING_MODE == 2) // reference implementation
		for (int u = u0; u <= u1; ++u)
        {
			if (q < F) 
			{
				float r2 = q / F;
				float weight = filter_func(r2);
				num += weight * textureLod(tex0, vec2(u + 0.5f, v + 0.5f) / scale , int(lod));
				den += weight;
			}
			q += dq;
			dq += ddq;
		}
#else // FILTERING_MODE == 3 / 2-tex implementation

		for (int u = u0; u <= u1; u += 2)
		{
			float w1 = filter_func(q / F);
			w1 = (q < F) ? w1 : 0;
			q += dq;
			dq += ddq;
			float w2 = filter_func(q / F);
			w2 = (q < F) ? w2 : 0;
			float offest= w2 / (w1 + w2);
			float weight = (w1 + w2);
            if(weight > 0.0)
			{
				num += weight * textureLod(tex0, vec2(u + 0.5f + offest, v + 0.5f) / scale , int(lod));
				den += weight;
            }
			q += dq;
			dq += ddq;
		}
#endif

    }

#else
//FILTERING_MODE==4 4-tex implementation
	for (int v = v0; v <= v1; v += 2)
	{
		float V = v - p.t;
		float dq = A * (2.0f * U + 1) + B * V;
		float q = (C * V + B * U) * V + A * U * U;
		
		float V2 = v + 1.0f - p.t;
		float dq2 = A * (2.0f * U + 1) + B * V2;
		float q2 = (C * V2 + B * U) * V2 + A * U * U;

		for (int u = u0; u <= u1; u += 2)
		{
			float w1 = filter_func(q / F);
			w1 = (q < F) ? w1 : 0;
			q += dq;
			dq += ddq;
			float w2 = filter_func(q / F);
			w2 = (q < F) ? w2 : 0;
						
			float w3 = filter_func(q2 / F);
			//w3 = (q2 < F)? w3 : 0;
			q2 += dq2;
			dq2 += ddq;
			float w4 = filter_func(q2 / F);
			//w4 = (q2 < F)? w4 : 0;
			
			q += dq;
			dq += ddq;
			q2 += dq2;
			dq2 += ddq;
			
			float offest_v = (w3 + w4) / (w1 + w2 + w3 + w4);
			float offest_u;// = (w4+w2)/(w1+w3);
			offest_u= (w4) / (w4 + w3);
			float weight = (w1 + w2 + w3 + w4);

		//	float Error = (w1*w4-w2*w3);
			if(weight > 0.1f)
			{
			    num += weight * textureLod(tex0, vec2(u + offest_u + 0.5, v + offest_v + 0.5) / scale , int(lod));
			    den += weight;
			}
		}
    }

#endif

	vec4 color = num * (1.0f / den);
	return color;
}

//==============================================================================================================================================================
// mip-map level selection routine
//==============================================================================================================================================================
vec2 textureQueryLOD_EWA(sampler2D sampler, vec2 du, vec2 dv, int psize)
{
	int scale = psize;

	float ux = du.s * scale;
    float vx = du.t * scale;
    float uy = dv.s * scale;
    float vy = dv.t * scale;

	// compute ellipse coefficients
    // A*x*x + B*x*y + C*y*y = F.
    float A = vx * vx + vy * vy;
    float B = -2.0f * (ux * vx + uy * vy);
    float C = ux * ux + uy * uy;
    float F = A * C - 0.25f * B * B;
    float inv_F = 1.0f / F;
		
	A *= inv_F;
    B *= inv_F;
    C *= inv_F;
	
	float root = sqrt((A - C) * (A - C) + B * B);
	float majorRadius = sqrt(2.0f / (A + C - root));
	float minorRadius = sqrt(2.0f / (A + C + root));

	float majorLength = majorRadius;
    float minorLength = max(minorRadius, 0.01);

    const float maxEccentricity = MAX_ECCENTRICITY;

    float e = majorLength / minorLength;

    if (e > maxEccentricity)
		minorLength *= (e / maxEccentricity);
	
    float lod = log2(minorLength / TEXELS_PER_PIXEL);  
	lod = clamp(lod, 0.0, log2(psize));

	return vec2(lod, e);
}

vec4 texture_EWA(sampler2D sampler, vec2 uv)
{
	vec2 duv_dx = dFdx(uv);
	vec2 duv_dy = dFdy(uv);
	
	int psize = textureSize(sampler, 0).x;
	float lod;
#ifdef USE_HARDWARE_LOD
	lod = textureQueryLOD(sampler, uv).x;
#else
	lod = textureQueryLOD_EWA(sampler, duv_dx, duv_dy, psize).x;
#endif
	return ewaFilter(sampler, uv, duv_dx, duv_dy, lod, psize);
}

//==============================================================================================================================================================
// helper : visualize the absolute deviation between hardware and software lod selection
//==============================================================================================================================================================
vec4 lodError(sampler2D sampler, vec2 uv)
{
	vec2 duv_dx = dFdx(uv);
	vec2 duv_dy = dFdy(uv);
	
	int psize = textureSize(sampler, 0).x;

	float lod1 = textureQueryLOD(sampler, uv).x;
	float lod2 = textureQueryLOD_EWA(sampler, duv_dx, duv_dy, psize).x;

	return vec4(vec3(2.0f * abs(lod2 - lod1)), 1.0f);
}

vec4 map_A(float h)
{
    vec4 colors[3];
    colors[0] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    colors[1] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    colors[2] = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	h = clamp(h, 0 ,16);
	if(h > 8)
		return mix(colors[1], colors[2], (h - 8) / 8);
	else
		return mix(colors[0], colors[1], h / 8);
}

vec4 map_B(float h)
{
    vec4 colors[3];
    colors[0] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    colors[1] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    colors[2] = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	h = mod(h, 3);
	if(h > 1)
		return mix(colors[1], colors[2], h - 1);
	else
		return mix(colors[0], colors[1], h);
}

//==============================================================================================================================================================
// helper : visualize the anisotropy level
//==============================================================================================================================================================
vec4 anisoLevel(sampler2D sampler, vec2 uv)
{
	vec2 duv_dx = dFdx(uv);
	vec2 duv_dy = dFdy(uv);
	int psize = textureSize(sampler, 0).x;
	float aniso = textureQueryLOD_EWA(sampler, duv_dx, duv_dy, psize).y;
	return mix(map_A(aniso), texture(sampler, uv), 0.4);
}

//==============================================================================================================================================================
// helper : visualize the mip-map level
//==============================================================================================================================================================
vec4 mipLevel(sampler2D sampler, vec2 uv)
{
	vec2 duv_dx = dFdx(uv);
	vec2 duv_dy = dFdy(uv);
	int psize = textureSize(sampler, 0).x;
	float lod = textureQueryLOD_EWA(sampler, duv_dx, duv_dy, psize).x;
	return mix(map_B(lod), texture(sampler, uv), 0.45);
}

//==============================================================================================================================================================
// Approximated EWA (normal / spatial / temporal)
//==============================================================================================================================================================
vec4 texture2DApprox(sampler2D sampler, vec2 uv)
{
	vec2 du = dFdx(uv);
	vec2 dv = dFdy(uv);
	
	int psize = textureSize(sampler, 0).x;

#if (FILTERING_MODE == 6)
	float vlod = textureQueryLOD_EWA(sampler, du, dv, psize).y;

	vec4 hcolor = texture(sampler, uv);
	if(vlod < 12)
		return hcolor;
#endif

	int scale = psize;
	scale = 1;

	vec2 p = scale * uv;

	float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

	// compute ellipse coefficients to bound the region: 
    // A*x*x + B*x*y + C*y*y = F.
    float A = vx * vx + vy * vy;
    float B = -2.0f * (ux * vx + uy * vy);
    float C = ux * ux + uy * uy;
    float F = A * C - 0.25f * B * B;

	A = A / F;
    B = B / F;
    C = C / F;

	float root = sqrt((A - C) * (A - C) + B * B);
	float majorRadius = sqrt(2.0f / (A + C - root));
	float minorRadius = sqrt(2.0f / (A + C + root));

	int iProbes = NUM_PROBES;

	float lineLength = 2 * (majorRadius - 8 * minorRadius);
	if(lineLength < 0) lineLength = 0;
	//lineLength *=2.0;

	float theta = atan(B, A - C);
	if (A > C) theta = theta + 0.5f * pi;

	float dpu = cos(theta) * lineLength / (iProbes - 1);
	float dpv = sin(theta) * lineLength / (iProbes - 1);

	vec4 num = texture(sampler, uv);
	float den = 1;
	if (lineLength == 0)
		iProbes = 0;
	
#if (FILTERING_MODE != 7)
	for(int i = 1; i < iProbes / 2; i++)
	{
		float d =  (0.5f * float(i)) * length(vec2(dpu, dpv)) / lineLength;
		float weight = filter_func(d);

		num += weight * texture(sampler, uv + (i * vec2(dpu, dpv)) / scale);
		num += weight * texture(sampler, uv - (i * vec2(dpu, dpv)) / scale);

		den += weight;
		den += weight;
	}
#else
	//only 3 probes per frame are supported for the temporal filtering
	if((frame & 1) == 1)
	{
		num += texture(sampler, (p - 1 * vec2(dpu, dpv)) / scale);
		num += texture(sampler, (p + 2 * vec2(dpu, dpv)) / scale);
		den = 3;
	}
	else{
		num += texture(sampler, (p + 1 * vec2(dpu, dpv)) / scale);
		num += texture(sampler, (p - 2 * vec2(dpu, dpv)) / scale);
		den = 3;
	}
#endif

#if (FILTERING_MODE == 6)
	vec4 scolor = (1.0f / den) * num;
	return mix(hcolor, scolor, smoothstep(0.0f, 1.0f, (vlod - 8.0f) / 13.0f));
#else
	return (1.0 / den) * num;
#endif

}

/*
vec4 superTexture2D(sampler2D tex0, vec2 uv)
{
    vec4 color =  texture2D(tex0,uv);

    // hardware filtering
  #if (FILTERING_MODE == 1)
	vec4 color2 = texture(tex0, uv);
  #endif

    // EWA/EWA 2-tex/EWA 4-tex
  #if (FILTERING_MODE == 2 || FILTERING_MODE == 3 || FILTERING_MODE == 4)
    vec4 color2 = texture2DEWA(tex0, uv);
  #endif
    // Approximate EWA/Approximate Spatial EWA/Approximate Temporal EWA 
  #if (FILTERING_MODE == 5 || FILTERING_MODE == 6 || FILTERING_MODE == 7)
	vec4 color2 = texture2DApprox(tex0, uv);
  #endif

    // Display hardware mip-map selection deviation  
  #if (FILTERING_MODE == 8)
	vec4 color2 = lodError(tex0,uv);
    #undef SPLIT_SCREEN
  #endif

    // Display anisotropy levels
  #if (FILTERING_MODE == 9) 
	vec4 color2 = anisoLevel(tex0,uv);
    #undef SPLIT_SCREEN
  #endif

    // Display mip levels
  #if (FILTERING_MODE == 0) 
	vec4 color2 = mipLevel(tex0,uv);
    #undef SPLIT_SCREEN
  #endif

  #if (SPLIT_SCREEN == 1)
	if (abs (gl_FragCoord.x - RESX / 2) < 1)
		return vec4(0.0f, 0.0f, 0.0f, 1.0f);

	if (gl_FragCoord.x > RESX / 2)
		return color;
	else
  #endif
	return color2;

}
*/

void main()
{
    FragmentColor = vec4(texture_filter(uv), 1.0f);
    // gl_FragColor = superTexture2D(tex0,uv);
}
