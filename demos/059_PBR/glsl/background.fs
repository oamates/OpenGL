#version 330 core

in vec3 position_ws;

uniform samplerCube environmentMap;

out vec4 FragmentColor;

void main()
{		
    vec3 environment_color = textureLod(environmentMap, position_ws, 0.0f).rgb;
    
    // HDR tonemap and gamma correct
    environment_color = environment_color / (environment_color + vec3(1.0f));
    environment_color = pow(environment_color, vec3(1.0f / 2.2f)); 
    
    FragmentColor = vec4(environment_color, 1.0f);
}
