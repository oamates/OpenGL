#version 330

in vec2 uv;  

uniform sampler2D y_channel;  
uniform sampler2D u_channel;  
uniform sampler2D v_channel;   

out vec4 FragmentColor;

// transformation matrix taken from :: https://ru.wikipedia.org/wiki/YUV

//  r = 1.000000 * y + 0.000000 * u + 1.139830 * v - 0.569915
//  g = 1.000000 * y - 0.394650 * u - 0.580600 * v + 0.487625
//  b = 1.000000 * y + 2.032110 * u + 0.000000 * v - 1.016055
//  1 = 0.000000 * y + 0.000000 * u + 0.000000 * v + 1.000000

const mat4 YUV_2_RGB = mat4(vec4( 1.000000,  1.000000,  1.000000,  0.000000),
                            vec4( 0.000000, -0.394650,  2.032110,  0.000000),
                            vec4( 1.139830, -0.580600,  0.000000,  0.000000),
                            vec4(-0.569915,  0.487625, -1.016055,  1.000000));

void main() 
{  
    vec4 yuv1 = vec4(texture(y_channel, uv).r, 
                     texture(u_channel, uv).r, 
                     texture(v_channel, uv).r, 1.0);

    FragmentColor = YUV_2_RGB * yuv1;  
	//FragmentColor = vec4(yuv1.ggg, 1);
}
