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

uniform int frame;

out vec4 FragmentColor;

//==============================================================================================================================================================
// FILTER FUNCTIONS
//==============================================================================================================================================================
const float pi = 3.14159265359f;
const float LANCZOS_PARAMETER = 1.0f / 1.3f;
uniform float sharpness = 2.0f;

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
    if (r2 == 0.0f) return 1.0f;
    float r = sqrt(r2);
    return sinc(r) * sinc(LANCZOS_PARAMETER * r);
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
    float q = sqrt(r2) / sharpness;
    return 1.0f - q * q * (3.0f - 2.0f * q);
}

float filter_func(float q)
{
    return gauss_filter(q);
}

//==============================================================================================================================================================
// function that selects mipmap level based on the length of major ellipse axis
// this matches exactly to how Radeon performs mipmap level selection for GL_LINEAR_MIPMAP_LINEAR filtering (no anisotropy enabled)
//==============================================================================================================================================================
float mip_level_major_axis(vec2 duv_dx, vec2 duv_dy)
{
    vec2 size = textureSize(mipmap_mode_tex, 0);
    float max_level = log2(max(size.x, size.y));

    mat2 jacobian = mat2(duv_dx, duv_dy);
    mat2 qform = jacobian * transpose(jacobian);

    float A = qform[0][0];
    float B = qform[0][1];
    float C = qform[1][1];
    float Q = C - A;
    float sp = C + A;
    float R = sqrt(Q * Q + 4.0f * B * B);
    float major_axis_sqr = 0.5f * (sp + R);
    //float minor_axis_sqr = sp - major_axis_sqr;

    return clamp(0.5 * log2(major_axis_sqr), 0.0f, max_level);
}

//==============================================================================================================================================================
// EWA filter parameters
//==============================================================================================================================================================
const int NUM_PROBES = 6;
const int TEXEL_LIMIT = 128;

const float MAX_ECCENTRICITY = 16.0f;
const float FILTER_WIDTH = 1.0f;
const float TEXELS_PER_PIXEL = 1.0f;

//==============================================================================================================================================================
// EWA filter : reference implementation
//==============================================================================================================================================================

vec4 ewa00(vec2 uv)
{
    vec2 size = textureSize(mipmap_mode_tex, 0);
    float max_level = log2(max(size.x, size.y));

    vec2 unorm_uv = size * uv;
    vec2 duv_dx = dFdx(unorm_uv);
    vec2 duv_dy = dFdy(unorm_uv);

    mat2 jacobian = mat2(duv_dx, duv_dy);
    mat2 qform = jacobian * transpose(jacobian);

    float A = qform[1][1];
    float B = qform[0][1];
    float C = qform[0][0];
    float Q = C - A;
    float sp = C + A;
    float R = sqrt(Q * Q + 4.0f * B * B);
    float major_axis_sqr = 0.5f * (sp + R);
    float minor_axis_sqr = max(sp - major_axis_sqr, 0.5);

    float majorLength = sqrt(major_axis_sqr);
    float minorLength = sqrt(minor_axis_sqr);

    const float maxEccentricity = MAX_ECCENTRICITY;

    float e = majorLength / minorLength;
    minorLength *= max(e / maxEccentricity, 1.0f);
    
    float lod = log2(minorLength);  
    lod = clamp(lod, 0.0, max_level);


//    float lod = clamp(0.5 * log2(major_axis_sqr), 0.0f, max_level);

    ivec2 iscale = textureSize(mipmap_mode_tex, int(floor(lod)));
    vec2 scale = vec2(iscale);
    unorm_uv = scale * uv;
    vec2 p = unorm_uv - vec2(0.5f);
    duv_dx = dFdx(unorm_uv);
    duv_dy = dFdy(unorm_uv);

    if (iscale.x * iscale.y <= 2)
        return textureLod(mipmap_mode_tex, uv, lod);


    vec2 inv_scale = 1.0f / scale;


    jacobian = mat2(duv_dx, duv_dy);
    qform = jacobian * transpose(jacobian);

    //==========================================================================================================================================================
    // compute ellipse coefficients Axx + 2Bxy + Cyy = F
    //==========================================================================================================================================================
    A =  qform[1][1] + 1.0f;
    B = -qform[0][1];
    C =  qform[0][0] + 1.0f;
    float F = determinant(qform);

    //==========================================================================================================================================================
    // Compute the ellipse's (u,v) bounding box in texture space
    //==========================================================================================================================================================
    vec2 ebbox_d = sqrt(vec2(C, A)); 

    //==========================================================================================================================================================
    // compute the symmetric bounding box, clamp it so that it includes at most TEXEL_LIMIT texels
    // then compute u, v bounds to loop over
    //==========================================================================================================================================================
    float ebbox_area = ebbox_d.s * ebbox_d.t;
    ebbox_d *= min(sqrt(TEXEL_LIMIT / ebbox_area), 1.0f);

    vec2 ebbox_min = floor(p - ebbox_d);
    vec2 ebbox_max = ceil (p + ebbox_d);

    //==========================================================================================================================================================
    // Heckbert MS thesis, p. 59: scan over the bounding box of the ellipse and incrementally update the value of Axx + Bxy + Cyy; when this
    // value, q, is less than F, we're inside the ellipse so add weighted texel to the total sum
    //==========================================================================================================================================================
    vec4 num = vec4(0.0f);
    float total_weight = 0.0f;
    float ddq = 2.0f * A;
    float U = ebbox_min.s - p.s;

    float v = ebbox_min.t;
    while (v <= ebbox_max.t)
    {
        float V = v - p.t;
        float dq = A + 2.0f * (A * U + B * V);
        float q = (C * V + 2.0f * B * U) * V + A * U * U;

        float u = ebbox_min.s;
        while (u <= ebbox_max.s)
        {
            if (q < F) 
            {
                float r2 = q / F;
                float weight = filter_func(r2);
                num += weight * textureLod(mipmap_mode_tex, inv_scale * vec2(u + 0.5f, v + 0.5f), lod);
                total_weight += weight;
            }
            q += dq;
            dq += ddq;
            u += 1.0f;
        }
        v += 1.0f;
    }

    vec4 color = num * (1.0f / total_weight);
    return color;

}

vec4 ewa(sampler2D sampler, vec2 uv, vec2 duv_dx, vec2 duv_dy, float lod, int psize)
{
    int scale = psize >> int(lod);
        
    //==========================================================================================================================================================
    // use regular filtering if the scale is very small
    //==========================================================================================================================================================
    if(scale < 2)                                           
        return texture(sampler, uv);

    float inv_scale = 1.0f / scale;

    vec2 p = scale * uv - vec2(0.5f);

    float S = FILTER_WIDTH * scale;
    float ux = S * duv_dx.s;
    float vx = S * duv_dx.t;

    float uy = S * duv_dy.s;
    float vy = S * duv_dy.t;

    //==========================================================================================================================================================
    // compute ellipse coefficients Axx + 2Bxy + Cyy = F
    //==========================================================================================================================================================
    float A = vx * vx + vy * vy + 1.0f;
    float B = -(ux * vx + uy * vy);
    float C = ux * ux + uy * uy + 1.0f;
    float F = A * C - B * B;

    //==========================================================================================================================================================
    // Compute the ellipse's (u,v) bounding box in texture space
    //==========================================================================================================================================================
    vec2 ebbox_d = sqrt(vec2(C, A)); 

    //==========================================================================================================================================================
    // compute the symmetric bounding box, clamp it so that it includes at most TEXEL_LIMIT texels
    // then compute u, v bounds to loop over
    //==========================================================================================================================================================
    float ebbox_area = ebbox_d.s * ebbox_d.t;
    ebbox_d *= min(sqrt(TEXEL_LIMIT / ebbox_area), 1.0f);

    vec2 ebbox_min = floor(p - ebbox_d);
    vec2 ebbox_max = ceil (p + ebbox_d);

    //==========================================================================================================================================================
    // Heckbert MS thesis, p. 59: scan over the bounding box of the ellipse and incrementally update the value of Axx + Bxy + Cyy; when this
    // value, q, is less than F, we're inside the ellipse so add weighted texel to the total sum
    //==========================================================================================================================================================
    vec4 num = vec4(0.0f);
    float total_weight = 0.0f;
    float ddq = 2.0f * A;
    float U = ebbox_min.s - p.s;

    float v = ebbox_min.t;
    while (v <= ebbox_max.t)
    {
        float V = v - p.t;
        float dq = A + 2.0f * (A * U + B * V);
        float q = (C * V + 2.0f * B * U) * V + A * U * U;

        float u = ebbox_min.s;
        while (u <= ebbox_max.s)
        {
            if (q < F) 
            {
                float r2 = q / F;
                float weight = filter_func(r2);
                num += weight * textureLod(sampler, inv_scale * vec2(u + 0.5f, v + 0.5f), lod - 2.0);
                total_weight += weight;
            }
            q += dq;
            dq += ddq;
            u += 1.0f;
        }
        v += 1.0f;
    }

    vec4 color = num * (1.0f / total_weight);
    return color;
}


//==============================================================================================================================================================
// EWA filter : 2-tex implementation
//==============================================================================================================================================================
vec4 ewa2tex(sampler2D sampler, vec2 uv, vec2 du, vec2 dv, float lod, int psize)
{
    int scale = psize >> int(lod);
    vec4 foo = texture(sampler, uv);
    
    // don't bother with elliptical filtering if the scale is very small
    if(scale < 2)
        return foo;

    vec2 p = scale * uv - vec2(0.5f);

    float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

    // compute ellipse coefficients :: Axx + Bxy + Cyy = F.
    float A = vx * vx + vy * vy + 1.0f;
    float B = -2.0f * (ux * vx + uy * vy);
    float C = ux * ux + uy * uy + 1.0f;
    float F = A * C - 0.25f * B * B;

    // Compute the ellipse's (u,v) bounding box in texture space
    float bbox_du = sqrt(C);
    float bbox_dv = sqrt(A);

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

    for (int v = v0; v <= v1; ++v)
    {
        float V = v - p.t;
        float dq = A * (2 * U + 1) + B * V;
        float q = (C * V + B * U) * V + A * U * U;
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
                num += weight * textureLod(sampler, vec2(u + 0.5f + offest, v + 0.5f) / scale , int(lod));
                den += weight;
            }
            q += dq;
            dq += ddq;
        }
    }
    vec4 color = num * (1.0f / den);
    return color;
}

//==============================================================================================================================================================
// EWA filter : 4-tex implementation
//==============================================================================================================================================================
vec4 ewa4tex(sampler2D sampler, vec2 p0, vec2 du, vec2 dv, float lod, int psize)
{
    int scale = psize >> int(lod);
    vec4 foo = texture(sampler, p0);
    
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

        //  float Error = (w1*w4-w2*w3);
            if(weight > 0.1f)
            {
                num += weight * textureLod(sampler, vec2(u + offest_u + 0.5, v + offest_v + 0.5) / scale , int(lod));
                den += weight;
            }
        }
    }

    vec4 color = num * (1.0f / den);
    return color;
}

//==============================================================================================================================================================
// mip-map level selection routine
//==============================================================================================================================================================
vec2 textureQueryLOD_EWA(sampler2D sampler, vec2 duv_dx, vec2 duv_dy, int size)
{
    int scale = size;

    float ux = duv_dx.s * scale;
    float vx = duv_dx.t * scale;

    float uy = duv_dy.s * scale;
    float vy = duv_dy.t * scale;

    //==========================================================================================================================================================
    // compute ellipse coefficients Axx + 2Bxy + Cyy = F
    //==========================================================================================================================================================
    float A = vx * vx + vy * vy + 1.0f;
    float B = -(ux * vx + uy * vy);
    float C = ux * ux + uy * uy + 1.0f;
    float F = A * C - B * B;
    float inv_F = 1.0f / F;
        
    A *= inv_F;
    B *= inv_F;
    C *= inv_F;
    
    float root = sqrt((A - C) * (A - C) + 4.0f * B * B);
    float majorRadius = sqrt(2.0f / (A + C - root));
    float minorRadius = sqrt(2.0f / (A + C + root));

    float majorLength = majorRadius;
    float minorLength = max(minorRadius, 0.01);

    const float maxEccentricity = MAX_ECCENTRICITY;

    float e = majorLength / minorLength;

    if (e > maxEccentricity)
        minorLength *= (e / maxEccentricity);
    
    float lod = log2(minorLength / TEXELS_PER_PIXEL);  
    lod = clamp(lod, 0.0, log2(size));

    return vec2(lod, e);
}

//==============================================================================================================================================================
// helper : visualize the absolute deviation between hardware and software lod selection
//==============================================================================================================================================================
vec4 lodError(sampler2D sampler, vec2 uv)
{
    vec2 duv_dx = dFdx(uv);
    vec2 duv_dy = dFdy(uv);
    
    int size = textureSize(sampler, 0).x;

    float lod1 = textureQueryLOD(sampler, uv).x;
    float lod2 = textureQueryLOD_EWA(sampler, duv_dx, duv_dy, size).x;

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
vec4 anisotropyLevel(sampler2D sampler, vec2 uv)
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
// approximate EWA
//==============================================================================================================================================================
vec4 approximate_ewa(sampler2D sampler, vec2 uv)
{
    vec2 du = dFdx(uv);
    vec2 dv = dFdy(uv);
    
    int psize = textureSize(sampler, 0).x;

    int scale = psize;
    scale = 1;

    vec2 p = scale * uv;

    float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

    // compute ellipse coefficients to bound the region: Axx + Bxy + Cyy = F.
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
    
    for(int i = 1; i < iProbes / 2; i++)
    {
        float d =  (0.5f * float(i)) * length(vec2(dpu, dpv)) / lineLength;
        float weight = filter_func(d);

        num += weight * texture(sampler, uv + (i * vec2(dpu, dpv)) / scale);
        num += weight * texture(sampler, uv - (i * vec2(dpu, dpv)) / scale);

        den += weight;
        den += weight;
    }

    return (1.0 / den) * num;
}

//==============================================================================================================================================================
// approximate EWA spatial
//==============================================================================================================================================================
vec4 approximate_ewa_spatial(sampler2D sampler, vec2 uv)
{
    vec2 du = dFdx(uv);
    vec2 dv = dFdy(uv);
    
    int psize = textureSize(sampler, 0).x;

    float vlod = textureQueryLOD_EWA(sampler, du, dv, psize).y;

    vec4 hcolor = texture(sampler, uv);
    if(vlod < 12)
        return hcolor;

    int scale = psize;
    scale = 1;

    vec2 p = scale * uv;

    float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

    // compute ellipse coefficients to bound the region: Axx + Bxy + Cyy = F
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
    
    for(int i = 1; i < iProbes / 2; i++)
    {
        float d =  (0.5f * float(i)) * length(vec2(dpu, dpv)) / lineLength;
        float weight = filter_func(d);

        num += weight * texture(sampler, uv + (i * vec2(dpu, dpv)) / scale);
        num += weight * texture(sampler, uv - (i * vec2(dpu, dpv)) / scale);

        den += weight;
        den += weight;
    }

    vec4 scolor = (1.0f / den) * num;
    return mix(hcolor, scolor, smoothstep(0.0f, 1.0f, (vlod - 8.0f) / 13.0f));
}

//==============================================================================================================================================================
// approximate EWA temporal
//==============================================================================================================================================================
vec4 approximate_ewa_temporal(sampler2D sampler, vec2 uv)
{
    vec2 du = dFdx(uv);
    vec2 dv = dFdy(uv);
    
    int psize = textureSize(sampler, 0).x;

    int scale = psize;
    scale = 1;

    vec2 p = scale * uv;

    float ux = FILTER_WIDTH * du.s * scale;
    float vx = FILTER_WIDTH * du.t * scale;
    float uy = FILTER_WIDTH * dv.s * scale;
    float vy = FILTER_WIDTH * dv.t * scale;

    // compute ellipse coefficients to bound the region: Axx + Bxy + Cyy = F
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

    return (1.0 / den) * num;
}

//==============================================================================================================================================================
// texture filtering subroutines
//==============================================================================================================================================================
subroutine vec4 texture_filter_func(vec2 uv);
subroutine uniform texture_filter_func texture_filter;

subroutine(texture_filter_func) vec4 textureGrad_HW(vec2 uv)
{
    return textureGrad(anisotropic_mode_tex, uv, dFdx(uv), dFdy(uv));
}

subroutine(texture_filter_func) vec4 area_distortion_HW(vec2 uv)
{
    vec2 size = textureSize(mipmap_mode_tex, 0);
    float max_level = log2(max(size.x, size.y));

    vec2 unorm_uv = size * uv;
    vec2 duv_dx = dFdx(unorm_uv);
    vec2 duv_dy = dFdy(unorm_uv);


    float rho_sqr = max(dot(duv_dx, duv_dx), dot(duv_dy, duv_dy));
    float lambda = 0.5 * log2(rho_sqr);
//    float rho_sqr = max(length(duv_dx), length(duv_dy));
//    float lambda = log2(rho_sqr);
    float sw_lod = clamp(lambda, 0.0f, max_level);

    float rho_sqr2 = max(abs(duv_dx.x) + abs(duv_dx.y), abs(duv_dy.x) + abs(duv_dy.y));
    float lambda2 = log2(rho_sqr2);
    float sw_lod2 = clamp(lambda2, 0.0f, max_level);

    mat2 jacobian = mat2(duv_dx, duv_dy);
    float area = determinant(jacobian);
    float rho3 = log2(abs(area));
    float sw_lod3 = clamp(rho3, 0.0f, max_level);


    float hw_lod = textureQueryLOD(mipmap_mode_tex, uv).x;

    float diff = abs(-hw_lod + mip_level_major_axis(duv_dx, duv_dy));

    return vec4(vec3(diff), 1.0f);
}

subroutine(texture_filter_func) vec4 nearest_filter_HW(vec2 uv)
{
    return texture(nearest_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 linear_filter_HW(vec2 uv)
{
    return texture(linear_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 mipmap_filter_HW(vec2 uv)
{
    return texture(mipmap_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 anisotropic_filter_HW(vec2 uv)
{
    return texture(anisotropic_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 linear_filter_SW(vec2 uv)
{
    vec2 size = textureSize(nearest_mode_tex, 0);

    vec2 texel_size = 1.0f / size;
    vec2 P = size * uv - 0.5f;
    vec2 Pf = fract(P);
    vec2 Pi = P - Pf;

    vec4 offset = vec4(0.5f, 0.5f, 1.5f, 1.5f) * texel_size.xyxy;
    vec2 base = texel_size * Pi;

    vec4 texel00 = texture(nearest_mode_tex, base + offset.xy);
    vec4 texel10 = texture(nearest_mode_tex, base + offset.zy);
    vec4 texel01 = texture(nearest_mode_tex, base + offset.xw);
    vec4 texel11 = texture(nearest_mode_tex, base + offset.zw);

    vec4 texel_y0 = mix(texel00, texel10, Pf.x);
    vec4 texel_y1 = mix(texel01, texel11, Pf.x);

    vec4 texel = mix(texel_y0, texel_y1, Pf.y);
    return texel;
}

vec4 cubic(float v)
{
    vec4 n = vec4(1.0f, 2.0f, 3.0f, 4.0f) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

subroutine(texture_filter_func) vec4 bicubic_filter_SW(vec2 uv)
{
    vec2 size = textureSize(linear_mode_tex, 0);
    vec2 texel_size = 1.0f / size;
   
    vec2 P = uv * size - 0.5;
   
    vec2 Pf = fract(P);
    vec2 Pi = P - Pf;

    vec4 xcubic = cubic(Pf.x);
    vec4 ycubic = cubic(Pf.y);

    vec4 c = Pi.xxyy + vec4(-0.5f, 1.5f, -0.5f, 1.5f);
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;
    
    offset *= texel_size.xxyy;
    
    vec4 sample00 = texture(linear_mode_tex, offset.xz);
    vec4 sample10 = texture(linear_mode_tex, offset.yz);
    vec4 sample01 = texture(linear_mode_tex, offset.xw);
    vec4 sample11 = texture(linear_mode_tex, offset.yw);

    s.xy = s.xz / (s.xz + s.yw);

    return mix(mix(sample11, sample01, s.x), mix(sample10, sample00, s.x), s.y);
}

//#define MIPMAP_SW_FILTER_USE_MIPMAP_SAMPLING
//#define MIPMAP_SW_FILTER_USE_LINEAR_SAMPLING
#define MIPMAP_SW_FILTER_USE_NEAREST_SAMPLING
#define MIPMAP_SW_FILTER_USE_HW_LOD

subroutine(texture_filter_func) vec4 mipmap_filter_SW(vec2 uv)
{
    vec2 size;
    float lod;

#ifdef MIPMAP_SW_FILTER_USE_HW_LOD

    lod = textureQueryLOD(mipmap_mode_tex, uv).x;

#else       /* COMPUTE LOD AS A MAJOR ELLIPTIC FOOTPRINT AXIS */ 

    size = textureSize(mipmap_mode_tex, 0);
    float max_lod = log2(max(size.x, size.y));

    vec2 unorm_uv = size * uv;
    vec2 duv_dx = dFdx(unorm_uv);
    vec2 duv_dy = dFdy(unorm_uv);

    mat2 jacobian = mat2(duv_dx, duv_dy);
    mat2 qform = jacobian * transpose(jacobian);

    float A = qform[0][0];
    float B = qform[0][1];
    float C = qform[1][1];
    float Q = C - A;
    float sp = C + A;
    float R = sqrt(Q * Q + 4.0f * B * B);
    float major_axis_sqr = 0.5f * (sp + R);
    lod = clamp(0.5 * log2(major_axis_sqr), 0.0f, max_lod);

#endif

#ifdef MIPMAP_SW_FILTER_USE_MIPMAP_SAMPLING

    return textureLod(mipmap_mode_tex, uv, lod);

#else

    float mip_l = floor(lod);
    float mip_h = mip_l + 1.0f;
    float f = lod - mip_l;

  #ifdef MIPMAP_SW_FILTER_USE_LINEAR_SAMPLING

    vec4 texel_l = textureLod(mipmap_mode_tex, uv, mip_l);
    vec4 texel_h = textureLod(mipmap_mode_tex, uv, mip_h);
    return mix(texel_l, texel_h, f);

  #else     /* NEAREST SAMPLING */

    size = textureSize(mipmap_mode_tex, int(mip_l));
    vec2 texel_size = 1.0f / size;
    vec2 P = size * uv - 0.5f;
    vec2 Pf = fract(P);
    vec2 Pi = P - Pf;

    vec4 offset = vec4(0.5f, 0.5f, 1.5f, 1.5f) * texel_size.xyxy;
    vec2 base = texel_size * Pi;

    vec4 texel00 = textureLod(mipmap_mode_tex, base + offset.xy, mip_l);
    vec4 texel10 = textureLod(mipmap_mode_tex, base + offset.zy, mip_l);
    vec4 texel01 = textureLod(mipmap_mode_tex, base + offset.xw, mip_l);
    vec4 texel11 = textureLod(mipmap_mode_tex, base + offset.zw, mip_l);

    vec4 texel_y0 = mix(texel00, texel10, Pf.x);
    vec4 texel_y1 = mix(texel01, texel11, Pf.x);

    vec4 texel_l = mix(texel_y0, texel_y1, Pf.y);

    size = textureSize(mipmap_mode_tex, int(mip_h));
    texel_size = 1.0f / size;
    P = size * uv - 0.5f;
    Pf = fract(P);
    Pi = P - Pf;

    offset = vec4(0.5f, 0.5f, 1.5f, 1.5f) * texel_size.xyxy;
    base = texel_size * Pi;

    texel00 = textureLod(mipmap_mode_tex, base + offset.xy, mip_h);
    texel10 = textureLod(mipmap_mode_tex, base + offset.zy, mip_h);
    texel01 = textureLod(mipmap_mode_tex, base + offset.xw, mip_h);
    texel11 = textureLod(mipmap_mode_tex, base + offset.zw, mip_h);

    texel_y0 = mix(texel00, texel10, Pf.x);
    texel_y1 = mix(texel01, texel11, Pf.x);

    vec4 texel_h = mix(texel_y0, texel_y1, Pf.y);
    return mix(texel_l, texel_h, f);    

  #endif

#endif 
}

//#define USE_HARDWARE_LOD

subroutine(texture_filter_func) vec4 ewa_SW(vec2 uv)
{/*
    vec2 duv_dx = dFdx(uv);
    vec2 duv_dy = dFdy(uv);
    
    int size = textureSize(mipmap_mode_tex, 0).x;
    float lod;
  #ifdef USE_HARDWARE_LOD
    lod = textureQueryLOD(mipmap_mode_tex, uv).x;
  #else
    lod = textureQueryLOD_EWA(mipmap_mode_tex, duv_dx, duv_dy, size).x;
  #endif

    return ewa(mipmap_mode_tex, uv, duv_dx, duv_dy, lod, size);
*/
    return ewa00(uv);
}

subroutine(texture_filter_func) vec4 ewa2tex_SW(vec2 uv)
{
    vec2 duv_dx = dFdx(uv);
    vec2 duv_dy = dFdy(uv);
    
    int size = textureSize(mipmap_mode_tex, 0).x;
    float lod;
  #ifdef USE_HARDWARE_LOD
    lod = textureQueryLOD(mipmap_mode_tex, uv).x;
  #else
    lod = textureQueryLOD_EWA(mipmap_mode_tex, duv_dx, duv_dy, size).x;
  #endif
    return ewa2tex(mipmap_mode_tex, uv, duv_dx, duv_dy, lod, size);
}

subroutine(texture_filter_func) vec4 ewa4tex_SW(vec2 uv)
{
    vec2 duv_dx = dFdx(uv);
    vec2 duv_dy = dFdy(uv);
    
    int size = textureSize(mipmap_mode_tex, 0).x;
    float lod;
  #ifdef USE_HARDWARE_LOD
    lod = textureQueryLOD(mipmap_mode_tex, uv).x;
  #else
    lod = textureQueryLOD_EWA(mipmap_mode_tex, duv_dx, duv_dy, size).x;
  #endif
    return ewa4tex(mipmap_mode_tex, uv, duv_dx, duv_dy, lod, size);
}

subroutine(texture_filter_func) vec4 approximate_ewa_SW(vec2 uv)
{
    return approximate_ewa(mipmap_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 approximate_ewa_spatial_SW(vec2 uv)
{
    return approximate_ewa_spatial(mipmap_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 approximate_ewa_temporal_SW(vec2 uv)
{
    return approximate_ewa_temporal(mipmap_mode_tex, uv);
}

//==============================================================================================================================================================
// helper / debugging filtering subroutines
//==============================================================================================================================================================
subroutine(texture_filter_func) vec4 lodError_SW(vec2 uv)
{
    return lodError(mipmap_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 anisotropyLevel_SW(vec2 uv)
{
    return anisotropyLevel(mipmap_mode_tex, uv);
}

subroutine(texture_filter_func) vec4 mipLevel_SW(vec2 uv)
{
    return mipLevel(mipmap_mode_tex, uv);
}

void main()
{
    FragmentColor = texture_filter(uv);
}