//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D raymarcher
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "sdf.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.01f);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);  
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt); 
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt); 
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

//===================================================================================================================================================================================================================
// procedural generator of sdf texture of the type GL_R32F
// optionally saves generated data to file
//===================================================================================================================================================================================================================
struct proc_sdf_compute_data_t
{
    glm::ivec3 size;
    glm::dvec3 bbox_base;
    glm::dvec3 bbox_cell;    
};

template<typename field_generator_func_t> void fill_sdf_chunk(proc_sdf_compute_data_t* compute_data, int z_min, int z_max, float* texture_data_chunk)
{
    field_generator_func_t generator_func;
    float* texture_data_ptr = texture_data_chunk;

    for(int z = z_min; z < z_max; ++z)
        for(int y = 0; y < compute_data->size.y; ++y)
            for(int x = 0; x < compute_data->size.x; ++x)
            {
                glm::dvec3 p = compute_data->bbox_base + compute_data->bbox_cell * glm::dvec3(x, y, z);
                *texture_data_ptr = generator_func(p);
                ++texture_data_ptr;
            }
}

template<typename field_generator_func_t, int threads> texture3d_t generate_sdf(GLenum texture_unit, const glm::ivec3& size, const bbox_t& bbox, const char* file_name = 0)
{
    int z_per_thread = size.z / threads; 
    int z_extra = size.z % threads; 

    glm::dvec3 bbox_cell = bbox.size / glm::dvec3(size - 1);

    proc_sdf_compute_data_t compute_data = 
    {
        .size = size,
        .bbox_base = bbox.center - 0.5 * bbox.size,
        .bbox_cell = bbox_cell
    };

    const int z_layer_size = size.x * size.y;
    const int texture_size = z_layer_size * size.z;
    const int data_size = texture_size * sizeof(float);

    float* texture_data = (float*) malloc(data_size);
    float* texture_data_chunk = texture_data;

    std::thread computation_thread[threads - 1];

    GLuint z_min = 0;
    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
    {
        GLuint z_max = z_min + z_per_thread + GLint(thread_id < z_extra);
        debug_msg("Launching thread #%u. Z-interval: [%u, %u].", thread_id, z_min, z_max);
        computation_thread[thread_id] = std::thread(fill_sdf_chunk<field_generator_func_t>, &compute_data, z_min, z_max, texture_data_chunk);
        texture_data_chunk += z_layer_size * (z_max - z_min);
        z_min = z_max;
    }    

    debug_msg("Main thread :: Z-interval: [%u, %u].", z_min, size.z);
    fill_sdf_chunk<field_generator_func_t>(&compute_data, z_min, size.z, texture_data_chunk);

    for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        computation_thread[thread_id].join();

    tex3d_header_t tex3d_header = 
    {
        .target = GL_TEXTURE_3D,
        .internal_format = GL_R32F,
        .format = GL_RED,
        .type = GL_FLOAT,
        .size = size,
        .data_size = data_size
    };

    sdf_header_t sdf_header
    {
        .tex3d_header = tex3d_header,
        .bbox = bbox
    };

    if (file_name)
    {
        debug_msg("Writing results to: %s", file_name);
        FILE* f = fopen(file_name, "wb");
        fwrite(&sdf_header, sizeof(sdf_header_t), 1, f);
        fwrite(texture_data, data_size, 1, f);
        fclose(f);
    }

    texture3d_t texture = texture3d_t(texture_unit, tex3d_header, texture_data);
    free(texture_data);
    return texture;
}

struct signed_distance_field
{
    float operator () (const glm::dvec3& p)
    {
        return holed_box_sdf(p);
    }

    double cube_sdf (const glm::dvec3& p)
    {
        const double size = 0.7500000001;
        return glm::max(glm::max(glm::abs(p.x), glm::abs(p.y)), glm::abs(p.z)) - size;
    }

    double sphere_sdf(const glm::dvec3& p)
    {
        const double radius = 0.5;
        return glm::length(p) - radius;
    }

    double dodecahedron_sdf(const glm::dvec3& p)
    {
        const double phi = 1.618033988749894848204586834365638117720309179805762862135; // (sqrt(5) + 1) / 2
        const double psi = 1.511522628152341460960267404050002785276889577787122118459; // (sqrt(5) + 3) / (2 * sqrt(3))
        glm::dvec3 q = glm::abs(p);
        glm::dvec3 l = q + phi * glm::dvec3(q.y, q.z, q.x);
        return 0.5 * glm::max(l.x, glm::max(l.y, l.z)) - 0.5 * psi;
    }

    double icosahedron_sdf(const glm::dvec3& p)
    {
        const double size = 0.75;
        const double inv_sqrt3 = 0.577350269189625764509148780501957455647601751270126876018; // 1 / sqrt(3)
        const double tau       = 0.934172358962715696451118623548045329629287826516995242440; // (sqrt(5) + 1) / (2 * sqrt(3))
        const double psi       = 0.356822089773089931941969843046087873981686075246868366421; // (sqrt(5) - 1) / (2 * sqrt(3))

        glm::dvec3 q = glm::abs(p);
        double v = inv_sqrt3 * (q.x + q.y + q.z);
        glm::dvec3 l = tau * q + psi * glm::dvec3(q.y, q.z, q.x);
        return glm::max(glm::max(v, l.x), glm::max(l.y, l.z)) - size;
    }

    double torus_sdf(const glm::dvec3& p)
    {
        const double R = 0.71;
        const double r = 0.26;
        glm::dvec2 q = glm::dvec2(glm::length(glm::dvec2(p.x, p.y)) - R, p.z);
        return length(q) - r;
    }

    double hexprism_sdf(const glm::dvec3& p)
    {
        const double half_root3 = 0.866025403784438646763723170752936183471402626905190314027;
        const double half = 0.5;
        const double height = 0.85;
        const double radius = 0.65;
        glm::dvec3 q = glm::abs(p);
        return glm::max(q.z - height, glm::max((half_root3 * q.x + half * q.y), q.y) - radius);
    }

    double box_sdf(const glm::dvec3& p)
    {
        glm::dvec3 sizes = glm::dvec3(0.57, 0.93, 0.21);
        glm::dvec3 d = glm::abs(p) - sizes;
        return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0) + glm::length(glm::max(d, 0.0));
    }

    double twisted_box_sdf(const glm::dvec3& p)
    {
        const double angle = 0.57 * p.y;
        const double cs = glm::cos(angle);
        const double sn = glm::sin(angle);

        double x =  cs * p.x + sn * p.z; 
        double z = -sn * p.x + cs * p.z;
        glm::dvec3 q = glm::dvec3(x, p.y, z);

        glm::dvec3 sizes = glm::dvec3(0.47, 0.93, 0.17);
        glm::dvec3 d = glm::abs(q) - sizes;
        return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0) + glm::length(glm::max(d, 0.0));
    }

    double balls_sdf(const glm::dvec3& p)
    {
        const double r = 0.4;
        const double R = 0.47;
        glm::dvec3 q = glm::abs(p) - glm::dvec3(r);
        double s = glm::length(q) - R;
        return s;
    }

    double holed_box_sdf(const glm::dvec3& p)
    {
        glm::dvec3 external_size = glm::dvec3(0.57, 0.93, 0.37);
        glm::dvec3 internal_size = external_size - glm::dvec3(0.13);
        glm::dvec3 q = glm::abs(p);

        glm::dvec3 de = q - external_size;
        glm::dvec3 di = q - internal_size;

        double qe = glm::min(glm::max(de.x, glm::max(de.y, de.z)), 0.0) + glm::length(glm::max(de, 0.0));
        double qi = glm::min(glm::max(di.x, glm::max(di.y, di.z)), 0.0) + glm::length(glm::max(di, 0.0));
        return glm::max(qe, -qi);
    }


};


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Simple Volume RayMarch", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // volume raymarch shader
    //===================================================================================================================================================================================================================
    glsl_program_t sdf_raymarch(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/sdf_marcher.fs"));

    sdf_raymarch.enable();

    uniform_t uni_cm_camera_matrix = sdf_raymarch["camera_matrix"];       
    uniform_t uni_cm_camera_ws     = sdf_raymarch["camera_ws"];       
    uniform_t uni_cm_light_ws      = sdf_raymarch["light_ws"];

    glm::vec2 focal_scale = glm::vec2(1.0f / window.camera.projection_matrix[0][0], 1.0f / window.camera.projection_matrix[1][1]);

    sdf_raymarch["focal_scale"] = focal_scale;
    sdf_raymarch["tb_tex"] = 0;
    sdf_raymarch["environment_tex"] = 1;
    sdf_raymarch["sdf_tex"] = 2;

    //===================================================================================================================================================================================================================
    // load textures
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE0);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/metal.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);
    
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    const char* sunset_files[6] = {"../../../resources/cubemap/sunset/positive_x.png",
                                   "../../../resources/cubemap/sunset/negative_x.png",
                                   "../../../resources/cubemap/sunset/positive_y.png",
                                   "../../../resources/cubemap/sunset/negative_y.png",
                                   "../../../resources/cubemap/sunset/positive_z.png",
                                   "../../../resources/cubemap/sunset/negative_z.png"};
    GLuint env_tex_id = image::png::cubemap(sunset_files);

    bbox_t bbox;

//    texture3d_t demon_sdf = texture3d_t::load_sdf(GL_TEXTURE2, "demon.sdf", bbox);

    debug_msg("GL_R32F = %u. GL_RED = %u. GL_FLOAT = %u.", GL_R32F, GL_RED, GL_FLOAT);
    bbox.size = glm::dvec3(2.0f);
    bbox.center = glm::dvec3(0.0f);
    texture3d_t demon_sdf = generate_sdf<signed_distance_field, 8>(GL_TEXTURE2, glm::ivec3(256), bbox, "dodecahedron.sdf");



    glm::dvec3 bbox_size   = bbox.size;
    glm::dvec3 bbox_center = bbox.center;
    glm::dvec3 bbox_min = bbox_center - 0.5 * bbox_size;
    glm::dvec3 bbox_max = bbox_center + 0.5 * bbox_size;    

    debug_msg("SDF loaded :: bbox size   = %s", glm::to_string(bbox_size).c_str());
    debug_msg("              bbox center = %s", glm::to_string(bbox_center).c_str());

    sdf_raymarch["bbox_min"] = glm::vec3(bbox_min);
    sdf_raymarch["bbox_center"] = glm::vec3(bbox_center);
    sdf_raymarch["bbox_half_size"] = glm::vec3(0.5 * bbox_size);
    sdf_raymarch["bbox_inv_size"] = glm::vec3(1.0 / bbox_size);

    //===================================================================================================================================================================================================================
    // light variables
    //===================================================================================================================================================================================================================
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT);

        float time = 0.55 * window.frame_ts;
        glm::vec3 light_ws = glm::vec3(12.5f, 1.75f * glm::cos(time), 1.75f * glm::sin(time));

        glm::mat4 cmatrix4x4;
        glm::mat3 camera_matrix;
        glm::vec3 camera_ws;

if (0 == 0) {


        float radius = 2.05f + 0.75f * glm::cos(0.173f * time);
        float z = -0.35f + 0.35f * glm::sin(0.191f * time);
        camera_ws = glm::vec3(radius * glm::cos(0.314f * time), z, radius * glm::sin(0.314f * time));
        glm::vec3 up = glm::normalize(glm::vec3(glm::cos(0.41 * time), -6.0f, glm::sin(0.41 * time)));
        glm::mat4 view_matrix = glm::lookAt(camera_ws, glm::vec3(0.0f), up);
        cmatrix4x4 = glm::inverse(view_matrix);
        camera_matrix = glm::mat3(cmatrix4x4);
}
else
{
        cmatrix4x4 = glm::inverse(window.camera.view_matrix);
        camera_matrix = glm::mat3(cmatrix4x4);
        camera_ws = glm::vec3(cmatrix4x4[3]);
}


        //===============================================================================================================================================================================================================
        // raymarch through signed distance field
        //===============================================================================================================================================================================================================
        uni_cm_camera_matrix = camera_matrix;
        uni_cm_camera_ws = camera_ws;
        uni_cm_light_ws = light_ws;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
