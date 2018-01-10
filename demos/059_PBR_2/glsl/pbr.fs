#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

//==============================================================================================================================================================
// IBL
//==============================================================================================================================================================
uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D brdf;

//==============================================================================================================================================================
// material parameters
//==============================================================================================================================================================
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_map;
uniform sampler2D roughness_map;
uniform sampler2D ao_map;

//==============================================================================================================================================================
// lights
//==============================================================================================================================================================
const int light_count = 4;
uniform vec3 light_positions[light_count];
uniform vec3 light_colors[light_count];

uniform vec3 camera_ws;

out vec4 FragmentColor;


const float pi = 3.14159265359f;

//==============================================================================================================================================================
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
//==============================================================================================================================================================
vec3 getNormalFromMap()
{
    vec3 n = 2.0 * texture(normal_map, uv).xyz - 1.0f;

    vec3 Q1  = dFdx(position_ws);
    vec3 Q2  = dFdy(position_ws);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N   = normalize(normal_ws);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * n);
}

//==============================================================================================================================================================
// Trowbridge-Reitz model of Normal Distribution Function depending on roughness parameter
// -- approximates the amount of surface microfacets aligned to halfway vector
//==============================================================================================================================================================
float NDF(vec3 n, vec3 h, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float dp = max(dot(n, h), 0.0);
    float q = 1.0f + (alpha2 - 1.0f) * dp * dp;
    float s = pi * q * q;
    return alpha2 / s;
}

//==============================================================================================================================================================
// GeometrySchlickGGX
//==============================================================================================================================================================
float GeometrySchlickGGX(float dp, float roughness)
{
    float r = 1.0 + roughness;
    float k = 0.125f * r * r;
    return dp / (dp * (1.0 - k) + k);
}

//==============================================================================================================================================================
// Geometry Function
// -- describes self-shadowing properties of the surface
//==============================================================================================================================================================
float GF(vec3 n, vec3 v, vec3 l, float roughness)
{
    float nv = max(dot(n, v), 0.0);
    float nl = max(dot(n, l), 0.0);
    float ggx_v = GeometrySchlickGGX(nv, roughness);
    float ggx_l = GeometrySchlickGGX(nl, roughness);
    return ggx_v * ggx_l;
}

//==============================================================================================================================================================
// Schlick approximation of the Fresnel coefficient
//==============================================================================================================================================================
vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

//==============================================================================================================================================================
// Fresnel-Schlick roughness
//==============================================================================================================================================================
vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    //==========================================================================================================================================================
    // material properties
    //==========================================================================================================================================================
    vec3 albedo = pow(texture(albedo_map, uv).rgb, vec3(2.2));
    float metallness = texture(metallic_map, uv).r;
    float roughness = texture(roughness_map, uv).r;
    float ao = texture(ao_map, uv).r;

    //==========================================================================================================================================================
    // input lighting data
    //==========================================================================================================================================================
    vec3 n = getNormalFromMap();                        /* world-space normal */
    vec3 v = normalize(camera_ws - position_ws);        /* view direction, from fragment to eye */
    vec3 r = reflect(-v, n);                            /* reflected view direction */

    //==========================================================================================================================================================
    // calculate reflectance at normal incidence;
    // for di-electric (like plastic) use F0 = 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)
    //==========================================================================================================================================================
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallness);

    //==========================================================================================================================================================
    // reflectance equation
    //==========================================================================================================================================================
    vec3 Lo = vec3(0.0);

    for(int i = 0; i < light_count; ++i)
    {
        //======================================================================================================================================================
        // calculate per-light radiance
        //======================================================================================================================================================
        vec3 light = light_positions[i] - position_ws;
        float distance = length(light);
        vec3 l = light / distance;
        vec3 h = normalize(v + l);                      /* half-view vector */
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light_colors[i] * attenuation;

        //======================================================================================================================================================
        // Cook-Torrance BRDF
        //======================================================================================================================================================
        float ndf = NDF(n, h, roughness);                       /* normal distribution function */
        float G = GF(n, v, l, roughness);                       /* geometry self-shadowing distribution */
        vec3 F = fresnel_schlick(max(dot(h, v), 0.0), F0);      /* Fresnel coefficient */

        vec3 nominator = ndf * G * F;
        float NdotV = max(dot(n, v), 0.0);
        float NdotL = max(dot(n, l), 0.0);
        float denominator = 4 * NdotV * NdotL + 0.001;
        vec3 brdf = nominator / denominator;

        //======================================================================================================================================================
        // kS is equal to Fresnel
        //======================================================================================================================================================
        vec3 kS = F;

        //======================================================================================================================================================
        // for energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light);
        // to preserve this relationship the diffuse component (kD) should equal 1.0 - kS.
        //======================================================================================================================================================
        vec3 kD = vec3(1.0) - kS;

        //======================================================================================================================================================
        // multiply kD by the inverse metalness such that only non-metals have diffuse lighting,
        // or a linear blend if partly metal (pure metals have no diffuse light)
        // the rest of unused energy will heat the metal
        //======================================================================================================================================================
        kD *= 1.0 - metallness;

        //======================================================================================================================================================
        // scale light by NdotL and add to outgoing radiance Lo,
        // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        //======================================================================================================================================================
        Lo += (kD * albedo / pi + brdf) * radiance * NdotL; //
    }

    //==========================================================================================================================================================
    // ambient lighting (we now use IBL as the ambient term)
    //==========================================================================================================================================================
    vec3 F = fresnel_schlick_roughness(max(dot(n, v), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallness;

    vec3 irradiance = texture(irradiance_map, n).rgb;
    vec3 diffuse = irradiance * albedo;

    //==========================================================================================================================================================
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    //==========================================================================================================================================================
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefiltered_color = textureLod(prefilter_map, r,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdf, vec2(max(dot(n, v), 0.0), roughness)).rg;
    vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 color = ambient + Lo;

    //==========================================================================================================================================================
    // HDR tonemapping and gamma correct
    //==========================================================================================================================================================
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragmentColor = vec4(color, 1.0f);
}
