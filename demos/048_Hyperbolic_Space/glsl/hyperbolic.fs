#version 430 core

out vec4 FragmentColor;

in vec3 texture_coordinates;


uniform sampler2D pentagon_texture;

const float cos2pi5 =  0.30901699437494742410; 
const float sin2pi5 =  0.95105651629515357211; 
const float cos4pi5 = -0.80901699437494742410; 
const float sin4pi5 =  0.58778525229247312917; 

const vec2 pentagon[5] = vec2[5]
(
    vec2(      1,        0),
    vec2(cos2pi5,  sin2pi5),
    vec2(cos4pi5,  sin4pi5),
    vec2(cos4pi5, -sin4pi5),
    vec2(cos2pi5, -sin2pi5)
);

void main( void )
{
    float x = texture_coordinates.x;
    float y = texture_coordinates.y;
    float z = texture_coordinates.z;

    vec2 plane_coord0 = x * (vec2(0.5f) + 0.5f * pentagon[0]) + y * (vec2(0.5f) + 0.5f * pentagon[1]) + z * vec2(0.5f);
    vec2 plane_coord1 = x * (vec2(0.5f) + 0.5f * pentagon[1]) + y * (vec2(0.5f) + 0.5f * pentagon[2]) + z * vec2(0.5f);
    vec2 plane_coord2 = x * (vec2(0.5f) + 0.5f * pentagon[2]) + y * (vec2(0.5f) + 0.5f * pentagon[3]) + z * vec2(0.5f);
    vec2 plane_coord3 = x * (vec2(0.5f) + 0.5f * pentagon[3]) + y * (vec2(0.5f) + 0.5f * pentagon[4]) + z * vec2(0.5f);
    vec2 plane_coord4 = x * (vec2(0.5f) + 0.5f * pentagon[4]) + y * (vec2(0.5f) + 0.5f * pentagon[0]) + z * vec2(0.5f);


    vec4 texel = (texture2D(pentagon_texture, plane_coord0) + 
                  texture2D(pentagon_texture, plane_coord1) + 
                  texture2D(pentagon_texture, plane_coord2) + 
                  texture2D(pentagon_texture, plane_coord3) + 
                  texture2D(pentagon_texture, plane_coord4)) / 5.0;

    texel = 3.0f * texel * texel; 
    
    if (z*z > 0.05) discard;
    texel.w =  0.5f;
    FragmentColor = texel;
}












