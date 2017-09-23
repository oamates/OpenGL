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



void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (-iResolution.xy + 2.0 * fragCoord.xy) / iResolution.y;



    vec3 ro = 0.6 * vec3(2.0,-3.0, 4.0);
    vec3 ta = 0.5 * vec3(0.0, 4.0,-4.0);

    ro.x -= cos(iGlobalTime / 30.0) * 2.0;
    ta.y -= sin(iGlobalTime / 10.0) * 2.0;

    float fl = 1.0;
    vec3 ww = normalize(ta - ro);
    vec3 uu = normalize(cross(vec3(1.0, 0.0, 0.0), ww));
    vec3 vv = normalize(cross(ww,uu) );
    vec3 rd = normalize(p.x * uu + p.y * vv + fl * ww);

    float px = (2.0 / iResolution.y) * (1.0 / fl);

    vec3 col = vec3(0.0);

    //---------------------------------------------
    // raymach loop
    //---------------------------------------------
    const float maxdist = 64.0; // 64.0
    const int maxiteration = 128; //1024; // 256;

    vec3 res = vec3(-1.0);
    float t = 0.0;
  #ifdef ANTIALIASING
    vec3 oh = vec3(0.0);
    mat4 hit = mat4(-1.0,-1.0,-1.0,-1.0,
                    -1.0,-1.0,-1.0,-1.0,
                    -1.0,-1.0,-1.0,-1.0,
                    -1.0,-1.0,-1.0,-1.0);
  #endif


    for(int i = 0; i < maxiteration; i++ )
    {
        vec3 h = vec3(map(ro + t * rd), 10.7, 10.0);
        float th1 = px * t;
        res = vec3(t, h.yz);
        if(h.x < th1 || t > maxdist) break;

      #ifdef ANTIALIASING
        float th2 = px * t * 3.0;
        if((h.x < th2) && (h.x > oh.x))
        {
            float lalp = 1.0 - (h.x - th1) / (th2 - th1);
          #ifdef SLOW_ANTIALIAS
            vec3  lcol = shade( t, oh.y, oh.z, ro, rd );
            tmp.xyz += (1.0 - tmp.w) * lalp * lcol;
            tmp.w += (1.0 - tmp.w) * lalp;
            if(tmp.w > 0.99) break;
          #else
            if(hit[0].x < 0.0 )
            {
                hit[0] = hit[1];
                hit[1] = hit[2];
                hit[2] = hit[3];
                hit[3] = vec4( t, oh.yz, lalp );
            }
          #endif
        }
        oh = h;
      #endif

        t += min(h.x, 0.5) * 0.5;
    }


    //---------------------------------------------
    col = pow(col, vec3(0.3, 0.24, 0.1));
    col = col / 2.0 + col / 1.3 * vec3(pow(col.r * col.g * col.b * 1.3, 1.5));
    fragColor = vec4( col, 1.0 );
}




