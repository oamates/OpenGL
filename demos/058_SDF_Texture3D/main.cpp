//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D generator
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

struct tex3d_header_t
{
    GLenum target;
    GLenum internal_format;
    GLenum format;
    GLenum type;
    glm::ivec3 size;
    GLuint data_size;    
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
        if(header.mode == GL_TRIANGLES)
            debug_msg("Primitive mode :: GL_TRIANGLES");
        else
            debug_msg("Primitive mode :: GL_TRIANGLE_STRIP");

        test_manifoldness();
        
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

    struct uvec2_lex : public glm::uvec2
    {
        uvec2_lex(GLuint a, GLuint b) : glm::uvec2(a, b) {};

        friend bool operator < (const uvec2_lex a, const uvec2_lex b)
        {
            if (a.y < b.y) return true;
            if (a.y > b.y) return false;
            if (a.x < b.x) return true;
            return false;
        };
    };

    void test_manifoldness()
    {
        debug_msg("test_manifoldness :: begin");
        std::map<uvec2_lex, GLuint> edge_count;

        for(GLuint f = 0; f < F; ++f)
        {
            GLuint iA = indices[3 * f + 0];
            GLuint iB = indices[3 * f + 1];
            GLuint iC = indices[3 * f + 2];

            uvec2_lex AB = uvec2_lex(iA, iB);
            uvec2_lex BC = uvec2_lex(iB, iC);
            uvec2_lex CA = uvec2_lex(iC, iA);

            std::map<uvec2_lex, GLuint>::iterator it = edge_count.find(AB);
            if (it != edge_count.end()) edge_count[AB] = 1;
            else
                edge_count[AB]++;

            it = edge_count.find(BC);
            if (it != edge_count.end()) edge_count[BC] = 1;
            else
                edge_count[BC]++;

            it = edge_count.find(CA);
            if (it != edge_count.end()) edge_count[CA] = 1;
            else
                edge_count[CA]++;
        }

        for(std::map<uvec2_lex, GLuint>::const_iterator it0 = edge_count.begin(); it0 != edge_count.end(); ++it0)
        {
            uvec2_lex key = it0->first;
            GLuint value0 = it0->second;
            GLuint value1 = 0;
            uvec2_lex BA = uvec2_lex(key.y, key.x);
            std::map<uvec2_lex, GLuint>::iterator it1 = edge_count.find(BA);
            if (it1 != edge_count.end())
                value1 = it1->second;
            if ((value1 != value0) || (value1 != 1) || (value0 != 1))
            {
                debug_msg("Error !! value0 = %u, value1 = %u, key = (%u, %u)", value0, value1, key.x, key.y);
            }
        }        
        debug_msg("test_manifoldness :: end");
    }

    void compute_area_weighted_normals()
    {
        debug_msg("Averaging face normals with area weight ...");
        memset(normals, 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A = positions[indices[3 * f + 0]];
            glm::dvec3 B = positions[indices[3 * f + 1]];
            glm::dvec3 C = positions[indices[3 * f + 2]];

            glm::dvec3 n = glm::cross(B - A, C - A);

            normals[indices[3 * f + 0]] += n;
            normals[indices[3 * f + 1]] += n;
            normals[indices[3 * f + 2]] += n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
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

    void test_normals()
    {
        debug_msg("\n\n\n\t\t\tTesting normals ... \n\n");
        int errors = 0;
        double max_min_edge = 0.0;

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A  = positions[indices[3 * f + 0]];
            glm::dvec3 nA = normals[indices[3 * f + 0]];
            glm::dvec3 B  = positions[indices[3 * f + 1]];
            glm::dvec3 nB = normals[indices[3 * f + 1]];
            glm::dvec3 C  = positions[indices[3 * f + 2]];
            glm::dvec3 nC = normals[indices[3 * f + 2]];

            glm::dvec3 n = glm::cross(B - A, C - A);
            double area = length(n);
            n /= area;

            double lAB = glm::length(B - A);
            double lBC = glm::length(C - B);
            double lCA = glm::length(A - C);

            double min_edge = glm::min(glm::min(lAB, lBC), lCA);

            double dpA = glm::dot(n, nA);
            double dpB = glm::dot(n, nB);
            double dpC = glm::dot(n, nC);

            if ((dpA < 0.0) || (dpB < 0.0) || (dpC < 0.0))
            {
                debug_msg("Degeneracy at triangle %u :: ", f);
                debug_msg("A = %s", glm::to_string(A).c_str());
                debug_msg("B = %s", glm::to_string(B).c_str());
                debug_msg("C = %s", glm::to_string(C).c_str());
                debug_msg("AB = %f", lAB);
                debug_msg("BC = %f", lBC);
                debug_msg("CA = %f", lCA);
                debug_msg("area = %.17f", area);
                debug_msg("min_edge = %.17f", min_edge);
                debug_msg("\tdpA = %f, dpB = %f, dpC = %f", dpA, dpB, dpC);
                max_min_edge = glm::max(max_min_edge, min_edge);
                errors++;
            }
        }
        debug_msg("Total number of errors :: %u", errors);
        debug_msg("Maximal minimal edge :: %f", max_min_edge);


    }

    void test_degeneracy(double shift_value)
    {
        debug_msg("\n\n\n\t\t\tTesting degeneracies for shift = %f.\n\n", shift_value);
        int errors = 0;

        for(GLuint f = 0; f < F; ++f)
        {
            glm::dvec3 A  = positions[indices[3 * f + 0]];
            glm::dvec3 nA = normals[indices[3 * f + 0]];
            glm::dvec3 B  = positions[indices[3 * f + 1]];
            glm::dvec3 nB = normals[indices[3 * f + 1]];
            glm::dvec3 C  = positions[indices[3 * f + 2]];
            glm::dvec3 nC = normals[indices[3 * f + 2]];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            glm::dvec3 As = A + shift_value * nA;
            glm::dvec3 Bs = B + shift_value * nB;
            glm::dvec3 Cs = C + shift_value * nC;

            glm::dvec3 ns = glm::normalize(glm::cross(Bs - As, Cs - As));

            double dpA = glm::dot(ns, nA);
            double dpB = glm::dot(ns, nB);
            double dpC = glm::dot(ns, nC);

            if ((dpA < 0.0) || (dpB < 0.0) || (dpC < 0.0))
            {
                debug_msg("Degeneracy at triangle %u :: shift_value = %f", f, shift_value);
                debug_msg("\tdpA = %f, dpB = %f, dpC = %f", dpA, dpB, dpC);
                debug_msg("\t\tOriginal products :: ");
                debug_msg("\t\t\tdpA = %f, dpB = %f, dpC = %f", glm::dot(n, nA), glm::dot(n, nB), glm::dot(n, nC));
                errors++;
            }
        }

        debug_msg("Total number of errors :: %u", errors);
    }

    //===================================================================================================================================================================================================================
    // auxiliary structure to be passed to all mesh udf computation threads
    //===================================================================================================================================================================================================================
    struct tri_udf_compute_data_t
    {
        unsigned int max_level;

        glm::dvec3* positions;
        GLuint* indices;
        GLuint triangles;

        std::atomic_uint triangle_index;
        std::atomic_uint* octree;
        std::atomic_uint* udf_texture;
    };

    //===================================================================================================================================================================================================================
    // auxiliary structure to be passed to all point udf computation threads
    //===================================================================================================================================================================================================================
    struct pnt_udf_compute_data_t
    {
        unsigned int max_level;

        glm::dvec3* positions;
        GLuint points;

        std::atomic_uint point_index;
        std::atomic_uint* octree;
        std::atomic_uint* udf_texture;
    };

    //===================================================================================================================================================================================================================
    // computes exact unsigned distance function from a triangle mesh to a discrete lattice in 3D-space
    // the function runs multiple threads executing the main unsigned distance function computation algorithm
    // no modification is necessary to increase the amount of threads -- so let it be equal to the number of processor (logical) cores
    // the implementation will work for unsigned distance textures up to 1024 x 1024 x 1024 dimension
    // this should be enough and will take forever to compute
    //===================================================================================================================================================================================================================
    template<int threads> GLuint tri_udf_compute(int max_level, GLenum texture_unit)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "Atomic uint has larger size than uint. Not good.");

        tri_udf_compute_data_t compute_data;

        compute_data.max_level = max_level;
        compute_data.positions = positions;
        compute_data.indices = indices;
        compute_data.triangles = F;
        compute_data.triangle_index = 0;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        const unsigned int diameter = 929887697;        // = 2^28 * 2sqrt(3)
        //===============================================================================================================================================================================================================
        // to avoid dealing with std::atomic<double>, to gain some speed and to save some space 
        // the distance field values (bounded by the length of the [-1, 1] 3d-cube diagonal) are scaled and result is stored in an integer atomic array
        //===============================================================================================================================================================================================================

        compute_data.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        {
            computation_thread[thread_id].join();
            debug_msg("Thread #%u joined the main thread.", thread_id);
        }

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

        //===============================================================================================================================================================================================================
        // uncomment this line to allocate immutable texture storage, i.e. if you are not planning to change it dimensions and internal type in future
        //===============================================================================================================================================================================================================
        // glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, p2, p2, p2);

        GLfloat* texture_data = (GLfloat*) malloc(texture_size * sizeof(GLfloat));

        int* udf_data = (int*) compute_data.udf_texture;
        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = (float) (double(udf_data[p]) * INV_INT_SCALE);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(compute_data.octree);
        free(compute_data.udf_texture);
        free(texture_data);

        return texture_id;
    }

    //===================================================================================================================================================================================================================
    // computes approximate signed distance function from a triangle mesh to a discrete lattice in 3D-space
    // the function runs multiple threads executing the main unsigned distance function computation algorithm
    // on both external and internal layers of the model 
    // no modification is necessary to increase the amount of threads -- so let it be equal to the number of processor (logical) cores
    // the implementation will work for distance textures up to 1024 x 1024 x 1024 dimension
    //===================================================================================================================================================================================================================
    template<int threads> GLuint tri_sdf_compute(int max_level, GLenum texture_unit, double delta)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "Atomic uint has larger size than uint. Not good.");

        const unsigned int diameter = 929887697;        // = 2^28 * 2sqrt(3)

        tri_udf_compute_data_t compute_data;

        //===============================================================================================================================================================================================================
        // step 1 :: create common compute structure with atomic counter to be used by all threads
        //===============================================================================================================================================================================================================
        compute_data.max_level = max_level;

        glm::dvec3* layer = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V);
        for(unsigned int v = 0; v < V; ++v)
            layer[v] = positions[v] + delta * normals[v];

        compute_data.positions = layer;
        compute_data.indices = indices;
        compute_data.triangles = F;
        compute_data.triangle_index = 0;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        compute_data.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i)  compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        //===============================================================================================================================================================================================================
        // step 2 :: launch threads to compute udf for the external shell of the model
        //===============================================================================================================================================================================================================
        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

        //===============================================================================================================================================================================================================
        // step 3 :: compute udf for the internal shell of the model
        //===============================================================================================================================================================================================================
        for(unsigned int v = 0; v < V; ++v)
            layer[v] = positions[v] - delta * normals[v];

        compute_data.triangle_index = 0;

        int* external_udf = (int*) compute_data.udf_texture;
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

        int* internal_udf = (int*) compute_data.udf_texture;

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

        //===============================================================================================================================================================================================================
        // uncomment this line to allocate immutable texture storage, i.e. if you are not planning to change it dimensions and internal type in future
        //===============================================================================================================================================================================================================
        // glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, p2, p2, p2);

        GLfloat* texture_data = (GLfloat*) malloc(texture_size * sizeof(GLfloat));

        for(unsigned int p = 0; p < texture_size; ++p)
        {
            double sdf;
            if (internal_udf[p] < external_udf[p])
            {
                //=======================================================================================================================================================================================================
                // the point is inside the mesh
                //=======================================================================================================================================================================================================
                sdf = -glm::max(double(external_udf[p]) * INV_INT_SCALE - delta, 0.0);
            }
            else
            {
                //=======================================================================================================================================================================================================
                // the point is outside inside the mesh -- use external field to determine signed distance
                //=======================================================================================================================================================================================================
                sdf = glm::max(double(internal_udf[p]) * INV_INT_SCALE - delta, 0.0);

            }

            texture_data[p] = (float) sdf;
        }
        // afoksha
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_R32F,
            .format = GL_RED,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = texture_size * sizeof(GLfloat)
        };  

        FILE* f = fopen("trefoil.t3d", "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(layer);
        free(compute_data.octree);
        free(external_udf);
        free(internal_udf);
        free(texture_data);

        return texture_id;
    }

    //===================================================================================================================================================================================================================
    // computes exact unsigned distance function from a point cloud to a discrete lattice in 3D-space
    //===================================================================================================================================================================================================================
    template<int threads> GLuint pnt_udf_compute(int max_level, GLenum texture_unit)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "Atomic uint has larger size than uint. Not good.");

        pnt_udf_compute_data_t compute_data;

        compute_data.max_level = max_level;
        compute_data.positions = positions;
        compute_data.point_index = 0;
        compute_data.points = V;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        const unsigned int diameter = 929887697;        // = 2^28 * 2sqrt(3)
        //===============================================================================================================================================================================================================
        // to avoid dealing with std::atomic<double>, to gain some speed and to save some space 
        // the distance field values (bounded by the length of the [-1, 1] 3d-cube diagonal) are scaled and result is stored in an integer atomic array
        //===============================================================================================================================================================================================================

        compute_data.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::pnt_udf_compute_thread, &compute_data);

        pnt_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        {
            computation_thread[thread_id].join();
            debug_msg("Thread #%u joined the main thread.", thread_id);
        }

        //===============================================================================================================================================================================================================
        // create 3d texture of the type GL_R32F
        //===============================================================================================================================================================================================================
        GLuint texture_id;
        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_3D, texture_id);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //===============================================================================================================================================================================================================
        // uncomment this line to allocate immutable texture storage, i.e. if you are not planning to change it dimensions and internal type in future
        //===============================================================================================================================================================================================================
        // glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, p2, p2, p2);

        GLfloat* texture_data = (GLfloat*) malloc(texture_size * sizeof(GLfloat));

        int* udf_data = (int*) compute_data.udf_texture;
        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = (float) (double(udf_data[p]) * INV_INT_SCALE);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(compute_data.octree);
        free(compute_data.udf_texture);
        free(texture_data);

        return texture_id;
    }

    //===================================================================================================================================================================================================================
    // computes (approximate) signed distance function from a triangle mesh to a discrete lattice in 3D-space
    // the function generates two point clouds (external and internal), computes two unsigned distance fields and by simple analysis 
    // decides whether the given lattice point lies inside or outside of the distance mesh and computes the distance
    //===================================================================================================================================================================================================================
    template<int threads> GLuint pnt_sdf_compute(int max_level, GLenum texture_unit)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "Atomic uint has larger size than uint. Not good.");

        const unsigned int diameter = 929887697;        // = 2^28 * 2sqrt(3)
        const double delta = 0.0078125 * 2;

        pnt_udf_compute_data_t compute_data;

        //===============================================================================================================================================================================================================
        // step 1 :: compute udf for external point cloud
        //===============================================================================================================================================================================================================
        compute_data.max_level = max_level;

        glm::dvec3* cloud = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V);
        for(unsigned int v = 0; v < V; ++v)
            cloud[v] = positions[v] + delta * normals[v];

        compute_data.positions = cloud;
        compute_data.point_index = 0;
        compute_data.points = V;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        compute_data.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i)  compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        debug_msg("Computing distance to the external cloud :: ");
        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::pnt_udf_compute_thread, &compute_data);

        pnt_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        {
            computation_thread[thread_id].join();
            debug_msg("Thread #%u joined the main thread.", thread_id);
        }

        //===============================================================================================================================================================================================================
        // step 2 :: compute udf for internal point cloud
        //===============================================================================================================================================================================================================
        for(unsigned int v = 0; v < V; ++v)
            cloud[v] = positions[v] - delta * normals[v];

        compute_data.point_index = 0;

        int* external_udf = (int*) compute_data.udf_texture;
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        debug_msg("Computing distance to the internal cloud :: ");
        //std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::pnt_udf_compute_thread, &compute_data);

        pnt_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
        {
            computation_thread[thread_id].join();
            debug_msg("Thread #%u joined the main thread.", thread_id);
        }

        int* internal_udf = (int*) compute_data.udf_texture;

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

        //===============================================================================================================================================================================================================
        // uncomment this line to allocate immutable texture storage, i.e. if you are not planning to change it dimensions and internal type in future
        //===============================================================================================================================================================================================================
        // glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, p2, p2, p2);

        GLfloat* texture_data = (GLfloat*) malloc(texture_size * sizeof(GLfloat));

        for(unsigned int p = 0; p < texture_size; ++p)
        {
            double sdf;
            if (internal_udf[p] < external_udf[p])
            {
                //=======================================================================================================================================================================================================
                // the point is inside the mesh
                //=======================================================================================================================================================================================================
                sdf = -glm::max(double(external_udf[p]) * INV_INT_SCALE - delta, 0.0);
            }
            else
            {
                //=======================================================================================================================================================================================================
                // the point is outside inside the mesh -- use external field to determine signed distance
                //=======================================================================================================================================================================================================
                sdf = glm::max(double(internal_udf[p]) * INV_INT_SCALE - delta, 0.0);

            }

            texture_data[p] = (float) sdf;
        }

        //=======================================================================================================================================================================================================
        // store texture to file
        //=======================================================================================================================================================================================================
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_R32F,
            .format = GL_RED,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = texture_size * sizeof(GLfloat)
        };  

        FILE* f = fopen("trefoil.t3d", "wb");
        fwrite(&header, sizeof(tex3d_header_t), 1, f);
        fwrite(texture_data, header.data_size, 1, f);
        fclose(f);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(compute_data.octree);
        free(external_udf);
        free(internal_udf);
        free(texture_data);

        return texture_id;
    }

    //===================================================================================================================================================================================================================
    // implementation of the main algorithm for unsigned distance function computation ::
    // the function takes a triangle and traverses (simultaneously modifying it) distance octree avoiding octree branches
    // that distances to this particular triangle will certainly not modify and updating releveant ones using atomic minimum operation 
    //===================================================================================================================================================================================================================
    static void tri_udf_compute_thread(tri_udf_compute_data_t* compute_data)
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
                        glm::dvec3 leaf_node = node_position[max_level - 1];
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
                        // stay on the same level or go up if the last digit (=7) on the current level has been processed
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

    //===================================================================================================================================================================================================================
    // implementation of the main algorithm for unsigned distance function computation ::
    // the function takes a triangle and traverses (simultaneously modifying it) distance octree avoiding octree branches
    // that distances to this particular triangle will certainly not modify and updating releveant ones using atomic minimum operation 
    //===================================================================================================================================================================================================================
    static void pnt_udf_compute_thread(pnt_udf_compute_data_t* compute_data)
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
        unsigned int point = compute_data->point_index++;

        while (point < compute_data->points)
        {
            debug_msg("Processing point #%u", point);
            //===========================================================================================================================================================================================================
            // get the position of the point to work with
            //===========================================================================================================================================================================================================
            glm::dvec3 position = compute_data->positions[point];

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

                double distance_to_node = glm::length(position - node_position[level]);
                unsigned int idistance_to_node = (unsigned int)(distance_to_node * INTEGRAL_SCALE);

                unsigned int icurrent_distance = atomic_min(compute_data->octree[node_index], idistance_to_node);
                float current_distance = float(icurrent_distance) * INV_INT_SCALE;
                double node_diameter = (scale - inv_p2) * cube_diameter;

                //=======================================================================================================================================================================================================
                // compare the distance with the distance currently stored in octree
                //=======================================================================================================================================================================================================

                if (distance_to_node >= node_diameter + current_distance)
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
                            double distance_to_leaf = glm::length(position - leaf_node);
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
            point = compute_data->point_index++;
        }
    }

};

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    int res_x = 1920;
    int res_y = 1080;

    int max_level = 8;
    int p2 = 1 << max_level;
    int p2m1 = 1 << (max_level - 1);
    double inv_p2m1 = 1.0 / p2m1;
    double texel_size = inv_p2m1;
    int octree_size = 0;
    int mip_size = 8;

    GLuint diameter = 929887697;        // = 2^28 * 2sqrt(3)
    GLuint zero = 0;

    for(int i = 0; i < max_level - 1; ++i)
    {
        octree_size += mip_size;
        mip_size <<= 3;
    }    

    //===================================================================================================================================================================================================================
    // step 0 :: initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("SDF Texture 3D generator", 4, 4, 3, res_x, res_y, true);

    glsl_program_t pnt_udf_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/pnt_udf_compute.cs"));
    glsl_program_t tri_udf_compute(glsl_shader_t(GL_COMPUTE_SHADER, "glsl/tri_udf_compute.cs"));

    //===================================================================================================================================================================================================================
    // step 1 :: load demon model
    //===================================================================================================================================================================================================================
    sdf_compute_t sdf_compute;
    sdf_compute.load_model("../../../resources/models/vao/trefoil.vao", 1.0 - 3 * texel_size);
    sdf_compute.calculate_statistics();
    //sdf_compute.compute_area_weighted_normals();
    sdf_compute.compute_angle_weighted_normals();
    sdf_compute.test_normals();

    //sdf_compute.test_degeneracy(0.0 /* 0.0625 * texel_size */);
    //sdf_compute.test_degeneracy(-0.0625 * texel_size);

    int F = sdf_compute.F;
    int V = sdf_compute.V;

    //===================================================================================================================================================================================================================
    // 1. index buffer
    //===================================================================================================================================================================================================================
/*
    GLuint ibo_id;

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
    glm::vec4* positions = (glm::vec4*) malloc(sizeof(glm::vec4) * V);

    for(int v = 0; v < V; ++v)
        positions[v] = glm::vec4(glm::vec3(sdf_compute.positions[v]), 0.0f);

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * V, positions, GL_STATIC_DRAW);

    free(positions);

    GLuint vbo_tex_id;
    glGenTextures(1, &vbo_tex_id);
    glBindTexture(GL_TEXTURE_BUFFER, vbo_tex_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, vbo_id);

    glBindImageTexture(1, vbo_tex_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    //===================================================================================================================================================================================================================
    // 3. create octree buffer with texture buffer access
    //===================================================================================================================================================================================================================
    GLuint octree_buf_id;
    glGenBuffers(1, &octree_buf_id);
    glBindBuffer(GL_TEXTURE_BUFFER, octree_buf_id);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(GLuint) * octree_size, 0, GL_DYNAMIC_COPY);

    GLuint octree_tex_id;
    glGenTextures(1, &octree_tex_id);
    glBindTexture(GL_TEXTURE_BUFFER, octree_tex_id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, octree_buf_id);

    glClearBufferData(GL_TEXTURE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &diameter);
    glBindImageTexture(2, octree_tex_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    //===================================================================================================================================================================================================================
    // 4. create atomic counter buffer with 1 element
    //===================================================================================================================================================================================================================
    GLuint acbo_id;
    glGenBuffers(1, &acbo_id);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo_id);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_COPY);
    glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo_id);

    //===================================================================================================================================================================================================================
    // 5. GL_TEXTURE_3D
    //===================================================================================================================================================================================================================
    GLuint texture_id;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, p2, p2, p2);
    glClearTexImage(texture_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &diameter);


    glBindImageTexture(3, texture_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    tri_udf_compute.enable();
    tri_udf_compute["triangles"] = (int) F;
    tri_udf_compute["level"] = (int) max_level;

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
*/
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);



    sdf_compute.tri_sdf_compute<8>(max_level, GL_TEXTURE0, texel_size);


    glsl_program_t udf_visualizer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/udf_visualize.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/udf_visualize.fs"));

    udf_visualizer.enable();
    uniform_t uni_uv_pv_matrix = udf_visualizer["projection_view_matrix"];
    udf_visualizer["mask"] = (int) ((1 << max_level) - 1);
    udf_visualizer["shift"] = (int) max_level;
    udf_visualizer["udf_tex"] = 0;

    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        uni_uv_pv_matrix = projection_view_matrix;

        glDrawArrays(GL_POINTS, 0, p2 * p2 * p2);


        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}