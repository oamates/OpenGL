#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

uniform vec2 inv_res;                                                       // inverse screen dimensions
uniform vec2 focal_scale;                                                   // camera focal scale
uniform mat3 camera_matrix;                                                 // camera matrix
uniform vec3 camera_ws;                                                     // camera world-space position

//==============================================================================================================================================================
// ray structure
//==============================================================================================================================================================
struct ray_t
{
    vec3 ori;       /* origin */
    vec3 dir;       /* direction */
    vec3 col;       /* accumulated color */
};

//==============================================================================================================================================================
// sphere objects uniform buffer
//==============================================================================================================================================================
const int MAX_SPHERE_COUNT = 32;
const int MAX_DEPTH = 16;

uniform int depth;                              // raytrace maximal number of ray iterative splitting
uniform int sphere_count;                       // the number of spheres

struct sphere_t
{
    vec3 center;                                // sphere center
    float radius;                               // sphere radius
    vec3 albedo;                                // surface albedo color
    float transparency;                         // surface transparency
    vec3 emission;                              // surface emission color
    float energy;                               // emission energy
    float reflectivity;                         // surface reflectivity
    float ior;                                  // surface index of refraction
    float pad0;                                 // padding1
    float pad1;                                 // padding2
};


layout (std140, binding = 0) uniform geometry_buffer
{
    sphere_t spheres[MAX_SPHERE_COUNT];
};

//==============================================================================================================================================================
// output image
//==============================================================================================================================================================
layout (rgba32f, binding = 0) uniform image2D output_image;

//==============================================================================================================================================================
// intersection with sphere test
//==============================================================================================================================================================
bool intersect(int index, vec3 origin, vec3 direction, out float t0, out float t1)
{
    sphere_t sphere = spheres[index];
    vec3 l = sphere.center - origin; 
    float tca = dot(l, direction);
    if (tca < 0.0)
        return false;
    float d = sphere.radius * sphere.radius + tca * tca - dot(l, l);
    if (d < 0.0)
        return false;
    float thc = sqrt(d);
    t0 = tca - thc;
    t1 = tca + thc;
    return true; 
} 

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    vec2 uv = inv_res * (vec2(gl_GlobalInvocationID.xy) + 0.5f);                // pixel half-integral coordinates
    vec2 ndc = 2.0f * uv - 1.0f;                                                // normalized device coordinates
    vec3 z_uv = vec3(focal_scale * ndc, -1.0);
    vec3 v = normalize(camera_matrix * z_uv);

    //==========================================================================================================================================================
    // initial eye ray
    //==========================================================================================================================================================
    ray_t r;                                                                    // ray are currently working on
    r.ori = camera_ws;
    r.dir = v;
    r.col = vec3(1.0);

    //==========================================================================================================================================================
    // loop control variables
    //==========================================================================================================================================================
    int d = 0;                                                                  // current ray depth
    int jmp[MAX_DEPTH];
    jmp[0] = -1;

    ray_t ray[MAX_DEPTH];                                                       // ray storage
    vec3 col = vec3(0.0);                                                     // accumulated pixel color

    //==========================================================================================================================================================
    // main ray tracing loop
    //==========================================================================================================================================================
    while(true)
    {
        float t = 1e8;                                                          // 'infinity'
        int id = -1;
        for (int i = 0; i < sphere_count; ++i)                                  // find intersection of this ray with the sphere in the scene
        { 
            float t0, t1;
            if (intersect(i, r.ori, r.dir, t0, t1))
            { 
                if (t0 < 0)
                    t0 = t1;
                if (t0 < t)
                {
                    t = t0;
                    id = i;
                }
            }
        }

        //======================================================================================================================================================
        // if there's no intersection accumulate and jump back
        //======================================================================================================================================================
        if (id == -1)
        {
            col += r.col;
            d = jmp[d];
            if (d < 0) break;
            r = ray[d];
            continue;
        }

        sphere_t sphere = spheres[id];                                          // this is the sphere that we have intersected

        vec3 p = r.ori + t * r.dir;                                             // point of intersection 
        vec3 n = normalize(p - sphere.center);                                  // normal at the intersection point 

        const float bias = 1e-4;                                                // bias to shift the intersection point along normal

        //======================================================================================================================================================
        // If the normal and the view direction are from not opposite to each other reverse the normal direction. 
        // That also means we are inside the sphere so set the inside bool to true. Finally reverse the sign of IdotN which we want to be positive.
        //======================================================================================================================================================
        bool inside = false;
        if (dot(r.dir, n) > 0.0)
        {
            n = -n;
            inside = true;
        }

        if (d < depth)
        {
            //==================================================================================================================================================
            // if we did not reach maximal depth, split the ray into reflected and refracted parts ...
            //==================================================================================================================================================
            float facing_ratio = -dot(r.dir, n); 
            float f = float(1.0) - facing_ratio;
            f = f * f * f;
            float fresnel_effect = 0.1 + 0.9 * f;                               // change the mix value to tweak the effect
            vec3 refl = r.dir - 2.0 * dot(r.dir, n) * n;                        // reflection direction, automatically normalized

            vec3 albedo = sphere.albedo * r.col;
            vec3 refl_col = fresnel_effect * albedo;


            //==================================================================================================================================================
            // ... independently of transparency proceed to next level working with reflected ray
            //==================================================================================================================================================
            jmp[d + 2] = jmp[d + 1];                              // set back jump to the current level for now

            r.ori = p + bias * n;
            r.dir = refl;
            r.col = refl_col;

            if (sphere.transparency > 0.0)                                      
            {
                //==============================================================================================================================================
                // if the sphere is transparent, compute (transmission) refraction ray, store it on the current level and set back jump to this level
                //==============================================================================================================================================

                float ior = sphere.ior; 
                float eta = (inside) ? ior : 1.0 / ior;                         // are we inside or outside the surface? 
                float cosi = -dot(n, r.dir); 
                float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
                vec3 refr = r.dir * eta + n * (eta * cosi - sqrt(k));
                refr = normalize(refr); 

                vec3 refr_col = sphere.transparency * (1.0 - fresnel_effect) * albedo;
                ray[d + 1].ori = p - bias * n;
                ray[d + 1].dir = refr;
                ray[d + 1].col = refr_col;
                jmp[d + 2] = d + 1;
            }
            d++;
        } 
        else
        {
            //==================================================================================================================================================
            // we reached maximal allowes depth, now just collect ray from all emitting spheres surrounding the current one
            //==================================================================================================================================================
            vec3 c = vec3(0.0);

            for (int i = 0; i < sphere_count; ++i)
            {
                if (spheres[i].energy > 0.0)
                {
                    float transmission = 1.0;                              // this is light
                    vec3 light = spheres[i].center - p;
                    light = normalize(light);
                    for (int j = 0; j < sphere_count; ++j)
                    { 
                        if (i != j)
                        {
                            float t0, t1;
                            if (intersect(j, p + n * bias, light, t0, t1))
                            {
                                transmission = 0.0; 
                                break;
                            } 
                        } 
                    }
                    vec3 albedo = sphere.albedo;
                    c += sphere.albedo * transmission * max(0.0, dot(n, light)) * spheres[i].emission;
                }
            }
            col += (c + sphere.emission) * r.col;
            d = jmp[d];
            if (d < 0) break;
            r = ray[d];
            jmp[d + 1] = jmp[d];
        }
    }

    imageStore(output_image, ivec2(gl_GlobalInvocationID.xy), vec4(col, 1.0));
}

