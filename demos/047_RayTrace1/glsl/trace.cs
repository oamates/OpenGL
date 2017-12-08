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
layout (rgba8, binding = 0) uniform image2D output_image;

//==============================================================================================================================================================
// intersection with sphere test
//==============================================================================================================================================================
bool intersect(int index, vec3 ori, vec3 dir, out float t0, out float t1)
{
    sphere_t sphere = spheres[index];
    vec3 l = sphere.center - ori; 
    float dp = dot(l, dir);
    if (dp < 0.0)
        return false;
    float d = sphere.radius * sphere.radius + dp * dp - dot(l, l);
    if (d < 0.0)
        return false;
    float qq = sqrt(d);
    t0 = dp - qq;
    t1 = dp + qq;
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
    ray_t ray;                                                                    // ray are currently working on
    ray.ori = camera_ws;
    ray.dir = v;
    ray.col = vec3(1.0);

    //==========================================================================================================================================================
    // loop control variables
    //==========================================================================================================================================================
    int d = 0;                                                                  // current ray depth
    int jmp[MAX_DEPTH];
    jmp[0] = -1;

    ray_t rays[MAX_DEPTH];                                                       // ray storage
    vec3 col = vec3(0.0);                                                     // accumulated pixel color

    //==========================================================================================================================================================
    // main ray tracing loop
    //==========================================================================================================================================================
    while(true)
    {
        float ti = 1e8;                                                         // 'infinity'
        int id = -1;
        for (int i = 0; i < sphere_count; ++i)                                  // find intersection of this ray with the sphere in the scene
        { 
            float t0, t1;
            if (intersect(i, ray.ori, ray.dir, t0, t1))
            { 
                if (t0 < 0)
                    t0 = t1;
                if (t0 < ti)
                {
                    ti = t0;
                    id = i;
                }
            }
        }

        //======================================================================================================================================================
        // if there's no intersection accumulate and jump back
        //======================================================================================================================================================
        if (id == -1)
        {
            col += ray.col;
            d = jmp[d];
            if (d < 0) break;
            ray = rays[d];
            jmp[d + 1] = jmp[d];
            d++;
            continue;
        }

        sphere_t sphere = spheres[id];                                          // this is the sphere that we have intersected

        const float bias = 1e-4;                                                // bias to shift the intersection point along normal

        if (d < depth)
        {
            //==================================================================================================================================================
            // if we did not reach maximal depth, split the ray into reflected and refracted parts ...
            //==================================================================================================================================================

            vec3 p = ray.ori + ti * ray.dir;                                    // point of intersection 
            vec3 n = normalize(p - sphere.center);                              // normal at the intersection point 

            const float bias = 1e-2;                                            // bias to shift the intersection point along normal
            vec3 i = ray.dir;                                                   // incident ray emanating from incidence point
            float cosi = -dot(i, n);

            //==================================================================================================================================================
            // If the normal and the incident ray make acute angle revert the normal
            // That also means we are inside the sphere so set the inside bool to true. Finally reverse the sign of IdotN which we want to be positive.
            //==================================================================================================================================================

            float ior = sphere.ior;

            if (cosi < 0.0)                                                     // are we inside or outside the surface?
            {
                n = -n;
                ior = 1.0 / ior;
                cosi = -cosi;
            }

            vec3 r = i + (2.0 * cosi) * n;                                      // reflection direction, automatically normalized
            float k = 1.0 - ior * ior * (1.0 - cosi * cosi);                    // the square of cosine of refraction angle, must be >= 0
            float R = 1.0;                                                      // reflected energy

            jmp[d + 1] = jmp[d];                                                // set back jump to the current back jump for now
            vec3 albedo = sphere.albedo * ray.col;

            //==============================================================================================================================================
            // compute (transmission) refraction ray, store it on the current level and set back jump to this level
            //==============================================================================================================================================
            if (k >= 0.0)
            {
                float cost = pow(k, 0.8);
                vec3 t = ior * i + (ior * cosi - cost) * n;                     // refracted direction, automatically normalized
                float q1 = (ior * cosi - cost) / (ior * cosi + cost);
                float q2 = (cosi - ior * cost) / (cosi + ior * cost);
                R = 0.5 * (q1 * q1 + q2 * q2);                                  // Fresnel equations
                rays[d].ori = p - bias * n;
                rays[d].dir = t;
                rays[d].col = (1.0 - R) * albedo;

                jmp[d + 1] = d;                                                 // must return back and process this ray also
            }

            //==================================================================================================================================================
            // ... independently of transparency proceed to next level working with reflected ray
            //==================================================================================================================================================
            ray.ori = p + bias * n;
            ray.dir = r;
            ray.col = R * albedo;
            d++;
        } 
        else
        {
            //==================================================================================================================================================
            // we reached maximal allowes depth, now just collect ray from all emitting spheres surrounding the current one
            //==================================================================================================================================================
            /*
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
            */
            col += (/*c*/ vec3(1.0) + sphere.emission) * ray.col;

            d = jmp[d];
            if (d < 0) break;
            ray = rays[d];
            jmp[d + 1] = jmp[d];
            d++;
            continue;
        }
    }

    imageStore(output_image, ivec2(gl_GlobalInvocationID.xy), vec4(col, 1.0));
}
