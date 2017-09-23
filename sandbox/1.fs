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
    const float ground_level = -1.0f;
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z - ground_level + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

//==============================================================================================================================================================
// basic raymarching loop
//==============================================================================================================================================================

float trace(vec3 p, vec3 v, float t0)
{    
    float t = t0;
    for(int i = 0; i < MAX_TRACE_ITER; i++)
    {    
        float d = sdf(p + t * v);
        float d_abs = abs(d);
        if(d_abs < (0.001 + 0.00025 * t) || t > HORIZON) break;
        t += d;
    }
    return min(t, HORIZON);
}

const float pixel_scale = 1.0 / 1920.0;

vec3 shade(vec3 p)
{
    return vec3(0.0f);
}

float trace(vec3 p, vec3 v, float t_min)
{
    vec3 color = vec3(0.0);                 // color accumulated so far

    vec3 q0 = p;
    float t = t_min;

    float min_v = z_near;

    while()
    {
        vec3 q1 = q0 + t * v;

        float value = sdf(q1);

        if(value < pixel_scale * t)
        {
            //==================================================================================================================================================
            // we are coming very close to the surface
            //==================================================================================================================================================



        }


    }
}





