#version 330 core

in vec3 vertNormal;
in vec3 vertLightDir;
in vec4 vertShadowCoord;

uniform sampler2DShadow ShadowMap;

out float fragIntensity;

void main()
{
    vec3 ShadowCoord = 0.5 + 0.5 * (vertShadowCoord.xyz / vertShadowCoord.w);
    float s = 0.0f;

    if (ShadowCoord.x >= 0.0 && ShadowCoord.x <= 1.0 &&
        ShadowCoord.y >= 0.0 && ShadowCoord.y <= 1.0 &&
        ShadowCoord.z <= 1.0)
        s = max(texture(ShadowMap, ShadowCoord), 0.05);

    float l = max(dot(vertNormal, vertLightDir) + 0.1, 0.0);
    fragIntensity = l * s;
}