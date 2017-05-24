#version 330 core

in vec2 uv;
in vec3 view;

const int kernel_size = 64;

uniform sampler2DShadow depth_tex;
uniform sampler2D normal_ws_tex;

uniform mat4 projection_view_matrix;
uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec4 samples[kernel_size];
uniform float radius;
uniform float bias;

layout (location = 0) out vec2 FragmentOcclusionR;

void main()
{
    //==========================================================================================================================================================
    // Get camera-space normal and Z
    //==========================================================================================================================================================
    vec4 data_in = texture(normal_ws_tex, uv);
    vec3 N_ws = normalize(data_in.xyz);
    float R = data_in.w;                                        // distance from fragment to camera 
    vec3 v = normalize(view);
    vec3 position_cs = R * normalize(v);
    vec3 position_ws = camera_ws + camera_matrix * position_cs;
    
    //==========================================================================================================================================================
    // Iterate over the sample kernel and calculate occlusion factor
    //==========================================================================================================================================================
    float occlusion = 0.0;
    float total = 0.0f;

    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 sample = samples[i].xyz;
        float dp = dot(sample, N_ws);

        if (dp > 0.0f)
        {
            float radius = samples[i].w;
            //======================================================================================================================================================
            // get sample position in camera space and in ndc
            //======================================================================================================================================================
            vec3 sample_ws = position_ws + radius * sample;
            vec4 sample_ss = projection_view_matrix * vec4(sample_ws, 1.0f);
            vec3 ndc = sample_ss.xyz / sample_ss.w;
            vec3 uvw = 0.5f + 0.5f * ndc;

            uvw.z += 0.0016 / R;

            //======================================================================================================================================================
            // get sample distance, range check & accumulate
            //======================================================================================================================================================
            occlusion += dp * texture(depth_tex, uvw);
            total += dp;
        }
    }
    
    FragmentOcclusionR = vec2(occlusion / total, R);
}                                            