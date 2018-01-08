#version 330 core

in vec3 ray;

uniform samplerCube environment_map;
uniform float level;

out vec4 FragmentColor;

void main()
{
    vec3 e = textureLod(environment_map, ray, level).rgb;

    //==========================================================================================================================================================
    // HDR tonemap and gamma correction
    //==========================================================================================================================================================    
    e = e / (e + vec3(1.0f));
    e = pow(e, vec3(1.0f / 2.2f));
    
    FragmentColor = vec4(e, 1.0f);
}
