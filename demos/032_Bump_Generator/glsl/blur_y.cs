#version 330 core

in vec2 uv;

uniform sampler2D image;

const int MAX_FILTER_WIDTH = 16;

uniform int filter_width;
uniform bool horizontal;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

out vec4 FragmentColor;

void main()
{             
     vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
     vec3 result = texture(image, uv).rgb * weight[0];
         for(int i = 1; i < 5; ++i)
         {
            result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
         }
     FragmentColor = vec4(result, 1.0);
}