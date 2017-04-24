#version 330 core

in vec2 uv;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

uniform vec3 light_cs;
uniform vec3 light_color;
uniform int draw_mode;

out vec4 FragmentColor;

void main()
{             
    // Retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, uv).rgb;
    vec3 Normal = texture(gNormal, uv).rgb;
    vec3 Diffuse = texture(gAlbedo, uv).rgb;
    float AmbientOcclusion = texture(ssao, uv).r;
    
    // Then calculate lighting as usual
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // Viewpos is (0.0.0)
    // Diffuse
    vec3 lightDir = normalize(light_cs - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light_color;
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light_color * spec;
    // Attenuation
    float distance = length(light_cs - FragPos);
    float attenuation = 1.0 / (1.0f + 0.09f * distance + 0.032f * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    if(draw_mode == 1)
        FragmentColor = vec4(lighting, 1.0);
    else if(draw_mode == 2)
        FragmentColor = vec4(FragPos, 1.0);
     else if(draw_mode == 3)
        FragmentColor = vec4(Normal, 1.0);
    else if(draw_mode == 4)
        FragmentColor = vec4(vec3(AmbientOcclusion), 1.0);
}
