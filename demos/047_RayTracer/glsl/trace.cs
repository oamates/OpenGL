#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

//==============================================================================================================================================================
// ray SSBO
//==============================================================================================================================================================
struct ray_t
{
    ivec4 screen;
    vec4 origin;
    vec4 direction;
};

layout (std430, binding = 0) buffer input_ray_buffer
{
    ray_t rays[];
} input_buffer;

layout (std430, binding = 1) buffer output_ray_buffer
{
    ray_t rays[];
} output_buffer;

//==============================================================================================================================================================
// sphere objects uniform buffer
//==============================================================================================================================================================
const int MAX_SPHERE_COUNT = 32;

uniform int max_depth;                          // raytrace maximal number of ray iterative splitting
uniform int sphere_count;

struct sphere_t
{
    vec3 center;                                // sphere center
    float radius;                               // sphere radius
    vec3 albedo;                                // surface albedo color
    float alpha;                                // surface transparency
    vec3 emission;                              // surface emission color
    float pad0;                                 // padding0
    float reflectivity;                         // surface reflectivity
    float ior;                                  // surface index of refraction
    float pad1;                                 // padding1
    float pad2;                                 // padding2
};


layout (std140, binding = 0) uniform geometry_buffer
{
    sphere_t spheres[MAX_SPHERE_COUNT];
};

//==============================================================================================================================================================
// atomic counters
//==============================================================================================================================================================
layout (binding = 0, offset = 0) uniform atomic_uint consume_counter;
layout (binding = 0, offset = 4) uniform atomic_uint append_counter;

//==============================================================================================================================================================
// output image
//==============================================================================================================================================================
layout (rgba32f, binding = 0) uniform image2D output_image;

/*

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

vec3 trace(vec3 origin, vec direction, int depth)
{ 
    float tnear = 1e8;                                              // 'infinity'
    int index = -1;
    
    for (int i = 0; i < sphere_count; ++i)                   // find intersection of this ray with the sphere in the scene
    { 
        float t0, t1;

        if (intersect(spheres[i], origin, direction, t0, t1))
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

    if (index == -1)                                                        // if there's no intersection return white color
        return vec3(1.0); 

    sphere_t sphere = spheres[index]; 

    vec3 color = vec3(0.0);         // color of the ray / surfaceof the object intersected by the ray 
    vec3 phit = origin + direction * tnear;               // point of intersection 
    vec3 nhit = phit - sphere->center;                    // normal at the intersection point 
    nhit = normalize(nhit);                                        // normalize normal direction 

    // If the normal and the view direction are not opposite to each other reverse the normal direction. 
    // That also means we are inside the sphere so set the inside bool to true. Finally reverse the sign of IdotN which we want to be positive.

    float bias = 1e-4;                                                 // add some bias to the point from which we will be tracing 
    bool inside = false; 

    if (dot(direction, nhit) > 0.0f)
    {
        nhit = -nhit;
        inside = true;
    }

    if ((sphere.transparency > 0.0 || sphere.reflectivity > 0.0) && depth < max_depth)
    { 
        float facing_ratio = -dot(direction, nhit); 
        float f = float(1.0) - facing_ratio;
        f = f * f * f;
        float fresnel_effect = 0.1 + 0.9 * f;          // change the mix value to tweak the effect
        vec3 refldir = direction - 2.0 * dot(direction, nhit) * nhit;      // compute reflection direction (not need to normalize because all vectors are already normalized)
        refldir = glm::normalize(refldir);
        vec3 reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
        vec3 refraction = vec3(0.0); 
        
        if (sphere.transparency > 0.0)                                 // if the sphere is also transparent compute refraction ray (transmission)
        {
            float ior = sphere.ior; 
            float eta = (inside) ? ior : 1.0 / ior;            // are we inside or outside the surface? 
            float cosi = -dot(nhit, direction); 
            float k = 1.0 - eta * eta * (1.0 - cosi * cosi); 
            vec3 refrdir = direction * eta + nhit * (eta * cosi - sqrt(k)); 
            refrdir = normalize(refrdir); 
            refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1); 
        } 
        refraction *= sphere.transparency;
        vec3 albedo_color = sphere.albedo;
        color = mix(refraction, reflection, fresnel_effect) * albedo_color;    // the result is a mix of reflection and refraction (if the sphere is transparent)
    } 
    else                                                                // it's a diffuse object, no need to raytrace any further
    { 
        for (int i = 0; i < sphere_count; ++i)                       
        {
            if (spheres[i].emission.x > 0.0)
            {
                vec3 transmission = vec3(1.0);  // this is a light
                vec3 light = spheres[i].center - phit; 
                light = normalize(light);
                for (int j = 0; j < sphere_count; ++j)
                { 
                    if (i != j)
                    {
                        float t0, t1;
                        if (spheres[j].intersect(phit + nhit * bias, light, t0, t1))
                        {
                            transmission = vec3(0.0); 
                            break; 
                        } 
                    } 
                }
                vec3 albedo_color = sphere.albedo_color;
                color += albedo_color * transmission * max(0.0, dot(nhit, light)) * spheres[i].emission; 
            } 
        } 
    }
    return color + sphere.emission_color; 
}
*/

void main()
{
    uint ray_index = atomicCounterIncrement(consume_counter);
    ray_t input_ray = input_buffer.rays[ray_index];
    barrier();

    imageStore(output_image, input_ray.screen.xy, vec4(abs(input_ray.direction.xyz), 1.0));

    ray_index = atomicCounterIncrement(append_counter);
    input_ray.direction.x += 0.01;
    output_buffer.rays[ray_index] = input_ray;

    ray_index = atomicCounterIncrement(append_counter);
    input_ray.direction.y += 0.01;
    output_buffer.rays[ray_index] = input_ray;
}

