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
    vec3 origin;
    vec3 direction;
    vec3 accumulated_color;
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

vec3 trace(vec3 origin, vec3 direction, int depth)
{ 


}

void main()
{


struct ray_t
{
    ivec4 screen;
    vec4 origin;
    vec4 direction;
};

layout (std430, binding = 0) buffer ray_buffer
{
    ray_t rays[];
};

void main()
{
    vec2 uv = inv_res * (vec2(gl_GlobalInvocationID.xy) + 0.5f);                // pixel half-integral coordinates
    vec2 ndc = 2.0f * uv - 1.0f;                                                // normalized device coordinates
    vec3 z_uv = vec3(focal_scale * ndc, -1.0);
    vec3 v = normalize(camera_matrix * z_uv);

    ray_t ray[MAX_DEPTH];

    ray[0].origin = camera_ws;
    ray[0].direction = v;
    ray[0].accumulated_color = vec3(1.0);

    int d = 0;                                                                  // current ray depth
    vec3 color = vec3(0.0);

    const float bias = 1e-4;                                                      // add some bias to the point from which we will be tracing 


    while(d >= 0)
    {

        float tnear = 1e8;                                                      // 'infinity'
        int index = -1;
        for (int i = 0; i < sphere_count; ++i)                                  // find intersection of this ray with the sphere in the scene
        { 
            float t0, t1;
            if (intersect(i, origin, direction, t0, t1))
            { 
                if (t0 < 0)
                    t0 = t1;
                if (t0 < tnear)
                {
                    tnear = t0; 
                    index = i;
                }
            }
        }

        //======================================================================================================================================================
        // if there's no intersection accumulate and go one level down
        //======================================================================================================================================================
        if (index == -1)                                                        // 
        {
            d--;
            color += r[d].accumulated_color;
            continue;
        }                        


        sphere_t sphere = spheres[index];                                       // this is the sphere that we have intersected

        //vec3 color = vec3(0.0);                                                 // color of the ray / surface of the object intersected by the ray 
        vec3 phit = origin + direction * tnear;                                 // point of intersection 
        vec3 nhit = phit - sphere.center;                                      // normal at the intersection point 
        nhit = normalize(nhit);                                                 // normalize normal direction 

        // If the normal and the view direction are not opposite to each other reverse the normal direction. 
        // That also means we are inside the sphere so set the inside bool to true. Finally reverse the sign of IdotN which we want to be positive.

        bool inside = false;

        if (dot(direction, nhit) > 0.0f)
        {
            nhit = -nhit;
            inside = true;
        }

        if (d < depth)
        {
            //==================================================================================================================================================
            // if we did not reach maximal depth, split the ray into reflected and refracted parts, store one of them on this level,
            // write the second into next and proceed
            //==================================================================================================================================================
            float facing_ratio = -dot(direction, nhit); 
            float f = float(1.0) - facing_ratio;
            f = f * f * f;
            float fresnel_effect = 0.1 + 0.9 * f;                               // change the mix value to tweak the effect
            vec3 refldir = direction - 2.0 * dot(direction, nhit) * nhit;       // compute reflection direction (not need to normalize because all vectors are already normalized)
            refldir = normalize(refldir);

            vec3 acc_albedo = sphere.albedo * r[d].accumulated_color;
            vec3 reflected_acc_color = fresnel_effect * acc_albedo;
                // reflected ray to trace is {phit + nhit * bias, refldir, depth + 1}

            /*
                ray[d].origin = phit + bias * nhit;
                ray[d].direction = refldir;
                ray[d].accumulated_color = reflected_acc_color;
            */
        
            if (sphere.transparency > 0.0)                                      // if the sphere is also transparent compute refraction ray (transmission)
            {
                float ior = sphere.ior; 
                float eta = (inside) ? ior : 1.0 / ior;                         // are we inside or outside the surface? 
                float cosi = -dot(nhit, direction); 
                float k = 1.0 - eta * eta * (1.0 - cosi * cosi); 
                vec3 refrdir = direction * eta + nhit * (eta * cosi - sqrt(k)); 
                refrdir = normalize(refrdir); 

                vec3 refracted_acc_color = sphere.transparency * (1.0 - fresnel_effect) * acc_albedo; 
                    // refracted ray to trace is {phit - nhit * bias, refrdir, depth + 1}

                /*
                    ray[d].origin = phit - bias * nhit;
                    ray[d].direction = refrdir;
                    ray[d].accumulated_color = refracted_acc_color;
                */

            } 







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
                    vec3 light = spheres[i].center - phit;
                    light = normalize(light);
                    for (int j = 0; j < sphere_count; ++j)
                    { 
                        if (i != j)
                        {
                            float t0, t1;
                            if (intersect(j, phit + nhit * bias, light, t0, t1))
                            {
                                transmission = 0.0; 
                                break;
                            } 
                        } 
                    }
                    vec3 albedo = sphere.albedo;
                    c += sphere.albedo * transmission * max(0.0, dot(nhit, light)) * spheres[i].emission;
                }
            }
            color += (c + sphere.emission) * r[d].accumulated_color;
            d--;
        }
    }

    imageStore(output_image, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0));
}

