#version 330 core

in vec3 geomNormal;
in vec3 geomTangent;
in vec3 geomBitangent;
in vec3 geomLightDir;
in vec3 geomViewDir;
in vec2 geomTexCoord;
in vec2 geomSTCoord;

uniform vec3 Color1;
uniform vec3 Color2;
uniform sampler2D ClothTex;
uniform sampler2D LightMap;

out vec4 FragmentColor;

void main()
{
    vec3 LightColor = texture(LightMap, geomSTCoord).rgb;

    vec3 Sample = 0.5 * (0.5f * texture(ClothTex, geomTexCoord * 0.5f).rgb +
                         1.0f * texture(ClothTex, geomTexCoord * 1.0f).rgb +
                         0.5f * texture(ClothTex, geomTexCoord * 2.0f).rgb);

    vec3 Normal = normalize(
        2.0 * geomNormal +
        (Sample.r - 0.5) * geomTangent +
        (Sample.g - 0.5) * geomBitangent
    );

    vec3 LightDir = normalize(geomLightDir);
    vec3 LightRefl = reflect(-LightDir, Normal);
    float Amount = pow(Sample.b, 2.0);
    float Specular = pow(clamp(dot(LightRefl, normalize(geomViewDir)) + 0.1, 0.0, 1.0) * 0.9, 16.0 + 16.0 * Amount) * (0.2 + 2.0 * Amount);
    float Diffuse = clamp(2.0 * (dot(Normal, LightDir) - 0.5), 0.0, 1.0);
    float Ambient = 0.2;
    vec3 Color = mix(Color1, Color2, Amount);
    FragmentColor = vec4(Color * Ambient + LightColor * Color * Diffuse + LightColor * Specular, 1.0);
}