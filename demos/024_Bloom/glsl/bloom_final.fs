#version 330 core

out vec4 FragmentColor;
in vec2 uv;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, uv).rgb;      
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    hdrColor += bloomColor;                                     // additive blending
                                                                // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));                    // also gamma correct while we're at it
    FragmentColor = vec4(result, 1.0f);
}