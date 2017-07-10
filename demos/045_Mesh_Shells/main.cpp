//========================================================================================================================================================================================================================
// DEMO 045 : Mesh Shells visualizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <atomic>
#include <thread>
#include <map>
 
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
#include "glsl_noise.hpp"
#include "plato.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "polyhedron.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "momenta.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    GLenum mode = GL_FILL;

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

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
        {
            mode = (mode == GL_FILL) ? GL_LINE : GL_FILL;
            glPolygonMode(GL_FRONT_AND_BACK, mode);
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

const double INTEGRAL_SCALE = 268435456.0;
const double INV_INT_SCALE = 1.0 / INTEGRAL_SCALE;


//=======================================================================================================================================================================================================================
// unsigned distance-to-triangle function
// has been tested a lot, works for non-degenerate triangles
//=======================================================================================================================================================================================================================
template<typename real_t> real_t triangle_udf(const glm::tvec3<real_t>& p, const glm::tvec3<real_t>& a, const glm::tvec3<real_t>& b, const glm::tvec3<real_t>& c)
{
    glm::tvec3<real_t> ba = b - a; glm::tvec3<real_t> pa = p - a;
    glm::tvec3<real_t> cb = c - b; glm::tvec3<real_t> pb = p - b;
    glm::tvec3<real_t> ac = a - c; glm::tvec3<real_t> pc = p - c;
    glm::tvec3<real_t> n = glm::cross(ba, ac);

    real_t q = glm::sign(glm::dot(glm::cross(ba, n), pa)) + 
               glm::sign(glm::dot(glm::cross(cb, n), pb)) + 
               glm::sign(glm::dot(glm::cross(ac, n), pc));

    if (q >= (real_t) 2.0) 
        return glm::sqrt(glm::dot(n, pa) * glm::dot(n, pa) / glm::length2(n));

    return glm::sqrt(
        glm::min(
            glm::min(
                glm::length2(ba * glm::clamp(glm::dot(ba, pa) / glm::length2(ba), (real_t) 0.0, (real_t) 1.0) - pa),
                glm::length2(cb * glm::clamp(glm::dot(cb, pb) / glm::length2(cb), (real_t) 0.0, (real_t) 1.0) - pb)
            ), 
            glm::length2(ac * glm::clamp(glm::dot(ac, pc) / glm::length2(ac), (real_t) 0.0, (real_t) 1.0) - pc)
        )
    );
}

unsigned int atomic_min(std::atomic_uint& atomic_var, unsigned int value)
{
    unsigned int previous_value = atomic_var;
    while(previous_value > value && !atomic_var.compare_exchange_weak(previous_value, value));
    return previous_value;
}

const glm::dvec3 shift[8] =
{
    glm::dvec3(-1.0, -1.0, -1.0),
    glm::dvec3( 1.0, -1.0, -1.0),
    glm::dvec3(-1.0,  1.0, -1.0),
    glm::dvec3( 1.0,  1.0, -1.0),
    glm::dvec3(-1.0, -1.0,  1.0),
    glm::dvec3( 1.0, -1.0,  1.0),
    glm::dvec3(-1.0,  1.0,  1.0),
    glm::dvec3( 1.0,  1.0,  1.0)
};


struct texture3d_t
{
    glm::ivec3 size;
	GLuint texture_id;
    GLenum texture_unit;
    GLenum internal_format;

    texture3d_t() {}

    texture3d_t(const glm::ivec3& size, GLenum texture_unit, GLenum internal_format)
        : size(size), texture_unit(texture_unit), internal_format(internal_format)
    {
    	glActiveTexture(texture_unit);
	    glGenTextures(1, &texture_id);
	    glBindTexture(GL_TEXTURE_3D, texture_id);

	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage3D(GL_TEXTURE_3D, 1, internal_format, size.x, size.y, size.z);
    }

    void bind_as_image(GLuint image_unit, GLenum access = GL_READ_WRITE)
        { glBindImageTexture(image_unit, texture_id, 0, GL_TRUE, 0, access, internal_format); }

    GLuint data_size()
        { return size.x * size.y * size.z * sizeof(GLuint); }

    ~texture3d_t()
        { glDeleteTextures(1, &texture_id); }
};


struct sdf_compute_t
{
    vao_t::header_t header;

    GLuint V;
    GLuint F;
    GLuint* indices;

    double bbox_max;

    glm::dvec3* positions; 
    glm::dvec3* normals; 

    GLuint ibo_id;

    sdf_compute_t() {}

    void load_model(const char* file_name, double bbox_max)
    {
        debug_msg("Loading model :: %s ... \n", file_name);

        FILE* f = fopen(file_name, "rb");
        fread (&header, sizeof(vao_t::header_t), 1, f);

        assert(header.layout == vertex_pn_t::layout && "File does not contain a valid PN - model");
        assert(header.mode == GL_TRIANGLES && "Primitive type must be GL_TRIANGLES");
        assert(header.type == GL_UNSIGNED_INT && "Index type is not GL_UNSIGNED_INT");
        V = header.vbo_size;
        F = header.ibo_size / 3;

        GLsizei stride = vertex_pn_t::total_dimension * sizeof(GLfloat);
        vertex_pn_t* vertices = (vertex_pn_t*) malloc(V * stride);
        fread(vertices, stride, header.vbo_size, f);
    
        indices = (GLuint *) malloc(sizeof(GLuint) * header.ibo_size);
        fread(indices, sizeof(GLuint), header.ibo_size, f);
        fclose(f);

        debug_msg("VAO Loaded :: V = %d. F = %d. indices = %d. ", V, F, header.ibo_size);
        
        sdf_compute_t::bbox_max = bbox_max;
        debug_msg("Normalizing the model :: bbox_max = %f.", bbox_max);

        positions = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V); 
        normals = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V);

        glm::dvec3 mass_center;
        glm::dmat3 covariance_matrix;

        momenta::calculate(vertices, V, mass_center, covariance_matrix);
        debug_msg("model mass center = %s", glm::to_string(mass_center).c_str());
        debug_msg("model covariance matrix = %s", glm::to_string(covariance_matrix).c_str());

        glm::dquat q = diagonalizer(covariance_matrix);
        glm::dmat3 Q = mat3_cast(q);
        glm::dmat3 Qt = glm::transpose(Q);

        debug_msg("diagonalizer = %s", glm::to_string(Q).c_str());

        glm::dvec3 bbox = glm::dvec3(0.0);

        for (GLuint v = 0; v < V; ++v)
        {
            vertex_pn_t& vertex = vertices[v];
            glm::dvec3 position = Q * (glm::dvec3(vertex.position) - mass_center);       
            positions[v] = position;
            bbox = glm::max(bbox, glm::abs(position));
            normals[v] = Q * vertex.normal;
        }

        double max_bbox = glm::max(bbox.x, glm::max(bbox.y, bbox.z));
        double scale = bbox_max / max_bbox;

        debug_msg("model bbox = %s", glm::to_string(bbox).c_str());
        debug_msg("bbox_max = %f. maximum = %f. scale = %f. Scaling ...", bbox_max, max_bbox, scale);

        bbox = glm::dvec3(0.0);
        for (GLuint v = 0; v < V; ++v)
        {
            positions[v] = scale * positions[v];
            bbox = glm::max(bbox, glm::abs(positions[v]));
        }

        covariance_matrix = (scale * scale) * (Q * covariance_matrix * Qt);

        debug_msg("model covariance matrix after normalization = %s", glm::to_string(covariance_matrix).c_str());
        debug_msg("Verification :: ");

        momenta::calculate(positions, V, mass_center, covariance_matrix);

        debug_msg("model mass center = %s", glm::to_string(mass_center).c_str());
        debug_msg("model covariance matrix = %s", glm::to_string(covariance_matrix).c_str());
        debug_msg("model bbox = %s", glm::to_string(bbox).c_str());

        free(vertices);
    }

    void compute_angle_weighted_normals()
    {
        debug_msg("Averaging face normals with angular weight ...");
        memset(normals, 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A = positions[indices[3 * f + 0]];
            glm::dvec3 B = positions[indices[3 * f + 1]];
            glm::dvec3 C = positions[indices[3 * f + 2]];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            glm::dvec3 AB = normalize(B - A);
            glm::dvec3 BC = normalize(C - B);
            glm::dvec3 CA = normalize(A - C);

            double qA = glm::sqrt(1.0 + dot(CA, AB));
            double qB = glm::sqrt(1.0 + dot(AB, BC));
            double qC = glm::sqrt(1.0 + dot(BC, CA));

            normals[indices[3 * f + 0]] += qA * n;
            normals[indices[3 * f + 1]] += qB * n;
            normals[indices[3 * f + 2]] += qC * n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
    }

    void calculate_statistics()
    {
        double area = 0.0;
        double max_area = 0.0;
        double edge = 0.0;
        double max_edge = 0.0;

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A = positions[indices[3 * f + 0]];
            glm::dvec3 B = positions[indices[3 * f + 1]];
            glm::dvec3 C = positions[indices[3 * f + 2]];

            double eAB = glm::length(B - A);
            double eBC = glm::length(C - B);
            double eCA = glm::length(A - C);

            edge += (eAB + eBC + eCA);
            max_edge = glm::max(max_edge, eAB);
            eBC = glm::max(eBC, eCA);
            max_edge = glm::max(max_edge, eBC);

            double a = glm::length(glm::cross(B - A, C - A));
            area += a;
            max_area = glm::max(max_area, a);
        }

        debug_msg("Max edge length = %f", max_edge);
        debug_msg("Average edge length = %f", edge / (3 * F));
        debug_msg("Max area = %f", max_area);
        debug_msg("Average area = %f", area / F);
    }

    vao_t create_vao()
    {
        vertex_pn_t* vertices = (vertex_pn_t*) malloc(V * sizeof(vertex_pn_t));

        for (GLuint v = 0; v < V; ++v)
        {
            vertices[v].position = glm::vec3(positions[v]);
            vertices[v].normal = glm::vec3(normals[v]);
        }

        vao_t model_vao = vao_t(GL_TRIANGLES, vertices, V, indices, 3 * F);
        free(vertices);

        return model_vao;
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

    demo_window_t window("Model shell visualizer", 4, 3, 3, res_x, res_y, true);

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    glsl_program_t shell_visualizer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/model_shell.vs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/model_shell.fs"));

    shell_visualizer.enable();
    uniform_t uni_pv_matrix = shell_visualizer["projection_view_matrix"];
    uniform_t uni_delta     = shell_visualizer["delta"];
    uniform_t uni_color     = shell_visualizer["color"];
    uniform_t uni_camera_ws = shell_visualizer["camera_ws"];
    uniform_t uni_light_ws  = shell_visualizer["light_ws"];

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    sdf_compute_t sdf_compute;
    sdf_compute.load_model("../../../resources/models/vao/demon.vao", 10.0);
    sdf_compute.calculate_statistics();
    sdf_compute.compute_angle_weighted_normals();

    vao_t demon_vao = sdf_compute.create_vao();

    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();

        float time = window.frame_ts;
        const float light_radius = 1.707f;
        glm::vec3 light_ws = glm::vec3(light_radius * cos(0.5f * time), 2.0f, light_radius * sin(0.5f * time));
        glm::vec3 camera_ws = window.camera.position();

        uni_pv_matrix = projection_view_matrix;
        uni_camera_ws = camera_ws;  
        uni_light_ws = light_ws;

        uni_delta = (float) 0.0f;
        uni_color = glm::vec3(1.0f, 0.0f, 0.0f);
        demon_vao.render();

        uni_delta = (float) 0.00125f;
        uni_color = glm::vec3(0.717f, 0.717f, 0.0f);
        demon_vao.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}