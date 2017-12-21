#version 330 core

in vec3 vertNormal;
in vec3 vertLightDir;
in vec4 vertShadowCoord;

const vec3 LightColor = vec3(0.6, 0.6, 1.0);
const vec3 Up = normalize(vec3(0.1, 1.0, 0.1));

uniform vec3 LightScreenPos;
uniform vec2 ScreenSize;
uniform sampler2DRect LightMap;
uniform sampler2DShadow ShadowMap;

out vec3 fragColor;

void main()
{
    vec3 ShadowCoord = (vertShadowCoord.xyz/vertShadowCoord.w)*0.5 + 0.5;"
    float s = 0.0f;"
    if (ShadowCoord.x >= 0.0 && ShadowCoord.x <= 1.0 && 
        ShadowCoord.y >= 0.0 && ShadowCoord.y <= 1.0 &&
        ShadowCoord.z <= 1.0)
        s = texture(ShadowMap, ShadowCoord);

    float a = 0.1 * (max(dot(vertNormal, Up) + 0.1, 0.0) + 0.1);
    float d = max(dot(vertNormal, vertLightDir) + 0.1, 0.0) + a;

    vec2 LMCoord = gl_FragCoord.xy;
    vec2 LPos = (LightScreenPos.xy * 0.5 + 0.5) * ScreenSize;
    vec2 Ray = LMCoord - LPos;
    float Len = length(Ray);
    int NSampl = int(max(abs(Ray.x), abs(Ray.y))) + 1;
    vec2 RayStep = Ray / NSampl;
    float r = texture(LightMap, LMCoord).r;

    NSampl = min(NSampl, int(min(ScreenSize.x, ScreenSize.y) * 0.25));

    for (int s = 0; s != NSampl; ++s)
    {
        r += texture(LightMap, LPos+RayStep*s).r;
    }

    r /= NSampl;
    r = min(r, 1.0);

    fragColor = LightColor * (mix(a, d, s) + r);
}