#version 330 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D albedo_tex;

struct Light
{
    vec3 position;
    vec3 color;    
    float linear;
    float quadratic;
    float radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
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
    vec3 view  = normalize(camera_ws - position_ws);

    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        //======================================================================================================================================================
        // Calculate distance between light source and current fragment
        //======================================================================================================================================================
        vec3 light = lights[i].position - position_ws;
        float distance = length(light);

        if(distance < lights[i].radius)
        {
            //==================================================================================================================================================
            // Diffuse
            //==================================================================================================================================================
            vec3 l = light / distance;
            vec3 diffuse_color = max(dot(normal_ws, l), 0.0) * diffuse * lights[i].color;

            //==================================================================================================================================================
            // Specular
            //==================================================================================================================================================
            vec3 h = normalize(l + v);  
            float specular_factor = pow(max(dot(normal_ws, h), 0.0), 16.0);
            vec3 specular_color = lights[i].color * specular_factor * specular_power;

            //==================================================================================================================================================
            // Attenuation
            //==================================================================================================================================================
            float attenuation = 1.0f / (1.0f + lights[i].linear * distance + lights[i].quadratic * distance * distance);
            diffuse_color *= attenuation;
            specular_color *= attenuation;
            color += diffuse_color;
            color += specular_color;
        }
    }    
    
    //==========================================================================================================================================================
    // Based on which of the 1-5 keys we pressed, show final result or intermediate g-buffer textures
    //==========================================================================================================================================================
    if(draw_mode == 0)
        FragColor = vec4(color, 1.0);
    else if(draw_mode == 1)
        FragColor = vec4(abs(position_ws), 1.0);
    else if(draw_mode == 2)
        FragColor = vec4(abs(normal_ws), 1.0);
    else if(draw_mode == 3)
        FragColor = vec4(diffuse, 1.0);
    else if(draw_mode == 4)
        FragColor = vec4(vec3(specular_power), 1.0);
}
