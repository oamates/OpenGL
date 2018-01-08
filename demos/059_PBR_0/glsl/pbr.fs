#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

//==============================================================================================================================================================
// material parameters
//==============================================================================================================================================================
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_map;
uniform sampler2D roughness_map;
uniform sampler2D ao_map;

//==============================================================================================================================================================
// camera and lights
//==============================================================================================================================================================
uniform vec3 camera_ws;

const int light_count = 4;
uniform vec3 light_positions[light_count];
uniform vec3 light_colors[light_count];


out vec4 FragmentColor;

const float pi = 3.14159265359f;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normal_map, uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(position_ws);
    vec3 Q2  = dFdy(position_ws);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N   = normalize(normal_ws);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    vec3 albedo     = pow(texture(albedo_map, uv).rgb, vec3(2.2));
    float metallic  = texture(metallic_map, uv).r;
    float roughness = texture(roughness_map, uv).r;
    float ao        = texture(ao_map, uv).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camera_ws - position_ws);

    //==========================================================================================================================================================
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    //==========================================================================================================================================================
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    //==========================================================================================================================================================
    // reflectance equation
    //==========================================================================================================================================================
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < light_count; ++i)
    {
        //======================================================================================================================================================
        // calculate per-light radiance
        //======================================================================================================================================================
        vec3 L = normalize(light_positions[i] - position_ws);
        vec3 H = normalize(V + L);
        float distance = length(light_positions[i] - position_ws);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light_colors[i] * attenuation;

        //======================================================================================================================================================
        // Cook-Torrance BRDF
        //======================================================================================================================================================
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        //======================================================================================================================================================
        // kS is equal to Fresnel
        //======================================================================================================================================================
        vec3 kS = F;

        //======================================================================================================================================================
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        //======================================================================================================================================================
        vec3 kD = vec3(1.0) - kS;

        //======================================================================================================================================================
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        //======================================================================================================================================================
        kD *= 1.0 - metallic;

        //======================================================================================================================================================
        // scale light by NdotL
        //======================================================================================================================================================
        float NdotL = max(dot(N, L), 0.0);

        //======================================================================================================================================================
        // add to outgoing radiance Lo
        //======================================================================================================================================================
        Lo += (kD * albedo / pi + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    //==========================================================================================================================================================
    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    //==========================================================================================================================================================
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    //==========================================================================================================================================================
    // HDR tonemapping and gamma correction
    //==========================================================================================================================================================
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragmentColor = vec4(color, 1.0);
}