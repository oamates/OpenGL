#include <cstdlib> 
#include <cstdio> 
#include <thread>
#include <vector> 

#include <glm/glm.hpp> 

#include "constants.hpp" 

typedef double real_t;
 
const int res_x = 1920 * 2;
const int res_y = 1080 * 2;
const int MAX_RAY_DEPTH = 16;

using vec3 = glm::tvec3<real_t>;

template<typename real_t> struct sphere_t
{
    glm::tvec3<real_t> center;                                          // position of the sphere 
    real_t radius, radius_sqr;                                          // sphere radius and radius squared
    glm::tvec3<real_t> albedo_color, emission_color;                    // surface albedo and emission (light) colors
    real_t transparency, reflectivity;                                  // surface transparency and reflectivity
    real_t ior;                                                         // surface index of refraction

    sphere_t(const glm::tvec3<real_t>& center, real_t radius,
             const glm::tvec3<real_t>& albedo_color, const glm::tvec3<real_t>& emission_color, 
             real_t reflectivity, real_t transparency, real_t ior)
        : center(center), radius(radius), 
          albedo_color(albedo_color), emission_color(emission_color),
          transparency(transparency), reflectivity(reflectivity) 
        { radius_sqr = radius * radius; }

    bool intersect(const glm::tvec3<real_t>& origin, const glm::tvec3<real_t>& direction, real_t& t0, real_t& t1) const 
    { 
        glm::tvec3<real_t> l = center - origin; 
        real_t tca = glm::dot(l, direction);
        if (tca < real_t(0.0))
            return false;
        real_t d2 = glm::dot(l, l) - tca * tca;
        if (d2 > radius_sqr)
            return false;
        real_t thc = glm::sqrt(radius_sqr - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true; 
    } 
};

template<typename real_t> glm::tvec3<real_t> trace(const glm::tvec3<real_t>& origin, const glm::tvec3<real_t>& direction, 
        const std::vector<sphere_t<real_t>>& spheres, const int &depth) 
{ 
    const real_t INF = 1e8;
    real_t tnear = INF; 
    const sphere_t<real_t>* sphere = 0;
    
    for (unsigned int i = 0; i < spheres.size(); ++i)                   // find intersection of this ray with the sphere in the scene
    { 
        real_t t0 = INF;
        real_t t1 = INF;
        if (spheres[i].intersect(origin, direction, t0, t1))
        { 
            if (t0 < 0)
                t0 = t1;
            if (t0 < tnear)
            { 
                tnear = t0; 
                sphere = &spheres[i]; 
            } 
        } 
    }

    if (!sphere)                                                        // if there's no intersection return white color
        return glm::tvec3<real_t>(real_t(1.0)); 

    glm::tvec3<real_t> color = glm::tvec3<real_t>(real_t(0.0));         // color of the ray / surfaceof the object intersected by the ray 
    glm::tvec3<real_t> phit = origin + direction * tnear;               // point of intersection 
    glm::tvec3<real_t> nhit = phit - sphere->center;                    // normal at the intersection point 
    nhit = glm::normalize(nhit);                                        // normalize normal direction 

    // If the normal and the view direction are not opposite to each other reverse the normal direction. 
    // That also means we are inside the sphere so set the inside bool to true. Finally reverse the sign of IdotN which we want to be positive.

    real_t bias = 1e-4;                                                 // add some bias to the point from which we will be tracing 
    bool inside = false; 

    if (glm::dot(direction, nhit) > 0.0f)
    {
        nhit = -nhit;
        inside = true;
    }

    if ((sphere->transparency > 0.0 || sphere->reflectivity > 0.0) && depth < MAX_RAY_DEPTH)
    { 
        real_t facing_ratio = -glm::dot(direction, nhit); 
        real_t f = real_t(1.0) - facing_ratio;
        f = f * f * f;
        real_t fresnel_effect = real_t(0.1) + real_t(0.9) * f;          // change the mix value to tweak the effect
        glm::tvec3<real_t> refldir = direction - real_t(2.0) * glm::dot(direction, nhit) * nhit;      // compute reflection direction (not need to normalize because all vectors are already normalized)
        refldir = glm::normalize(refldir);
        glm::tvec3<real_t> reflection = trace<real_t>(phit + nhit * bias, refldir, spheres, depth + 1); 
        glm::tvec3<real_t> refraction = glm::tvec3<real_t>(real_t(0.0)); 
        
        if (sphere->transparency > 0.0)                                 // if the sphere is also transparent compute refraction ray (transmission)
        {
            real_t ior = sphere->ior; 
            real_t eta = (inside) ? ior : real_t(1.0) / ior;            // are we inside or outside the surface? 
            real_t cosi = -glm::dot(nhit, direction); 
            real_t k = real_t(1.0) - eta * eta * (real_t(1.0) - cosi * cosi); 
            glm::tvec3<real_t> refrdir = direction * eta + nhit * (eta * cosi - glm::sqrt(k)); 
            refrdir = glm::normalize(refrdir); 
            refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1); 
        } 
        refraction *= sphere->transparency;
        glm::tvec3<real_t> albedo_color = sphere->albedo_color;
        color = glm::mix(refraction, reflection, fresnel_effect) * albedo_color;    // the result is a mix of reflection and refraction (if the sphere is transparent)
    } 
    else                                                                // it's a diffuse object, no need to raytrace any further
    { 
        for (unsigned int i = 0; i < spheres.size(); ++i)                       
        {
            if (spheres[i].emission_color.x > real_t(0.0))
            {
                glm::tvec3<real_t> transmission = glm::tvec3<real_t>(real_t(1.0));  // this is a light
                glm::tvec3<real_t> light = spheres[i].center - phit; 
                light = glm::normalize(light);
                for (unsigned int j = 0; j < spheres.size(); ++j)
                { 
                    if (i != j)
                    {
                        real_t t0, t1;
                        if (spheres[j].intersect(phit + nhit * bias, light, t0, t1))
                        {
                            transmission = glm::tvec3<real_t>(real_t(0.0)); 
                            break; 
                        } 
                    } 
                }
                glm::tvec3<real_t> albedo_color = sphere->albedo_color;
                color += albedo_color * transmission * glm::max(real_t(0.0), glm::dot(nhit, light)) * spheres[i].emission_color; 
            } 
        } 
    }
    return color + sphere->emission_color; 
}

template<typename real_t> struct ray_trace_data_t
{
    std::vector<sphere_t<real_t>> spheres;

    int res_x;
    int res_y;

    real_t inv_res_x;
    real_t inv_res_y;

    real_t focal_x;
    real_t focal_y;

    ray_trace_data_t(int res_x, int res_y, real_t fov)
        : res_x(res_x), res_y(res_y)
    {
        inv_res_x = real_t(1.0) / real_t(res_x);
        inv_res_y = real_t(1.0) / real_t(res_y);
        real_t aspect = real_t(res_x) / real_t(res_y); 
        real_t tan = glm::tan(real_t(0.5) * fov);
        focal_x = tan * aspect;
        focal_y = tan;
    }
};

template<typename real_t> void ray_trace_thread(const ray_trace_data_t<real_t>* ray_trace_data, uint8_t* rgb8_image, int y_min, int y_max)
{
    int res_x = ray_trace_data->res_x;

    real_t inv_res_x = ray_trace_data->inv_res_x;
    real_t inv_res_y = ray_trace_data->inv_res_y;
    real_t focal_x = ray_trace_data->focal_x;
    real_t focal_y = ray_trace_data->focal_y;

    int index = 3 * y_min * ray_trace_data->res_x;
    for (int y = y_min; y < y_max; ++y)
    { 
        for (int x = 0; x < res_x; ++x)
        {
            real_t xx = (real_t(2.0) * ((x + real_t(0.5)) * inv_res_x) - real_t(1.0)) * focal_x; 
            real_t yy = (real_t(1.0) - real_t(2.0) * ((y + real_t(0.5)) * inv_res_y)) * focal_y; 
            glm::tvec3<real_t> direction = glm::tvec3<real_t>(xx, yy, real_t(-1.0)); 
            direction = glm::normalize(direction);
            glm::tvec3<real_t> color = trace<real_t>(glm::tvec3<real_t>(real_t(0.0)), direction, ray_trace_data->spheres, 0);
            color = real_t(255.0) * glm::clamp(color, real_t(0.0), real_t(1.0));

            rgb8_image[index++] = (uint8_t) color.r;
            rgb8_image[index++] = (uint8_t) color.g;
            rgb8_image[index++] = (uint8_t) color.b; 
        }
    }
}
 
int main(int argc, char* argv[])
{ 
    ray_trace_data_t<real_t> ray_trace_data(res_x, res_y, constants::two_pi_d / 6.0);

    std::vector<sphere_t<real_t>>& spheres = ray_trace_data.spheres;

    /*                                                      center   radius            albedo color          emission color  transparency  reflectivity   ior */
    spheres.push_back(sphere_t<real_t>(vec3( 0.0, -10004.0, -20.0), 10000.0, vec3(0.80, 0.20, 0.20), vec3(0.01, 0.20, 0.04),          0.1,         0.22, 1.11));

    spheres.push_back(sphere_t<real_t>(vec3( 0.0,      0.0, -20.0),     4.0, vec3(1.00, 1.32, 0.36), vec3(0.07, 0.12, 0.11),          0.3,         0.57, 1.17)); 
    spheres.push_back(sphere_t<real_t>(vec3( 5.0,     -1.0, -15.0),     2.0, vec3(0.90, 0.76, 0.46), vec3(0.05, 0.06, 0.09),          0.5,         0.11, 1.31)); 
    spheres.push_back(sphere_t<real_t>(vec3( 5.0,      0.0, -25.0),     3.0, vec3(0.65, 0.77, 0.97), vec3(0.03, 0.11, 0.17),          0.7,         0.13, 1.15)); 
    spheres.push_back(sphere_t<real_t>(vec3(-5.5,      0.0, -15.0),     3.0, vec3(0.90, 0.90, 0.90), vec3(0.21, 0.10, 0.06),          0.9,         0.07, 1.07));
    spheres.push_back(sphere_t<real_t>(vec3( 0.0,     20.0, -30.0),     3.0, vec3(0.00, 0.00, 0.00), vec3(3.00, 2.00, 3.00),          1.0,         0.21, 1.19));

    const int size = 3 * res_x * res_y * sizeof(uint8_t);
    uint8_t* rgb8_image = (uint8_t*) malloc (size);

    const int threads = 7;
    std::thread computation_thread[threads - 1];

    int y_div = res_y / threads;
    int y_rem = res_y % threads;
    int y_min = 0;

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        int y_max = y_min + y_div + int(thread_id < y_rem);
        computation_thread[thread_id] = std::thread(ray_trace_thread<real_t>, &ray_trace_data, rgb8_image, y_min, y_max);
        y_min = y_max;
    }

    ray_trace_thread<real_t>(&ray_trace_data, rgb8_image, y_min, res_y);

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        computation_thread[thread_id].join();


    FILE* f = fopen("raytrace.ppm", "wb");

    char ppm_header[128];
    int bytes_written = sprintf(ppm_header, "P6\n%d %d\n255\n", res_x, res_y);
    int header_size = bytes_written + 1;
    fwrite(ppm_header, header_size, 1, f);
    fwrite(rgb8_image, size, 1, f);
    fclose(f);

    free(rgb8_image);
    return 0;
}
