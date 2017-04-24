#version 400 core

layout(location = 0) in vec2 uv;  

uniform float time;

out vec2 v_texCoord2D;
out vec3 v_texCoord3D;
out vec4 v_texCoord4D;

const vec2 aspect = vec2(16.0, 9.0);

void main()
{
    gl_Position = vec4(uv, 0.0f, 1.0f);
    v_texCoord2D = aspect * uv + vec2(0.0, 2.5 * time);

    v_texCoord3D = vec3(aspect * uv, 1.9 * time);
    
    v_texCoord4D = vec4(aspect * uv, 0.0, time);
}


