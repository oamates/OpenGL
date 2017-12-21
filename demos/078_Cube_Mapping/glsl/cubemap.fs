#version 330 core

in vec3 vertNormal;
in vec3 vertLightDir;
in vec3 vertLightRefl;
in vec3 vertViewDir;
in vec3 vertViewRefl;

uniform samplerCube TexUnit;

out vec4 FragmentColor;

void main()
{
    float l = length(vertLightDir);
    float d = dot(normalize(vertNormal), normalize(vertLightDir)) / l;
    float s = dot(normalize(vertLightRefl), normalize(vertViewDir));

    vec3 lt = vec3(1.0, 1.0, 1.0);
    vec3 env = texture(TexUnit, vertViewRefl).rgb;

    FragmentColor = vec4(env * 0.4 + (lt + env) * 1.5 * max(d, 0.0) + lt * pow(max(s, 0.0), 64), 1.0);
}
