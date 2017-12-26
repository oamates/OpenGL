#version 330 core

in vec3 geomNormal;
in vec3 geomTangent;
in vec3 geomBitangent;
in vec3 geomLightDir;
in vec3 geomViewDir;
in vec2 geomTexCoord;
in vec2 geomSTCoord;

uniform sampler2DArray albedo_tex;
uniform float ball_idx;

out vec4 FragmentColor;

void main()
{
    vec3 TexCoord = vec3(geomTexCoord, ball_idx);
    vec3 LightColor = vec3(1.0, 1.0, 1.0);
    vec3 Normal = normalize(geomNormal);
    vec3 LightDir = normalize(geomLightDir);
    vec3 ViewDir = normalize(geomViewDir);
    vec3 LightRefl = reflect(-LightDir, Normal);
    vec3 ViewRefl = reflect(-ViewDir, Normal);
    float Specular = pow(max(dot(LightRefl, normalize(geomViewDir)) + 0.1, 0.0), 64.0);
    float Diffuse = max(dot(Normal, LightDir) + 0.1, 0.0);

    const float Ambient = 0.2;
    vec3 Color = texture(albedo_tex, TexCoord).rgb;

    FragmentColor = vec4(Color * Ambient + LightColor * Color * Diffuse + LightColor * Specular, 1.0);
}