#version 330 core

uniform ivec2 origin;
uniform float grass_scale;

out vec3 position_ws;
out vec3 axis_x;
out vec3 axis_z;
out float height;

//==============================================================================================================================================================
// hashing functions
//==============================================================================================================================================================
vec3 hash3(ivec2 p)
{
    vec3 h = p.x * vec3(127.19f, -12.157f, 91.417f) + p.y * vec3(-513.29f, 77.441f, 13.491f);
    return fract(cos(h) * 43134.717f);
}

vec2 hash2(ivec2 p)
{
    vec2 h = p.x * vec2(-12.571f, 41.547f) + p.y * vec2(-51.913f, 61.419f);
    return fract(cos(h) * 43134.717f);
}

float hash(ivec2 p)
{
    float h = p.x * 47.547f - p.y * 11.419f;
    return fract(cos(h) * 43134.717f);
}

//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
vec3 tri(in vec3 x)
{
    vec3 q = abs(fract(x) - 0.5f);
    return q;
}

float sdf(vec3 p)
{    
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z + 3.5 + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

float ground_sdf(vec3 p)
{    
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z + 3.5 + dot(op, vec3(0.067));
    return ground;
}

//==============================================================================================================================================================
// distance function gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 calc_normal(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}


void main()
{
    ivec2 id = ivec2(gl_VertexID, gl_InstanceID);

    ivec2 h = origin + id;
    vec2 base = grass_scale * vec2(h) + hash2(h);

    position_ws = vec3(base, -3.5);
    for(int i = 0; i < 4; ++i)
    {
        float t = ground_sdf(position_ws);
        position_ws.z -= t;
    }
    
    height = 0.5 + 0.5 * hash(h);
    axis_x = hash3(h);
    axis_z = calc_normal(position_ws);
    axis_z = normalize(axis_z + 0.5f * axis_x);
    axis_x = normalize(axis_x - dot(axis_x, axis_z) * axis_z);
}


