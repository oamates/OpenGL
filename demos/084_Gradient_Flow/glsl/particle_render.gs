#version 430 core

layout(points) in;

layout(triangle_strip) out;
layout(max_vertices = 4) out;

in vec3 position_ws_gs[];
in vec3 normal_ws_gs[];

const float size = 0.0125f;
uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

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
    p *= 8.75; 
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return canyon;
}

//==============================================================================================================================================================
// sdf gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 grad(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}


void main()
{
    vec3 p = position_ws_gs[0];
    vec3 n = normal_ws_gs[0];
    vec3 t = normalize(cross(n, vec3(1.0, 0.0, 0.0)));
    vec3 b = normalize(cross(n, t));

    vec3 positions[4] = 
    {
        p + size * t,
        p + size * b,
        p - size * b,
        p - size * t,
    };

    vec2 uvs[4] = 
    {
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2( 0.0, -1.0),
        vec2(-1.0,  0.0),
    };

    for (int i = 0; i < 4; ++i)
    {
        position_ws = positions[i];
        normal_ws = grad(position_ws);
        uv = uvs[i];                                                                   
        gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);                                             
        EmitVertex();
    }                                                                   
}                                                                       