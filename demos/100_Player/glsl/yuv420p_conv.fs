#version 330 core

uniform sampler2D y_texture;
uniform sampler2D u_texture;
uniform sampler2D v_texture;

in vec2 uv;
out vec4 FragmentColor;

void main()
{

    const mat4 YUV_2_RGB = mat4(vec4( 1.000000f,  1.000000f,  1.000000f,  0.000000f),
                                vec4( 0.000000f, -0.394650f,  2.032110f,  0.000000f),
                                vec4( 1.139830f, -0.580600f,  0.000000f,  0.000000f),
                                vec4(-0.569915f,  0.487625f, -1.016055f,  1.000000f));

    vec4 yuv1 = vec4(texture(y_texture, uv).r, 
                     texture(u_texture, uv).r, 
                     texture(v_texture, uv).r, 1.0f);

    FragmentColor = YUV_2_RGB * yuv1;  
}