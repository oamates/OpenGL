#version 330 core

in vec3 geomNormal;
in vec3 geomTangent;
in vec3 geomBitangent;
in vec3 geomLightDir;
in vec3 geomViewDir;
in vec2 geomTexCoord;
in vec2 geomSTCoord;

uniform sampler2D ClothTex;
uniform sampler2D LightMap;

out vec4 FragmentColor;

void main()
{
    vec3 LightColor = texture(LightMap, geomSTCoord).rgb;
    vec3 Normal = normalize(geomNormal);
    vec3 LightDir = normalize(geomLightDir);
    vec3 LightRefl = reflect(-LightDir, Normal);
    float Specular = pow(clamp(dot(LightRefl, normalize(geomViewDir)) + 0.1, 0.0, 1.0), 16.0);
    float Diffuse = clamp(2.0 * (dot(Normal, LightDir) - 0.5), 0.0, 1.0);
    float Ambient = 0.275;
    vec3 Color = texture(ClothTex, geomTexCoord).rgb;
    FragmentColor = vec4(Color * Ambient + LightColor * Color * Diffuse + LightColor * Specular, 1.0);
}
