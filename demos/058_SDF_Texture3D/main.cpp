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
#include "hqs_model.hpp"
#include "he_manifold.hpp"
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

    hqs_model_t model("../../../resources/manifolds/demon.obj");
    model.normalize(1.0 - 4.0 * texel_size);
    model.sort_faces_by_area();

    he_manifold_t<GLuint> manifold(model.faces.data(), model.F, model.positions.data(), model.V);
    model.normals.resize(model.V);
    manifold.normals = model.normals.data();
    manifold.calculate_angle_weighted_normals();

    vao_t model_ori_vao = model.create_vao();

    sdf_compute_t<GLuint> sdf_compute(manifold.faces, manifold.F, manifold.positions, manifold.V);
    sdf_compute.normals = manifold.normals;

    manifold.export_vao("demon.vao", true);


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

    sdf_compute.tri_sdf_compute<8>(max_level, GL_TEXTURE0, 0.125 * texel_size, "demon_rg.sdf");

    glsl_program_t udf_visualizer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/udf_visualize.vs"),
                                  glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/udf_visualize.fs"));

    udf_visualizer.enable();
    uniform_t uni_uv_pv_matrix = udf_visualizer["projection_view_matrix"];
    udf_visualizer["mask"] = (int) ((1 << max_level) - 1);
    udf_visualizer["shift"] = (int) max_level;
    udf_visualizer["udf_tex"] = 0;


    //texture3d_t demon_sdf(GL_TEXTURE0, "../../../resources/sdf/demon.sdf");

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