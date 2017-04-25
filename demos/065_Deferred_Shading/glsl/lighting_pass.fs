#version 420 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D albedo_tex;

struct light_t
{
    vec4 position;
    vec4 color;    
};

const int NR_LIGHTS = 32;

layout (std140) uniform lights_block
{
    light_t light[NR_LIGHTS];
};


uniform vec3 camera_ws;
uniform int draw_mode;

out vec4 FragmentColor;


void main()
{             
    //==========================================================================================================================================================
    // Retrieve data from gbuffer
    //==========================================================================================================================================================
    vec3 position_ws = texture(position_tex, uv).rgb;
    vec3 normal_ws = texture(normal_tex, uv).rgb;
    vec4 ds = texture(albedo_tex, uv);
    vec3 diffuse = ds.rgb;
    float specular_power = ds.a;
    
    //==========================================================================================================================================================
    // Then calculate lighting as usual, hard-coded ambient component
    //==========================================================================================================================================================
    vec3 color = diffuse * 0.125;
    vec3 v = normalize(camera_ws - position_ws);

    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        //======================================================================================================================================================
        // Calculate distance between light source and current fragment
        //======================================================================================================================================================
        vec3 light_ws = light[i].position.xyz;
        vec3 l = light_ws - position_ws;
        float distance = length(l);
        float radius = light[i].position.w;

        if(distance < radius)
        {
            vec3 light_color = light[i].color.rgb;
            float light_spec_power = light[i].color.a;
            //==================================================================================================================================================
            // Diffuse
            //==================================================================================================================================================
            l /= distance;
            vec3 diffuse_color = max(dot(normal_ws, l), 0.0) * diffuse * light_color;

            //==================================================================================================================================================
            // Specular
            //==================================================================================================================================================
            vec3 h = normalize(l + v);  
            float specular_factor = pow(max(dot(normal_ws, h), 0.0), 80.0 * specular_power);
            vec3 specular_color = light_color * specular_factor * light_spec_power;

            //==================================================================================================================================================
            // Attenuation
            //==================================================================================================================================================
            float attenuation = 1.0f / (1.0f + 0.5 * distance * distance);
            diffuse_color *= attenuation;
            specular_color *= attenuation;
            color += diffuse_color;
            color += specular_color;
        }
    }    
    
    //==========================================================================================================================================================
    // Based on the draw mode, show final result or intermediate textures
    //==========================================================================================================================================================

    if(draw_mode == 0)
        FragmentColor = vec4(color, 1.0);
    else if(draw_mode == 1)
        FragmentColor = vec4(0.15 * abs(position_ws), 1.0);
    else if(draw_mode == 2)
        FragmentColor = vec4(abs(normal_ws), 1.0);
    else if(draw_mode == 3)
        FragmentColor = vec4(diffuse, 1.0);
    else if(draw_mode == 4)
        FragmentColor = vec4(vec3(specular_power), 1.0);
}
