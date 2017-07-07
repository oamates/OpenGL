//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D generator
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

float dot2(const glm::vec3& v) 
    { return glm::dot(v, v); }

float triangle_udf(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    glm::vec3 ba = b - a; glm::vec3 pa = p - a;
    glm::vec3 cb = c - b; glm::vec3 pb = p - b;
    glm::vec3 ac = a - c; glm::vec3 pc = p - c;
    glm::vec3 n = glm::cross(ba, ac);

    float q = glm::sign(glm::dot(glm::cross(ba, n), pa)) + 
              glm::sign(glm::dot(glm::cross(cb, n), pb)) + 
              glm::sign(glm::dot(glm::cross(ac, n), pc));

    if (q >= 2.0f) 
        return glm::sqrt(glm::dot(n, pa) * glm::dot(n, pa) / dot2(n));

    return glm::sqrt(
        glm::min(
            glm::min(
                dot2(ba * glm::clamp(glm::dot(ba, pa) / dot2(ba), 0.0f, 1.0f) - pa),
                dot2(cb * glm::clamp(glm::dot(cb, pb) / dot2(cb), 0.0f, 1.0f) - pb)
            ), 
            dot2(ac * glm::clamp(glm::dot(ac, pc) / dot2(ac), 0.0f, 1.0f) - pc)
        )
    );
}

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


};



//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
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
    // 3. 8 + 64 + 512 + 4096 + 32768 + 262144 + 2097152 + 16777216 octree buffer;
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

    GLuint minus1 = 0xFFFFFFFF;
    GLuint zero = 0;
    glClearBufferData(GL_ARRAY_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
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

    glClearTexImage(texture_id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &minus1);
    glBindImageTexture(3, texture_id, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);


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

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}