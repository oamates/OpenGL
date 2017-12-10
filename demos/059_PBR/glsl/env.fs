#version 330 core

in vec3 position_ws;

uniform samplerCube environment_map;

out vec4 FragmentColor;

void main()
{
    vec3 e = textureLod(environment_map, position_ws, 0.0f).rgb;

    //==========================================================================================================================================================
    // HDR tonemap and gamma correction
    //==========================================================================================================================================================    
    e = e / (e + vec3(1.0f));
    e = pow(e, vec3(1.0f / 2.2f));
    
    FragmentColor = vec4(e, 1.0f);
}
