#version 330 core

in vec2 uv;
in vec3 view_cs;
in vec3 view_ws;

uniform vec3 camera_ws;
uniform vec2 focal_scale;

out vec4 FragmentColor;


const int SAMPLES_PER_PIXEL = 16;

const mat2 hash_matrix = mat2(vec2( 12.9898, -78.2337), vec2(-71.0317,  19.4571));

vec2 hash(vec2 p)
    { return fract(43758.5453 * sin(hash_matrix * p)); }

//==============================================================================================================================================================
// Material, geometric and light structure definitions
//==============================================================================================================================================================
struct material_t
{
    float kA;                   // global illumination
    float kD;                   // diffuseness
    float kS;                   // specularity
    float kR;                   // reflectance
    float shininess;
    vec3 color;
};

struct sphere_t
{
    vec3 position;
    float radius;
    material_t material;
};

struct plane_t
{
    vec3 normal;
    float d;
    material_t material;
};

struct cylinder_t
{
    vec3 position;
    float a;
    material_t material;
};

struct pointlight_t
{
    vec3 position;
    float k;
    float falloff;
};

//==============================================================================================================================================================
// geometric objects on the scene
//==============================================================================================================================================================
const int NUM_SPHERES = 2;
const int NUM_PLANES = 2;
const int NUM_CYLINDERS = 1;

uniform sphere_t sphere[NUM_SPHERES];
uniform plane_t plane[NUM_PLANES];
uniform cylinder_t cylinder[NUM_CYLINDERS];

//==============================================================================================================================================================
// light sources
//==============================================================================================================================================================
const int NUM_LIGHTS = 2;
uniform pointlight_t pointlight[NUM_LIGHTS];





//==============================================================================================================================================================
// intersection with object routines
//==============================================================================================================================================================
const float EPS  = 0.000001f;
const float INFINITY_1 = 100000.0f;
const float INFINITY_2 = 200000.0f;

float intersect_with_sphere(vec3 o, vec3 p, sphere_t sphere)
{
    float t = INFINITY_2;
    float a = dot(p, p);                            // compute intersection with sphere
    float b = 2.0 * dot(p, o - sphere.position);
    float c = dot(sphere.position - o, sphere.position - o) - sphere.radius * sphere.radius;
    float D = b * b - 4.0 * a * c;

    if(D > 0.0f)
    {
        float t_ = -b - sqrt(D);                    // compute closest intersection
        if(t_ < 0) t_ = -b + sqrt(D);               // first root is negative, try the second one
        if(t_ >= 0) t = t_ / (2.0 * a);             // if we found a positive root, output it otherwise, we'll output infinity
    }
    return t; 
}

float intersect_with_plane(vec3 o, vec3 d, plane_t plane)
{
    float t = INFINITY_2;
    float a = dot(plane.normal, d);                 // compute intersection with plane
    float b = dot(plane.normal, o) + plane.d;
    
    if(abs(a) > EPS)                                // intersection only if a != 0, i.e., the ray is not parallel to the plane (perpendicular to the normal)
    {
        float _t = -b / a;
        if(_t > 0) t = _t;
    }
    return t;
}

float intersect_with_cylinder(vec3 o, vec3 d, cylinder_t cylinder)
{
    float t = INFINITY_2;
    vec2 o_ = vec2(o.x, o.z);
    vec2 d_ = vec2(d.x, d.z);
    vec2 c_ = vec2(cylinder.position.x, cylinder.position.z);
    
    float a = dot(d_, d_);                          // compute intersection with cylinder
    float b = 2.0 * (dot(o_, d_) - dot(c_, d_));
    float c = dot(o_ - c_, o_ - c_) - cylinder.a * cylinder.a;
    float D = b * b - 4.0 * a * c;

    if(D > 0.0f)
    {
        float t_ = -b - sqrt(D);                    // compute closest intersection
        if(t_ < 0) t_ = -b + sqrt(D);               // first root is negative, try the second one
        if(t_ >= 0) t = t_ / (2.0 * a);             // if we found a positive root, output it otherwise, we'll output infinity
    }
    return t;
}

const int VOID_ID = 0;
const int SPHERE_TYPE_ID = 1;
const int PLANE_TYPE_ID = 2;
const int CYLINDER_TYPE_ID = 3;

void cast_ray(vec3 o, vec3 d, out int id, out int type_id, out float t)
{
    id = -1;
    t = INFINITY_1;
    
    for(int i = 0; i < NUM_SPHERES; i++)                        // test intersection with spheres
    {
        float t_aux = intersect_with_sphere(o, d, sphere[i]);
        if(t_aux < t) 
        {
            t = t_aux;
            id = i;
            type_id = SPHERE_TYPE_ID;
        }
    }
    
    for(int i = 0; i < NUM_PLANES; i++)                         // test intersection with planes
    {
        float t_aux = intersect_with_plane(o, d, plane[i]);
        if(t_aux < t)
        {
            t = t_aux;
            id = i;
            type_id = PLANE_TYPE_ID;
        }
    }
    
    for(int i = 0; i < NUM_CYLINDERS; i++)                      // test intersection with cylinders
    {
        float t_aux = intersect_with_cylinder(o, d, cylinder[i]);
        if(t_aux < t)
        {
            t = t_aux;
            id = i;
            type_id = CYLINDER_TYPE_ID;
        }
    }
}


void get_material(in int id, in int type_id, out material_t material)
{
    if(type_id == SPHERE_TYPE_ID)
        material = sphere[id].material;
    else if(type_id == PLANE_TYPE_ID)
        material = plane[id].material;
    else if(type_id == CYLINDER_TYPE_ID)
        material = cylinder[id].material;
}

vec3 compute_normal(vec3 intersection, int id, int type_id)
{

    if(type_id == SPHERE_TYPE_ID)
        return normalize(intersection - sphere[id].position);
    if(type_id == PLANE_TYPE_ID)
        return plane[id].normal;
    if(type_id == CYLINDER_TYPE_ID)
    {
        vec3 normal = vec3(2.0f * (intersection.x - cylinder[id].position.x),
                           0.0f, 
                           2.0f * (intersection.z - cylinder[id].position.z));
        return normalize(normal);
    }
    return vec3(0.0f);
}

vec3 point_color(in vec3 inter, in vec3 normal, in vec3 eye2inter, in material_t material)
{
    vec3 color = material.kA * material.color;

    for(int i = 0; i < NUM_LIGHTS; i++)
    {
        vec3 inter2light = pointlight[i].position - inter;
        int occluder_id; int dummy; float t;                                                    // cast shadow ray  
        cast_ray(inter + 0.0001f * normal, inter2light, occluder_id, dummy, t);

        // diffuse and specular only if point of intersection is not occluded by another object OR occlusion occurs AFTER the light source
        if(occluder_id == -1 || t > 1.0f)
        {
            float falloff = 1.0f / (pointlight[i].falloff * dot(inter2light, inter2light));     // light attenuation, proportional to 1/(a.d²)
            float diff = dot(normalize(inter2light), normal);                                   // diffuse light
            diff = max(diff, 0.0f);
            float diffuse = diff * material.kD;
            vec3 ref_ray = reflect(-normalize(inter2light), normal);                            // specular light
            float spec = dot(ref_ray, -eye2inter);
            spec = max(spec, 0.0f);
            float specular = spec * material.kS;
            color += (pointlight[i].k * falloff * (diffuse + specular)) * material.color;       // add all
        }
    }
    return color;
}

void main()
{
    vec2 uv = gl_FragCoord.xy;
    vec3 sample_color = vec3(0.0f, 0.0f, 0.0f);

    for(int i = 0; i < SAMPLES_PER_PIXEL; i++)
    {
        vec2 uv0 = uv + i * vec2(-31.1579, 71.7943);
        vec2 shift = hash(uv0) - 0.5;
        vec3 v = view_cs;
        v.xy += focal_scale * shift;

        int max_depth = 3;                                                                  // this is computed from first to last intersection
        vec3 ray_origin = vec3(0.0f);
        vec3 ray_dir = normalize(v);
        float reflectance = 1.0f;

        while(max_depth > 0)
        {
            int id;                                                                         // compute closest intersection
            int type_id;
            float t;
            cast_ray(ray_origin, ray_dir, id, type_id, t);
            
            if(id == -1)                                                                    // if we intersected nothing, stop, otherwise, compute pixel color and set next recursion level
                max_depth = 0;
            else
            {
                material_t material;                                                        // needed data
                get_material(id, type_id, material);
                vec3 inter = ray_origin + t * ray_dir;
                vec3 normal = compute_normal(inter, id, type_id);
                vec3 eye2inter = inter - ray_origin;
                vec3 color = point_color(inter, normal, eye2inter, material);               // compute final color
                sample_color += reflectance * color;                                        // add computed color to accumulator
                ray_dir = reflect(ray_dir, normal);                                         // set parameters for the next level of recursion
                ray_origin = inter + normal * 0.001f;                                       // avoid spurious intersections
                reflectance = material.kR;
                max_depth -= 1;                                                             // decrease depth counter
            }
        }
    }

    sample_color *= (1.0f / SAMPLES_PER_PIXEL);
//    FragmentColor = vec4(sample_color, 1.0f);
    FragmentColor = vec4(1.0f);
}


