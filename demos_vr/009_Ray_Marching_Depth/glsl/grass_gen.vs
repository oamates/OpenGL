#version 330 core

uniform ivec2 origin;
uniform float grid_scale;

out vec3 position_ws;
out vec3 axis_x;
out vec3 axis_z;
out vec3 hash;
out float scale;

//==============================================================================================================================================================
// hashing functions
//==============================================================================================================================================================

const vec3 hash_x[3] = vec3[3]
(
    vec3( 17.193f,  12.439f,  21.741f),
    vec3(-27.881f, -11.127f,  93.237f),
    vec3( 16.649f, -74.671f, -10.351f)
);

const vec3 hash_y[3] = vec3[3]
(
    vec3(-71.219f,  74.511f,  17.491f),
    vec3( 13.829f, -77.467f, -27.913f),
    vec3(-51.293f, -18.341f, -13.957f)
);

vec3 hash_0(ivec2 p)
{
    vec3 h = p.x * hash_x[0] + p.y * hash_y[0];
    return fract(cos(h) * 43134.717f);
}

vec3 hash_1(ivec2 p)
{
    vec3 h = p.x * hash_x[1] + p.y * hash_y[1];
    return fract(cos(h) * 43134.717f);
}

vec3 hash_2(ivec2 p)
{
    vec3 h = p.x * hash_x[2] + p.y * hash_y[2];
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
    ivec2 grid_point = origin + id;

    vec3 h0 = hash_0(grid_point);
    vec3 h1 = hash_1(grid_point);
    vec3 h2 = hash_2(grid_point);

    position_ws = vec3(grid_scale * vec2(grid_point) + h0.xy, -3.5);

    for(int i = 0; i < 4; ++i)
    {
        float t = ground_sdf(position_ws);
        position_ws.z -= t;
    }
    
    scale = 0.3 + 0.7 * h0.z;
    axis_x = h1;
    axis_z = calc_normal(position_ws);
    axis_z = normalize(axis_z + 0.25f * axis_x);
    axis_x = normalize(axis_x - dot(axis_x, axis_z) * axis_z);
    hash = h2;

}


