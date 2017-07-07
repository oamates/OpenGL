//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D generator
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <atomic>
#include <thread>
 
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

        GLsizei stride = vertex_pn_t::total_dimension * sizeof(GLfloat);
        vertex_pn_t* vertices = (vertex_pn_t*) malloc(header.vbo_size * stride);
        fread(vertices, stride, header.vbo_size, f);
    
        indices = (GLuint *) malloc(sizeof(GLuint) * header.ibo_size);
        fread(indices, sizeof(GLuint), header.ibo_size, f);
        fclose(f);

        V = header.vbo_size;
        F = header.ibo_size / 3;
        debug_msg("VAO Loaded :: V = %d. F = %d. indices = %d. ", V, F, header.ibo_size);

        debug_msg("\n\n\n!!!!!!!!!!!! Testing degeneracies for original model. !!!!!!!!!!!!\n\n");
        for(GLuint f = 0; f < F; ++f)
        {
            glm::vec3 A  = vertices[indices[3 * f + 0]].position;
            glm::vec3 nA = vertices[indices[3 * f + 0]].normal;
            glm::vec3 B  = vertices[indices[3 * f + 1]].position;
            glm::vec3 nB = vertices[indices[3 * f + 1]].normal;
            glm::vec3 C  = vertices[indices[3 * f + 2]].position;
            glm::vec3 nC = vertices[indices[3 * f + 2]].normal;

            glm::vec3 n = glm::normalize(glm::cross(B - A, C - A));

            float dpA = glm::dot(n, nA);
            float dpB = glm::dot(n, nB);
            float dpC = glm::dot(n, nC);

            if ((dpA < 0.0) || (dpB < 0.0) || (dpC < 0.0))
            {
                debug_msg("Normal degeneracy at triangle %u ::", f);
                //debug_msg("\tdpA = %f, dpB = %f, dpC = %f", dpA, dpB, dpC);
            }
        }

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
        debug_msg("maximum = %f. scale = %f. Scaling ...", max_bbox, scale);

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

        /**/

        debug_msg("\n\n\nTesting distance-to-triangle function\n\n\n");

        for(int i = 0; i < 65536; ++i)
        {
            int base_index = 3 * (i % F);
            glm::dvec3 vA = positions[indices[base_index + 0]];
            glm::dvec3 vB = positions[indices[base_index + 1]];
            glm::dvec3 vC = positions[indices[base_index + 2]];

            glm::dvec3 vP = glm::dvec3(glm::linearRand(-1.0, 1.0), glm::linearRand(-1.0, 1.0), glm::linearRand(-1.0, 1.0));

            double distanceABC = triangle_udf(vP, vA, vB, vC);
            double distanceBCA = triangle_udf(vP, vB, vC, vA);
            double distanceCAB = triangle_udf(vP, vC, vA, vB);

            double distanceAP = glm::length(vP - vA);
            double distanceBP = glm::length(vP - vB);
            double distanceCP = glm::length(vP - vC);

            if ((distanceABC < 0.0) || (distanceBCA < 0.0) || (distanceCAB < 0.0))
            {
                debug_msg("Error !!!!! Negative distance returned !!!!");
                debug_msg("\tdistanceABC = %f, distanceBCA = %f, distanceCAB = %f", distanceABC, distanceBCA, distanceCAB);
            }

            if (distanceAP < distanceABC)
                debug_msg("distanceAP > distanceABC :: %f < %f", distanceAP, distanceABC);
            if (distanceAP < distanceBCA)
                debug_msg("distanceAP > distanceBCA :: %f < %f", distanceAP, distanceBCA);
            if (distanceAP < distanceCAB)
                debug_msg("distanceAP > distanceCAB :: %f < %f", distanceAP, distanceCAB);

            if (distanceBP < distanceABC)
                debug_msg("distanceBP > distanceABC :: %f < %f", distanceBP, distanceABC);
            if (distanceBP < distanceBCA)
                debug_msg("distanceBP > distanceBCA :: %f < %f", distanceBP, distanceBCA);
            if (distanceBP < distanceCAB)
                debug_msg("distanceBP > distanceCAB :: %f < %f", distanceBP, distanceCAB);

            if (distanceCP < distanceABC)
                debug_msg("distanceCP > distanceABC :: %f < %f", distanceCP, distanceABC);
            if (distanceCP < distanceBCA)
                debug_msg("distanceCP > distanceBCA :: %f < %f", distanceCP, distanceBCA);
            if (distanceCP < distanceCAB)
                debug_msg("distanceCP > distanceCAB :: %f < %f", distanceCP, distanceCAB);

            const double eps = 0.00001;

            if ((glm::abs(distanceABC - distanceBCA) > eps) || 
                (glm::abs(distanceBCA - distanceCAB) > eps) || 
                (glm::abs(distanceCAB - distanceABC) > eps))
                debug_msg("Distances not equal ABC = %f, BCA = %f, CAB = %f", distanceABC, distanceBCA, distanceCAB);
        }


        free(vertices);
    }

    void compute_area_weighted_normals()
    {
        debug_msg("Averaging face normals with area weight");
        memset(normals, 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A = positions[indices[3 * f + 0]];
            glm::dvec3 B = positions[indices[3 * f + 1]];
            glm::dvec3 C = positions[indices[3 * f + 2]];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            normals[indices[3 * f + 0]] += n;
            normals[indices[3 * f + 1]] += n;
            normals[indices[3 * f + 2]] += n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
    }

    void compute_angle_weighted_normals()
    {
        debug_msg("Averaging face normals with area weight");
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

    void test_original_degeneracy()
    {

        for(GLuint f = 0; f < F; ++f)
        {
            glm::vec3 A  = positions[indices[3 * f + 0]];
            glm::vec3 nA = normals[indices[3 * f + 0]];
            glm::vec3 B  = positions[indices[3 * f + 1]];
            glm::vec3 nB = normals[indices[3 * f + 1]];
            glm::vec3 C  = positions[indices[3 * f + 2]];
            glm::vec3 nC = normals[indices[3 * f + 2]];

            glm::vec3 n = glm::normalize(glm::cross(B - A, C - A));

            float dpA = glm::dot(n, nA);
            float dpB = glm::dot(n, nB);
            float dpC = glm::dot(n, nC);

            if ((dpA < 0.0) || (dpB < 0.0) || (dpC < 0.0))
            {
                debug_msg("Normal degeneracy at triangle %u ::", f);
                //debug_msg("\tdpA = %f, dpB = %f, dpC = %f", dpA, dpB, dpC);
            }
        }         
    }


    void test_degeneracy(double shift_value)
    {
        debug_msg("\n\n\n!!!!!!!!!!!! Testing degeneracies for shift = %f !!!!!!!!!!!!\n\n", shift_value);
        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A  = positions[indices[3 * f + 0]];
            glm::dvec3 nA = normals[indices[3 * f + 0]];
            glm::dvec3 B  = positions[indices[3 * f + 1]];
            glm::dvec3 nB = normals[indices[3 * f + 1]];
            glm::dvec3 C  = positions[indices[3 * f + 2]];
            glm::dvec3 nC = normals[indices[3 * f + 2]];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            glm::dvec3 As = A + shift_value * n;
            glm::dvec3 Bs = B + shift_value * n;
            glm::dvec3 Cs = C + shift_value * n;

            glm::dvec3 ns = glm::normalize(glm::cross(Bs - As, Cs - As));

            double dpA = glm::dot(ns, nA);
            double dpB = glm::dot(ns, nB);
            double dpC = glm::dot(ns, nC);

            if ((dpA < 0.0) || (dpA < 0.0) || (dpA < 0.0))
            {
                debug_msg("Degeneracy at triangle %u ::", f);
                debug_msg("\tdpA = %f, dpB = %f, dpC = %f", dpA, dpB, dpC);
                debug_msg("\t\tOriginal products :: ");
                debug_msg("\t\t\tdpA = %f, dpB = %f, dpC = %f", glm::dot(n, nA), glm::dot(n, nB), glm::dot(n, nC));
            }
        }        
    }

    struct udf_compute_info_t
    {
        unsigned int max_level;

        glm::dvec3* positions;
        GLuint* indices;
        GLuint triangles;

        std::atomic_uint triangle_index;
        std::atomic_uint* octree;
        std::atomic_uint* udf_texture;
    };

    template<int threads> GLuint udf_compute(int max_level, GLenum texture_unit)
    {
        udf_compute_info_t compute_info;

        compute_info.max_level = max_level;
        compute_info.positions = positions;
        compute_info.indices = indices;
        compute_info.triangles = F;
        compute_info.triangle_index = 0;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        unsigned int diameter = 929887697;        // = 2^28 * 2sqrt(3)

        compute_info.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_info.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i) compute_info.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_info.udf_texture[i] = diameter;

        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::udf_compute_thread_func, &compute_info);

        udf_compute_thread_func(&compute_info);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        {
            computation_thread[thread_id].join();
            debug_msg("Thread #%u joined the main thread.", thread_id);
        }

        /*
        FILE* f = fopen("tex3d.txt", "w");
        for(unsigned int i = 0; i < texture_size; ++i)
        {
            unsigned int texel_value = compute_info.udf_texture[i].load();
            double value = double(texel_value) * INV_INT_SCALE;
            fprintf(f, "texture[%u] = %u (%f)\n", i, texel_value, value);
        }
        fclose(f);
        */

        int* udf_data = (int*) compute_info.udf_texture;

        //===============================================================================================================================================================================================================
        // create 3d texture of the type GL_R32F
        //===============================================================================================================================================================================================================
        GLuint texture_id;
        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_3D, texture_id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, p2, p2, p2);

        GLfloat* texture_data = (GLfloat*) malloc(texture_size * sizeof(GLfloat));

        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = (float) (double(udf_data[p]) * INV_INT_SCALE);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(compute_info.octree);
        free(compute_info.udf_texture);
        free(texture_data);

        return texture_id;
    }


    static void udf_compute_thread_func(udf_compute_info_t* compute_data)
    {
        const unsigned int MAX_LEVEL = 10;
        unsigned int max_level = compute_data->max_level;
        unsigned int p2 = 1 << max_level;
        double p2m1 = double(1 << (max_level - 1));
        double inv_p2 = 1.0 / p2;
        double cube_diameter = 2.0 * constants::sqrt3_d;

        //===============================================================================================================================================================================================================
        // get the index of the triangle this invocation will work on 
        //===============================================================================================================================================================================================================
        unsigned int triangle = compute_data->triangle_index++;

        while (triangle < compute_data->triangles)
        {
            debug_msg("Processing triangle #%u", triangle);
            //===========================================================================================================================================================================================================
            // get the indices and the vertices of the triangle
            //===========================================================================================================================================================================================================
            unsigned int base_index = 3 * triangle;

            unsigned int iA = compute_data->indices[base_index + 0];
            unsigned int iB = compute_data->indices[base_index + 1];
            unsigned int iC = compute_data->indices[base_index + 2];

            glm::dvec3 vA = compute_data->positions[iA];
            glm::dvec3 vB = compute_data->positions[iB];
            glm::dvec3 vC = compute_data->positions[iC];

            glm::dvec3 BA = vB - vA; double dBA = glm::length2(BA);
            glm::dvec3 CB = vC - vB; double dCB = glm::length2(CB);
            glm::dvec3 AC = vA - vC; double dAC = glm::length2(AC);

            //===========================================================================================================================================================================================================
            // calculate triangle diameter
            //===========================================================================================================================================================================================================
            double triangle_diameter = glm::max(glm::max(glm::sqrt(dBA), glm::sqrt(dCB)), glm::sqrt(dAC));
            double inv_dBA = 1.0 / dBA;
            double inv_dCB = 1.0 / dCB;
            double inv_dAC = 1.0 / dAC;

            glm::dvec3 normal = glm::cross(BA, AC);
            double inv_area = 1.0 / glm::length(normal);

            //===========================================================================================================================================================================================================
            // our position in the octree and corresponding index into octree buffer
            //===========================================================================================================================================================================================================
            unsigned int octree_digit[MAX_LEVEL];
            unsigned int node_index = 0;
            octree_digit[0] = 0;
            octree_digit[1] = 0;

            //===========================================================================================================================================================================================================
            // the algorithm starts with jumping from level 0 to level 1 and recursively going down/up
            // when we come back to the level 0, distance octree will be traversed and updated
            //===========================================================================================================================================================================================================
            unsigned int level = 1;
            double scale = 0.5;

            glm::dvec3 node_position[MAX_LEVEL];
            node_position[0] = glm::dvec3(0.0);

            while(level)
            {
                //=======================================================================================================================================================================================================
                // update the current position of the node and calculate the distance from the triangle to it
                //=======================================================================================================================================================================================================
                node_position[level] = node_position[level - 1] + scale * shift[octree_digit[level]];
                glm::dvec3 p = node_position[level];

                glm::dvec3 pA = p - vA;
                glm::dvec3 pB = p - vB;
                glm::dvec3 pC = p - vC;

                double q = glm::sign(glm::dot(glm::cross(BA, normal), pA)) + glm::sign(glm::dot(glm::cross(CB, normal), pB)) + glm::sign(glm::dot(glm::cross(AC, normal), pC));

                double distance_to_node = (q >= 2.0f) ? inv_area * glm::abs(glm::dot(normal, pA)) : 
                    glm::sqrt(
                        glm::min(
                            glm::min(
                                glm::length2(glm::clamp(glm::dot(BA, pA) * inv_dBA, 0.0, 1.0) * BA - pA),
                                glm::length2(glm::clamp(glm::dot(CB, pB) * inv_dCB, 0.0, 1.0) * CB - pB)
                            ), 
                                glm::length2(glm::clamp(glm::dot(AC, pC) * inv_dAC, 0.0, 1.0) * AC - pC)
                        )
                    );

                unsigned int idistance_to_node = (unsigned int)(distance_to_node * INTEGRAL_SCALE);

                unsigned int icurrent_distance = atomic_min(compute_data->octree[node_index], idistance_to_node);
                float current_distance = float(icurrent_distance) * INV_INT_SCALE;
                double node_diameter = (scale - inv_p2) * cube_diameter;

                //=======================================================================================================================================================================================================
                // compare the distance with the distance currently stored in octree
                //=======================================================================================================================================================================================================

                if (distance_to_node >= node_diameter + current_distance + triangle_diameter)
                {
                    //===================================================================================================================================================================================================
                    // current_distance is small enough, the node can be skipped completely
                    // either stay on the same level or go up if octree_digit[level] == 7
                    //===================================================================================================================================================================================================
                    while(octree_digit[level] == 7)
                    {
                        scale += scale;
                        node_index = (node_index >> 3) - 1;
                        level--;
                    }
                    octree_digit[level]++;
                    node_index++;
                }
                else
                {
                    //===================================================================================================================================================================================================
                    // must process this node
                    //===================================================================================================================================================================================================
                    if (level == max_level - 1)
                    {
                        //===============================================================================================================================================================================================
                        // we came to 8 octree leafs, compute the 8 distances and do atomic_min
                        //===============================================================================================================================================================================================
                        glm::dvec3 leaf_node = node_position[compute_data->max_level - 1];
                        for(unsigned int v = 0; v < 8; ++v)
                        {
                            glm::dvec3 leaf_position = leaf_node + inv_p2 * shift[v];
                            glm::ivec3 uvw = glm::ivec3(glm::floor(p2m1 + p2m1 * leaf_position));
                            unsigned int tex3d_index = (uvw.z << (max_level + max_level)) + (uvw.y << max_level) + uvw.x; 

                            glm::dvec3 pA = leaf_position - vA;
                            glm::dvec3 pB = leaf_position - vB;
                            glm::dvec3 pC = leaf_position - vC;

                            double q = glm::sign(glm::dot(glm::cross(BA, normal), pA)) + glm::sign(glm::dot(glm::cross(CB, normal), pB)) + glm::sign(glm::dot(glm::cross(AC, normal), pC));
                            double distance_to_leaf = (q >= 2.0f) ? inv_area * glm::abs(glm::dot(normal, pA)) : 
                                glm::sqrt(
                                    glm::min(
                                        glm::min(
                                            glm::length2(glm::clamp(glm::dot(BA, pA) * inv_dBA, 0.0, 1.0) * BA - pA),
                                            glm::length2(glm::clamp(glm::dot(CB, pB) * inv_dCB, 0.0, 1.0) * CB - pB)
                                        ), 
                                        glm::length2(glm::clamp(glm::dot(AC, pC) * inv_dAC, 0.0, 1.0) * AC - pC)
                                    )
                                );

                            unsigned int idistance_to_leaf = (unsigned int)(distance_to_leaf * INTEGRAL_SCALE);

                            atomic_min(compute_data->udf_texture[tex3d_index], idistance_to_leaf);
                        }

                        //===============================================================================================================================================================================================
                        // come back one/more levels up in the octree
                        //===============================================================================================================================================================================================
                        while(octree_digit[level] == 7)
                        {
                            scale += scale;
                            node_index = (node_index >> 3) - 1;
                            level--;
                        }
                        octree_digit[level]++;
                        node_index++;
                    }   
                    else
                    {
                        //===============================================================================================================================================================================================
                        // go down
                        //===============================================================================================================================================================================================
                        level++;
                        octree_digit[level] = 0;
                        node_index = (node_index + 1) << 3;
                        scale *= 0.5;
                    }
                }
            }

            //===============================================================================================================================================================================================================
            // done ... proceed to next triangle
            //===============================================================================================================================================================================================================
            triangle = compute_data->triangle_index++;
        }
    }
};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    debug_msg("Sizeof(std::atomic_uint) = %u", sizeof(std::atomic_uint));

    int res_x = 1920;
    int res_y = 1080;

    //===================================================================================================================================================================================================================
    // step 0 :: initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Texture 3D generator", 4, 4, 3, res_x, res_y, true);

    glsl_program_t udf_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/udf_compute.cs"));

    //=========================================================================================================     ==========================================================================================================
    // step 2 :: load demon model
    //===================================================================================================================================================================================================================
    sdf_compute_t sdf_compute;
    sdf_compute.load_model("../../../resources/models/vao/trefoil.vao", 0.984375);
    sdf_compute.calculate_statistics();
    debug_msg("\n\n\n!!!!!!!!!!!! Testing degeneracies for original model after normalization. !!!!!!!!!!!!\n\n");



    sdf_compute.compute_area_weighted_normals();
    sdf_compute.test_original_degeneracy();
    debug_msg("\n\n\n!!!!!!!!!!!! Testing degeneracies for area-weighted normals. !!!!!!!!!!!!\n\n");



    sdf_compute.compute_angle_weighted_normals();
    sdf_compute.test_original_degeneracy();
    debug_msg("\n\n\n!!!!!!!!!!!! Testing degeneracies for angle-weighted normals. !!!!!!!!!!!!\n\n");



    //===================================================================================================================================================================================================================
    // 1. index buffer
    //===================================================================================================================================================================================================================
    GLuint ibo_id;
    int F = sdf_compute.F;

    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint) * F, sdf_compute.indices, GL_STATIC_DRAW);

    GLuint ibo_tex_id;
    glGenTextures(1, &ibo_tex_id);
    glBindTexture(GL_TEXTURE_BUFFER, ibo_tex_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, ibo_id);

    glBindImageTexture(0, ibo_tex_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

    //===================================================================================================================================================================================================================
    // 2. vertex positions buffer
    //===================================================================================================================================================================================================================
    int V = sdf_compute.V;
    glm::vec4* positions = (glm::vec4*) malloc(sizeof(glm::vec4) * V);

    for(int v = 0; v < V; ++v)
        positions[v] = glm::vec4(glm::vec3(sdf_compute.positions[v]), 0.0f);

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec4) * V, positions, GL_STATIC_DRAW);

    GLuint vbo_tex_id;
    glGenTextures(1, &vbo_tex_id);
    glBindTexture(GL_TEXTURE_BUFFER, vbo_tex_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, vbo_id);

    glBindImageTexture(1, vbo_tex_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // 3. 8 + 64 + 512 + 4096 + 32768 + 262144 + 2097152 octree buffer;
    //===================================================================================================================================================================================================================
    GLuint octree_size = 8 + 64 + 512 + 4096 + 32768 + 262144 + 2097152;

    GLuint octree_buf_id;
    glGenBuffers(1, &octree_buf_id);
    glBindBuffer(GL_ARRAY_BUFFER, octree_buf_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * octree_size, 0, GL_STATIC_DRAW);

    GLuint octree_tex_id;
    glGenTextures(1, &octree_tex_id);
    glBindTexture(GL_TEXTURE_BUFFER, octree_tex_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, octree_buf_id);

    GLuint diameter = 929887697;        // = 2^28 * 2sqrt(3)
    GLuint zero = 0;
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &diameter);
    glBindImageTexture(2, octree_tex_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

    //===================================================================================================================================================================================================================
    // 4. create the atomic counter buffer with 1 element
    //===================================================================================================================================================================================================================
    GLuint acbo_id;
    glGenBuffers(1, &acbo_id);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo_id);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);
    glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);



    //===================================================================================================================================================================================================================
    // 5. 256x256x256 GL_TEXTURE_3D
    //===================================================================================================================================================================================================================

    GLuint texture_id;
    GLuint size = 256;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, size, size, size);

    glClearTexImage(texture_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &diameter);
    glBindImageTexture(3, texture_id, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);

/*
    udf_compute.enable();
    udf_compute["triangles"] = (unsigned int) F;
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);    

    debug_msg("Computation done ...");


    GLuint pixel_data_size = sizeof(GLuint) * 256 * 256 * 256;
    GLvoid* pixels = (GLvoid*) malloc(pixel_data_size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture_id);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, pixels);

    int w = 112;

    debug_msg("Printing integral texture");
    for (int i0 = w; i0 < 256 - w; ++i0) for (int i1 = w; i1 < 256 - w; ++i1) for (int i2 = w; i2 < 256 - w; ++i2)
    {
        int index = i0 + 256 * i1 + 65536 * i2;
        GLuint value = ((GLuint *) pixels)[index];
        debug_msg("texture[%u, %u, %u] = %u.", i0, i1, i2, value);
    }

    free(pixels);    
*/

    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    sdf_compute.udf_compute<8>(7, GL_TEXTURE0);

    glsl_program_t udf_visualizer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/udf_visualize.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/udf_visualize.fs"));

    udf_visualizer.enable();
    uniform_t uni_uv_pv_matrix = udf_visualizer["projection_view_matrix"];
    udf_visualizer["mask"] = (int) 127;
    udf_visualizer["shift"] = (int) 7;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uni_uv_pv_matrix = projection_view_matrix;

        glDrawArrays(GL_POINTS, 0, 128 * 128 * 128);


        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}