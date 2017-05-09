#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (rgba8, binding = 0) uniform image2D scene_image;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec2 focal_scale;
uniform sampler2D tb_tex;
uniform vec3 light_ws;
uniform int hell;


const float pi = 3.14159265359;
const float HORIZON = 200.0;

vec3 tex3D(vec3 p, vec3 n)
{
    p *= 0.75;
    n = max(abs(n) - 0.35f, 0.0f);
    n /= dot(n, vec3(1.0));
    vec3 tx = texture(tb_tex, p.zy).xyz;
    vec3 ty = texture(tb_tex, p.xz).xyz;
    vec3 tz = texture(tb_tex, p.xy).xyz;
    return tx * n.x + ty * n.y + tz * n.z;
}

vec3 tri(in vec3 x)
    { return abs(fract(x) - 0.5); }

//==============================================================================================================================================================
// Canyon distance function
//==============================================================================================================================================================
float map(vec3 p)
{    
    vec3 w = p;                                                         // Saving the position prior to mutation.
    vec3 op = tri(p * 1.1 + tri(p.zxy * 1.1));                          // Triangle perturbation.
    float ground = p.y + 3.5 + dot(op, vec3(0.222)) * 0.3;              // Ground plane, slightly perturbed.
    p += (op - 0.25) * 0.3;                                             // Adding some triangular perturbation.
    p = cos(p * 0.315 * 1.41 + sin(p.zxy * 0.875 * 1.27));              // Applying the sinusoidal field (the rocky bit).
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

vec3 doBumpMap(in vec3 p, in vec3 n, float bf)
{
    const vec2 e = vec2(0.0025, 0);
    mat3 m = mat3(tex3D(p - e.xyy, n), tex3D(p - e.yxy, n), tex3D(p - e.yyx, n));
    vec3 g = vec3(0.299, 0.587, 0.114) * m;                             // Converting to greyscale.
    g = (g - dot(tex3D(p , n), vec3(0.299, 0.587, 0.114))) / e.x;
    return normalize(n + g * bf);                                       // Bumped normal. "bf" - bump factor.
}

float accum;

//==============================================================================================================================================================
// Basic raymarcher.
//==============================================================================================================================================================
float trace(in vec3 ro, in vec3 rd)
{    
    accum = 0.0;
    float t = 0.0, h;
    for(int i = 0; i < 160; i++)
    {    
        h = map(ro + rd * t);
        if(abs(h) < 0.001 * (t * 0.25 + 1.0) || t > HORIZON) break;
        t += h;
        if(abs(h) < 0.25) accum += (0.25 - abs(h)) / 24.0;
    }
    return min(t, HORIZON);
}

//==============================================================================================================================================================
// Ambient occlusion, second variation
//==============================================================================================================================================================
float calculateAO2(in vec3 p, in vec3 n)
{
    float ao = 0.0, l;
    const float maxDist = 2.;
    const float nbIte = 6.0;
    for(float i = 1.0; i < nbIte + 0.5; i++)
    {
        l = (i * 0.75 + fract(cos(i) * 45758.5453) * 0.25) / nbIte * maxDist;        
        ao += (l - map(p + n * l)) / (1.0 + l);
    }
    return clamp(1.0 - ao / nbIte, 0.0, 1.0);
}

//==============================================================================================================================================================
// Ambient occlusion routine
//==============================================================================================================================================================
float calculateAO(in vec3 p, in vec3 n)
{    
    float sca = 1., occ = 0.;
    for(float i = 0.0; i < 5.0; i++)
    {
        float hr = 0.01 + i * 0.5 / 4.0;        
        float dd = map(n * hr + p);
        occ += (hr - dd) * sca;
        sca *= 0.7;
    }
    return clamp(1.0 - occ, 0.0, 1.0);    
}

//==============================================================================================================================================================
// Tetrahedral normal
//==============================================================================================================================================================
vec3 calcNormal(in vec3 p)
{
    vec2 e = vec2(0.001, -0.001); 
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) + e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

//==============================================================================================================================================================
// Shadows.
//==============================================================================================================================================================
float shadows(in vec3 ro, in vec3 rd, in float start, in float end, in float k)
{
    float shade = 1.0;
    const int shadIter = 24; 
    float dist = start;
    //float stepDist = end/float(shadIter);

    for (int i = 0; i < shadIter; i++)
    {
        float h = map(ro + rd*dist);
        shade = min(shade, k * h / dist);
        //shade = min(shade, smoothstep(0.0, 1.0, k*h/dist)); // Subtle difference. Thanks to IQ for this tidbit.

        dist += clamp(h, 0.02, 0.2);
        
        // There's some accuracy loss involved, but early exits from accumulative distance function can help.
        if ((h) < 0.001 || dist > end) break; 
    }    
    return min(max(shade, 0.0), 1.0); 
}

// Very basic pseudo environment mapping... and by that, I mean it's fake. :) However, it 
// does give the impression that the surface is reflecting the surrounds in some way.
//
// Anyway, the idea is very simple. Obtain the reflected (or refracted) ray at the surface hit point, then index into 
// a repeat texture in some way. It can be pretty convincing  (in an abstract way) and facilitates environment mapping 
// without the need for a cube map or a reflective pass.

vec3 envMap(vec3 rd, vec3 n)
{    
    return tex3D(rd, n);
}

void main()
{
    vec2 uv = ivec2(gl_GlobalInvocationID.xy) / vec2(1920.0f, 1080.0f);
    vec3 position = camera_ws;
    vec3 view = camera_matrix * vec3(focal_scale * (2.0f * uv - 1.0f), -1.0f);

    vec3 rd = normalize(view);
    float t = trace(position, rd);   
    vec3 sceneCol = vec3(0.0);                                                          // Initialize the scene color.
    
    if(t < HORIZON)                                                                     // The ray has effectively hit the surface, so light it up.
    {
        vec3 sp = position + rd * t;                                                    // Surface position and surface normal.
        vec3 sn = calcNormal(sp);                                                       // Voxel normal.
        vec3 snNoBump = sn;                                                             // Sometimes, it's necessary to save a copy of the unbumped normal.
        const float tSize0 = 0.5;

        sn = doBumpMap(sp * tSize0, sn, 0.1);                                           // Texture-based bump mapping.
        vec3 ld = light_ws - sp;                                                        // Light direction vectors.
        float lDist = max(length(ld), 0.001);                                           // Distance from respective lights to the surface point.
        ld /= lDist;                                                                    // Normalize the light direction vectors.
        float shading = shadows(sp + sn * 0.005, ld, 0.05, lDist, 8.0);                 // Shadows.
        float ao = calculateAO(sp, sn);                                                 // Ambient occlusion.
        float atten = 1.0 / (1.0 + lDist * 0.007);                                      // Light attenuation, based on the distances above.
        float diff = max(dot(sn, ld), 0.0);                                             // Diffuse lighting.
        float spec = pow(max(dot(reflect(-ld, sn), -rd), 0.0), 32.0);                   // Specular lighting.
        float fre = pow(clamp(dot(sn, rd) + 1.0, 0.0, 1.0), 1.0);                       // Fresnel term. Good for giving a surface a bit of a reflective glow.
        float ambience = 0.35 * ao + fre * fre * 0.25;                                  // Ambient light, due to light bouncing around the the canyon.
        vec3 texCol = tex3D(sp * tSize0, sn);                                           // Object texturing, coloring and shading.

        // Adding in the white frost. A bit on the cheap side, but it's a subtle effect.
        // As you can see, it's improvised, but from a physical perspective, you want the frost to accumulate
        // on the flatter surfaces, hence the "sn.y" factor. There's some Fresnel thrown in as well to give
        // it a tiny bit of sparkle.
        texCol = mix(texCol, vec3(0.35, 0.55, 1.0) * (texCol * 0.5 + 0.5) * vec3(2.0), ((snNoBump.y * 0.5 + sn.y * 0.5) * 0.5 + 0.5) * pow(abs(sn.y), 4.0) * texCol.r * fre * 4.0);

        sceneCol = texCol * (diff + spec + ambience);                                   // Final color. Pretty simple.
        sceneCol += texCol * ((sn.y) * 0.5 + 0.5) * min(vec3(1.0, 1.15, 1.5) * accum, 1.0);     // A bit of accumulated glow.  
        sceneCol += texCol * vec3(0.8, 0.95, 1.0) * pow(fre, 4.0) * 0.5;                // Adding a touch of Fresnel for a bit of glow.
        vec3 sn2 = snNoBump * 0.5 + sn * 0.5;                                           // Faux environmental mapping. Adds a bit more ambience.
        vec3 ref = reflect(rd, sn2);
        vec3 em = envMap(0.5 * ref, sn2);
        ref = refract(rd, sn2, 1.0 / 1.31);
        vec3 em2 = envMap(ref / 8.0, sn2);
        sceneCol += sceneCol * 2.0 * (sn.y * 0.25 + 0.75) * mix(em2, em, pow(fre, 4.0));
        sceneCol *= atten * min(shading + ao * 0.35, 1.0) * ao;                         // Shading. Adding some ambient occlusion to the shadow for some fake global lighting.    
    }

    // Blend in a bit of light fog for atmospheric effect. I really wanted to put a colorful, gradient blend here, but my mind wasn't buying it, so dull, blueish grey it is. :)
    vec3 fog = vec3(0.6, 0.8, 1.2) * (rd.y * 0.5 + 0.5);
    if (hell != 0) fog *= 4.0;
    sceneCol = mix(sceneCol, fog, smoothstep(0.0f, 0.95f, t / HORIZON));
    
    if (hell != 0)
    {
        float gr = dot(sceneCol, vec3(0.299, 0.587, 0.114));
        sceneCol = sceneCol * 0.1 + pow(min(vec3(1.5, 1.0, 1.0) * gr * 1.2, 1.0), vec3(1, 3, 16));
    }
    
    vec2 uv_n = 0.5 + 0.5 * uv;
    sceneCol = mix(vec3(0.0, 0.1, 1.0), sceneCol, pow(16.0 * uv_n.x * uv_n.y * (1.0 - uv_n.x) * (1.0 - uv_n.y), 0.125) * 0.15 + 0.85);

    vec4 FragmentColor = vec4(sqrt(clamp(sceneCol, 0.0, 1.0)), 1.0);

    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), FragmentColor);
}
