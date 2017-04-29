#version 330 core

in vec3 position_ws;

uniform sampler2D equirectangularMap;

out vec4 FragmentColor;


const vec2 invAtan = vec2(0.1591, 0.3183);

void main()
{		
    vec3 N = normalize(position_ws);
    vec2 uv = 0.5 + invAtan * vec2(atan(N.z, N.x), asin(N.y));

    FragmentColor = texture(equirectangularMap, uv);
}
