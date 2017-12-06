#include <cstdlib> 
#include <cstdio> 
#include <thread>
#include <vector> 

#include <glm/glm.hpp> 

#include "constants.hpp" 
 
const float INF = 1e8;
const int MAX_RAY_DEPTH = 16;
 
struct sphere_t
{ 
    glm::vec3 center;                               // position of the sphere 
    float radius, radius_sqr;                       // sphere radius and radius squared
    glm::vec3 albedo_color, emission_color;         // surface albedo and emission (light) colors
    float transparency, reflectivity;               // surface transparency and reflectivity

    sphere_t(const glm::vec3& center, float radius,
             const glm::vec3& albedo_color, const glm::vec3& emission_color, 
             float reflectivity, float transparency)
        : center(center), radius(radius), 
          albedo_color(albedo_color), emission_color(emission_color),
          transparency(transparency), reflectivity(reflectivity) 
    {
        radius_sqr = radius * radius;
    }

    bool intersect(const glm::vec3& origin, const glm::vec3& direction, float& t0, float& t1) const 
    { 
        glm::vec3 l = center - origin; 
        float tca = glm::dot(l, direction);
        if (tca < 0)
            return false;
        float d2 = glm::dot(l, l) - tca * tca;
        if (d2 > radius_sqr)
            return false;
        float thc = sqrt(radius_sqr - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true; 
    } 
}; 
 
float mix(float a, float b, const float mix)
{ 
    return b * mix + a * (1 - mix); 
} 
 
glm::vec3 trace(const glm::vec3& origin, const glm::vec3 &raydir, const std::vector<sphere_t>& spheres, const int &depth) 
{ 
    float tnear = INF; 
    const sphere_t* sphere = 0;
    
    for (unsigned i = 0; i < spheres.size(); ++i)                   // find intersection of this ray with the sphere in the scene
    { 
        float t0 = INF, t1 = INF;
        if (spheres[i].intersect(origin, raydir, t0, t1))
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

    if (!sphere)                                                                // if there's no intersection return white color
        return glm::vec3(1.0f); 

    glm::vec3 color = glm::vec3(0.0f);                                          // color of the ray / surfaceof the object intersected by the ray 
    glm::vec3 phit = origin + raydir * tnear;                                   // point of intersection 
    glm::vec3 nhit = phit - sphere->center;                                     // normal at the intersection point 
    nhit = glm::normalize(nhit);                                                // normalize normal direction 

    // If the normal and the view direction are not opposite to each other reverse the normal direction. 
    // That also means we are inside the sphere so set the inside bool to true. 
    // Finally reverse the sign of IdotN which we want to be positive.

    float bias = 1e-4;                                                          // add some bias to the point from which we will be tracing 
    bool inside = false; 

    if (glm::dot(raydir, nhit) > 0.0f)
    {
        nhit = -nhit;
        inside = true;
    }

    if ((sphere->transparency > 0.0 || sphere->reflectivity > 0.0) && depth < MAX_RAY_DEPTH)
    { 
        float facingratio = -glm::dot(raydir, nhit); 
        float fresneleffect = mix(pow(1 - facingratio, 3), 1.0f, 0.1f);         // change the mix value to tweak the effect
        glm::vec3 refldir = raydir - 2.0f * glm::dot(raydir, nhit) * nhit;      // compute reflection direction (not need to normalize because all vectors are already normalized)
        refldir = glm::normalize(refldir);
        glm::vec3 reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1); 
        glm::vec3 refraction = glm::vec3(0.0f); 
        
        if (sphere->transparency)                                               // if the sphere is also transparent compute refraction ray (transmission)
        {
            float ior = 1.1f; 
            float eta = (inside) ? ior : 1 / ior;                               // are we inside or outside the surface? 
            float cosi = -glm::dot(nhit, raydir); 
            float k = 1 - eta * eta * (1 - cosi * cosi); 
            glm::vec3 refrdir = raydir * eta + nhit * (eta * cosi - glm::sqrt(k)); 
            refrdir = glm::normalize(refrdir); 
            refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1); 
        } 
        // the result is a mix of reflection and refraction (if the sphere is transparent)
        color = (reflection * fresneleffect + refraction * (1 - fresneleffect) * sphere->transparency) * sphere->albedo_color; 
    } 
    else                                                                        // it's a diffuse object, no need to raytrace any further
    { 
        for (unsigned int i = 0; i < spheres.size(); ++i)                       
        {
            if (spheres[i].emission_color.x > 0.0f)
            {
                glm::vec3 transmission = glm::vec3(1.0f);                       // this is a light
                glm::vec3 light = spheres[i].center - phit; 
                light = glm::normalize(light);
                for (unsigned int j = 0; j < spheres.size(); ++j)
                { 
                    if (i != j)
                    {
                        float t0, t1;
                        if (spheres[j].intersect(phit + nhit * bias, light, t0, t1))
                        {
                            transmission = glm::vec3(0.0f); 
                            break; 
                        } 
                    } 
                } 
                color += sphere->albedo_color * transmission * std::max(float(0), glm::dot(nhit, light)) * spheres[i].emission_color; 
            } 
        } 
    } 
 
    return color + sphere->emission_color; 
}

struct ray_trace_data_t
{
    std::vector<sphere_t> spheres;

    int res_x;
    int res_y;
    float fov;

    float inv_res_x;
    float inv_res_y;
    float aspect_ratio; 
    float angle;

    ray_trace_data_t(int res_x, int res_y, float fov)
        : res_x(res_x), res_y(res_y), fov(fov)
    {
        inv_res_x = 1.0 / float(res_x);
        inv_res_y = 1.0 / float(res_y);
        aspect_ratio = float(res_x) / float(res_y); 
        angle = tan(constants::pi * 0.5 * fov / 180.0f);
    }
};

void ray_trace_thread(const ray_trace_data_t* ray_trace_data, uint8_t* rgb8_image, int y_min, int y_max)
{
    int index = 3 * y_min * ray_trace_data->res_x;
    for (int y = y_min; y < y_max; ++y)
    { 
        for (int x = 0; x < ray_trace_data->res_x; ++x)
        { 
            float xx = (2 * ((x + 0.5) * ray_trace_data->inv_res_x) - 1.0) * ray_trace_data->angle * ray_trace_data->aspect_ratio; 
            float yy = (1 - 2 * ((y + 0.5) * ray_trace_data->inv_res_y)) * ray_trace_data->angle; 
            glm::vec3 direction = glm::vec3(xx, yy, -1); 
            direction = glm::normalize(direction);
            glm::vec3 color = trace(glm::vec3(0), direction, ray_trace_data->spheres, 0);
            color = 255.0f * glm::clamp(color, 0.0f, 1.0f);

            rgb8_image[index++] = (uint8_t) color.r;
            rgb8_image[index++] = (uint8_t) color.g;
            rgb8_image[index++] = (uint8_t) color.b; 
        } 
    }
}

 
int main(int argc, char* argv[])
{ 
    srand(13);

    const int res_x = 4096;
    const int res_y = 2048;

    ray_trace_data_t ray_trace_data(res_x, res_y, 30.0f);

    std::vector<sphere_t>& spheres = ray_trace_data.spheres;
    spheres.push_back(sphere_t(glm::vec3( 0.0f, -10004.0f, -20.0f), 10000.0f, glm::vec3(0.20f, 0.20f, 0.20f), glm::vec3(0.0f), 0.0f, 0.0f));
    spheres.push_back(sphere_t(glm::vec3( 0.0f,      0.0f, -20.0f),     4.0f, glm::vec3(1.00f, 0.32f, 0.36f), glm::vec3(0.0f), 1.0f, 0.5f)); 
    spheres.push_back(sphere_t(glm::vec3( 5.0f,     -1.0f, -15.0f),     2.0f, glm::vec3(0.90f, 0.76f, 0.46f), glm::vec3(0.0f), 1.0f, 0.0f)); 
    spheres.push_back(sphere_t(glm::vec3( 5.0f,      0.0f, -25.0f),     3.0f, glm::vec3(0.65f, 0.77f, 0.97f), glm::vec3(0.0f), 1.0f, 0.0f)); 
    spheres.push_back(sphere_t(glm::vec3(-5.5f,      0.0f, -15.0f),     3.0f, glm::vec3(0.90f, 0.90f, 0.90f), glm::vec3(0.0f), 1.0f, 0.0f));
    spheres.push_back(sphere_t(glm::vec3( 0.0f,     20.0f, -30.0f),     3.0f, glm::vec3(0.00f, 0.00f, 0.00f), glm::vec3(3.0f), 0.0f, 0.0f));

    const int size = 3 * res_x * res_y * sizeof(uint8_t);
    uint8_t* rgb8_image = (uint8_t*) malloc (size);

    const int threads = 8;
    std::thread computation_thread[threads - 1];

    int y_div = res_y / threads;
    int y_rem = res_y % threads;
    int y_min = 0;
    int y_max;

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        y_max = y_min + y_div + int(thread_id < y_rem);
        computation_thread[thread_id] = std::thread(ray_trace_thread, &ray_trace_data, rgb8_image, y_min, y_max);
        y_min = y_max;
    }

    ray_trace_thread(&ray_trace_data, rgb8_image, y_min, res_y);

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        computation_thread[thread_id].join();


    FILE* f = fopen("untitled.ppm", "wb");

    char ppm_header[128];
    int bytes_written = 1 + sprintf(ppm_header, "P6\n%d %d\n255\n", res_x, res_y);
    int header_size = bytes_written + 1;
    fwrite(ppm_header, header_size, 1, f);
    fwrite(rgb8_image, size, 1, f);
    fclose(f);

    free(rgb8_image);
    return 0;
}
