//========================================================================================================================================================================================================================
// DEMO 045 : Mesh Shells visualizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <atomic>
#include <thread>
#include <map>
#include <sstream>
#include <fstream>
 
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
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.001f);
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

    void on_scroll(double xoffset, double yoffset) override
    {
        const float max_speed = 8.0;
        const float min_speed = 0.125;

        float factor = exp(0.125 * yoffset);
        float speed = factor * camera.linear_speed;

        if ((speed <= max_speed) && (speed >= min_speed))
            camera.linear_speed = speed;
    }
};




struct hqs_model_t
{
    GLuint V;
    GLuint F;

    std::vector<glm::dvec3> positions;
    std::vector<glm::uvec3> indices;
    std::vector<glm::dvec3> normals;

    double bbox_max;

    hqs_model_t(const char* filename)
    {
        std::ifstream input_stream(filename);
        for (std::string buffer; getline(input_stream, buffer);)
        {        
            if (buffer.empty()) continue;
            std::stringstream line(buffer);
            std::string token;
            line >> token;
                                                                            
            if (token == "v")
            {
                glm::dvec3 position;
                line >> position.x >> position.y >> position.z;
                positions.push_back(position);
                continue;
            }

            if (token == "f")
            {
                glm::uvec3 index;
                line >> index.x >> index.y >> index.z;
                indices.push_back(index - glm::uvec3(1));
                continue;
            }
        }
        V = positions.size();
        F = indices.size();
        debug_msg("Done :: #vertices = %u. triangles = %u.", V, F);
    }

    void normalize(double bbox_max)
    {
        hqs_model_t::bbox_max = bbox_max;
        debug_msg("Normalizing the model :: bbox_max = %f.", bbox_max);

        glm::dvec3 mass_center;
        glm::dmat3 covariance_matrix;

        momenta::calculate(positions, mass_center, covariance_matrix);
        debug_msg("\tMass center = %s", glm::to_string(mass_center).c_str());
        debug_msg("\tCovariance matrix = %s", glm::to_string(covariance_matrix).c_str());

        glm::dquat q = diagonalizer(covariance_matrix);
        glm::dmat3 Q = mat3_cast(q);
        glm::dmat3 Qt = glm::transpose(Q);

        debug_msg("\tDiagonalizer = %s", glm::to_string(Q).c_str());

        glm::dvec3 bbox = glm::dvec3(0.0);

        for (GLuint v = 0; v < V; ++v)
        {
            positions[v] = Q * (positions[v] - mass_center);
            bbox = glm::max(bbox, glm::abs(positions[v]));
        }

        double max_bbox = glm::max(bbox.x, glm::max(bbox.y, bbox.z));
        double scale = bbox_max / max_bbox;

        debug_msg("\tModel BBox = %s", glm::to_string(bbox).c_str());
        debug_msg("\tBBox_max = %f. Maximum = %f. Scale = %f. Scaling ...", bbox_max, max_bbox, scale);

        covariance_matrix = (scale * scale) * (Q * covariance_matrix * Qt);
        debug_msg("\tCovariance matrix after normalization = %s", glm::to_string(covariance_matrix).c_str());

        bbox = glm::dvec3(0.0);
        for (GLuint v = 0; v < V; ++v)
        {
            positions[v] = scale * positions[v];
            bbox = glm::max(bbox, glm::abs(positions[v]));
        }
    }

    void calculate_area_weighted_normals()
    {
        debug_msg("Averaging face normals with angular weight ...");
        normals.resize(V);
        memset(normals.data(), 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = indices[f];
            glm::dvec3 A = positions[triangle.x];
            glm::dvec3 B = positions[triangle.y];
            glm::dvec3 C = positions[triangle.z];
            glm::dvec3 n = glm::cross(B - A, C - A);

            normals[triangle.x] += n;
            normals[triangle.y] += n;
            normals[triangle.z] += n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
    }

    void calculate_angle_weighted_normals()
    {
        debug_msg("Averaging face normals with angular weight ...");
        normals.resize(V);
        memset(normals.data(), 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = indices[f];
            glm::dvec3 A = positions[triangle.x];
            glm::dvec3 B = positions[triangle.y];
            glm::dvec3 C = positions[triangle.z];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            glm::dvec3 AB = glm::normalize(B - A);
            glm::dvec3 BC = glm::normalize(C - B);
            glm::dvec3 CA = glm::normalize(A - C);

            double qA = glm::sqrt(1.0 + dot(CA, AB));
            double qB = glm::sqrt(1.0 + dot(AB, BC));
            double qC = glm::sqrt(1.0 + dot(BC, CA));

            normals[triangle.x] += qA * n;
            normals[triangle.y] += qB * n;
            normals[triangle.z] += qC * n;
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
            glm::uvec3 triangle = indices[f];
            glm::dvec3 A = positions[triangle.x];
            glm::dvec3 B = positions[triangle.y];
            glm::dvec3 C = positions[triangle.z];

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


    struct uvec2_lex : public glm::uvec2
    {
        uvec2_lex(GLuint a, GLuint b) : glm::uvec2(a, b) {};

        friend bool operator < (const uvec2_lex a, const uvec2_lex b)
        {
            if (a.y < b.y) return true;
            if (a.y > b.y) return false;
            if (a.x < b.x) return true;
            return false;
        }
    };

    void test_manifoldness()
    {
        debug_msg("test_manifoldness :: begin");
        std::map<uvec2_lex, GLuint> edge_count;

        for(GLuint f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = indices[f];

            GLuint iA = triangle.x;
            GLuint iB = triangle.y;
            GLuint iC = triangle.z;

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
                debug_msg("Error !! value0 = %u, value1 = %u, key = (%u, %u)", value0, value1, key.x + 1, key.y + 1);
            }
        }        
        debug_msg("test_manifoldness :: end");
    }    

    void test_normals()
    {
        debug_msg("\n\n\n\t\t\tTesting normals ... \n\n");
        int errors = 0;
        double max_min_edge = 0.0;

        for(GLuint f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = indices[f];

            glm::dvec3 A  = positions[triangle.x];
            glm::dvec3 nA = normals[triangle.x];
            glm::dvec3 B  = positions[triangle.y];
            glm::dvec3 nB = normals[triangle.y];
            glm::dvec3 C  = positions[triangle.z];
            glm::dvec3 nC = normals[triangle.z];

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
    vao_t create_vao()
    {
        vertex_pn_t* vertices = (vertex_pn_t*) malloc(V * sizeof(vertex_pn_t));

        for (GLuint v = 0; v < V; ++v)
        {
            vertices[v].position = glm::vec3(positions[v]);
            vertices[v].normal = glm::vec3(normals[v]);
        }

        vao_t model_vao = vao_t(GL_TRIANGLES, vertices, V, (GLuint*) indices.data(), 3 * F);
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
                                    glsl_shader_t(GL_GEOMETRY_SHADER, "glsl/model_shell.gs"),
                                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/model_shell.fs"));

    shell_visualizer.enable();
    uniform_t uni_pv_matrix = shell_visualizer["projection_view_matrix"];
    uniform_t uni_camera_ws = shell_visualizer["camera_ws"];
    uniform_t uni_light_ws  = shell_visualizer["light_ws"];

    //===================================================================================================================================================================================================================
    // load demon model
    //===================================================================================================================================================================================================================
    hqs_model_t demon("../../../resources/models/obj/demon.obj");
    demon.normalize(1.0);
    demon.calculate_angle_weighted_normals();
    demon.test_manifoldness();
    demon.test_normals();

    vao_t demon_vao = demon.create_vao();

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
        demon_vao.render();

        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}