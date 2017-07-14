//========================================================================================================================================================================================================================
// DEMO 045 : Mesh Shells visualizer
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

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
#include "shader.hpp"
#include "camera.hpp"
#include "vertex.hpp"
#include "momenta.hpp"
#include "edgeface.hpp"

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
        const float min_speed = 0.03125;

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
                //line >> index.x >> index.y >> index.z;
                index.x = read_position_index(line);
                index.y = read_position_index(line);
                index.z = read_position_index(line);
                indices.push_back(index);
                continue;
            }
        }
        V = positions.size();
        F = indices.size();
        debug_msg("Done :: #vertices = %u. triangles = %u.", V, F);
    }

    GLuint read_position_index(std::stringstream& is)                
    {           
        GLuint p, n, t;
        is >> p;
        p--;                                                                            
        int slash = is.get();
        if (slash != '/') return p;
        if (is.peek() != '/')
        {
            is >> t;
            slash = is.get(); 
            if (slash != '/') return p;
        }
        else
            is.get();
        is >> n;
        return p;
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
        debug_msg("Averaging face normals with area weight ...");
        normals.resize(V);
        memset(normals.data(), 0, sizeof(glm::dvec3) * V);

        for(GLuint f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = indices[f];
            debug_msg("processing %s", glm::to_string(triangle).c_str());
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

            double qA = glm::acos(glm::sqrt(0.5 + 0.5 * dot(CA, AB)));
            double qB = glm::acos(glm::sqrt(0.5 + 0.5 * dot(AB, BC)));
            double qC = glm::acos(glm::sqrt(0.5 + 0.5 * dot(BC, CA)));


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

            if ((dpA < 0.25) || (dpB < 0.25) || (dpC < 0.25))
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

template<typename index_t> struct halfedge_t
{
    index_t a, b;
    uint32_t face;
    uint32_t opposite;
    uint32_t bc;
    uint32_t ca;

    halfedge_t(index_t a, index_t b, uint32_t face)
        : a(a), b(b), face(face), opposite(-1), bc(-1), ca(-1)
    {}
};

template<typename index_t> struct he_manifold_t
{
    //===================================================================================================================================================================================================================
    // the number of faces and the pointer to faces array
    //===================================================================================================================================================================================================================
    GLuint F;
    glm::tvec3<index_t>* faces;

    //===================================================================================================================================================================================================================
    // pointer to geometric position data, not a required field as many algorithms
    // may need only topological (index) data, will be 0 in this case
    //===================================================================================================================================================================================================================
    GLuint V;
    glm::dvec3* positions;

    //===================================================================================================================================================================================================================
    // main data structure for fast mesh traversing
    //===================================================================================================================================================================================================================
    GLuint E;
    halfedge_t<index_t>* edges;

    he_manifold_t(glm::tvec3<index_t>* faces, uint32_t F, glm::dvec3* positions, uint32_t V)
        : faces(faces), F(F), positions(positions), V(V)
    {
        E = F + F + F;
        edges = (halfedge_t<index_t>*) malloc(E * sizeof(halfedge_t<index_t>));

        //================================================================================================================================================================================================================
        // create half-edge structure array from the input data                                                                                                                                                                    
        //================================================================================================================================================================================================================
        for (uint32_t index = 0, f = 0; f < F; ++f)
        {
            index_t a = faces[f].x,
                    b = faces[f].y,
                    c = faces[f].z;

            uint32_t i_ab = index++;
            uint32_t i_bc = index++;
            uint32_t i_ca = index++;

            edges[i_ab].a = a;
            edges[i_ab].b = b;
            edges[i_ab].face = f;
            edges[i_ab].bc = i_bc;
            edges[i_ab].ca = i_ca;

            edges[i_bc].a = b;
            edges[i_bc].b = c;
            edges[i_bc].face = f;
            edges[i_bc].bc = i_ca;
            edges[i_bc].ca = i_ab;

            edges[i_ca].a = c;
            edges[i_ca].b = a;
            edges[i_ca].face = f;
            edges[i_ca].bc = i_ab;
            edges[i_ca].ca = i_bc;
        }

        //================================================================================================================================================================================================================
        // quick sort edges lexicographically
        //================================================================================================================================================================================================================
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

        struct
        {
            uint32_t l;                                                                             // left index of the sub-array that needs to be sorted
            uint32_t r;                                                                             // right index of the sub-array to sort
        } _stack[STACK_SIZE];

        int sp = 0;                                                                                 // stack pointer, stack grows up not down
        _stack[sp].l = 0;
        _stack[sp].r = E - 1;
                                                                                                                                                                                                                          
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            uint32_t l = _stack[sp].l;                                                                                                                                                                                
            uint32_t r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                uint32_t i = l;                                                                                                                                                                                       
                uint32_t j = r;                                                                                                                                                                                       
                uint32_t m = i + (j - i) / 2;                                                                                                                                                                         
                index_t a = edges[m].a;                                                                                                                                                                                             
                index_t b = edges[m].b;                                                                                                                                                                                             
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while ((edges[i].b < b) || ((edges[i].b == b) && (edges[i].a < a))) i++;        // lexicographic compare and proceed forward if less                                                                      
                    while ((edges[j].b > b) || ((edges[j].b == b) && (edges[j].a > a))) j--;        // lexicographic compare and proceed backward if less                                                                     
                                                                                                                                                                                                                          
                    if (i <= j)                                                                                                                                                                                               
                    {
                        std::swap(edges[i].a, edges[j].a);
                        std::swap(edges[i].b, edges[j].b);
                        std::swap(edges[i].face, edges[j].face);

                        edges[edges[i].bc].ca = j;
                        edges[edges[i].ca].bc = j;
                        edges[edges[j].bc].ca = i;
                        edges[edges[j].ca].bc = i;

                        i++;
                        j--;
                    }
                }
                while (i <= j);

                if (j - l < r - i)                                                                  // push the larger interval to stack and continue sorting the smaller one                                                      
                {
                    if (i < r)
                    {
                        ++sp;
                        _stack[sp].l = i;
                        _stack[sp].r = r;
                    }                                                                                                                                                                                                         
                    r = j;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
                else                                                                                                                                                                                                          
                {                                                                                                                                                                                                             
                    if (l < j)                                                                                                                                                                                                
                    {                                                                                                                                                                                                         
                        ++sp;                                                                                                                                                                                                 
                        _stack[sp].l = l;                                                                                                                                                                                     
                        _stack[sp].r = j;                                                                                                                                                                                     
                    }                                                                                                                                                                                                         
                    l = i;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
            while(l < r);                                                                                                                                                                                                     
        }                                                                                                                                                                                                                     
        while (sp >= 0);

        //============================================================================================================================================================================================================
        // fill opposite edge indices, -1 will indicate boundary edges                                                                                                                                                        
        //============================================================================================================================================================================================================
        for (uint32_t e = 0; e < E; ++e)
        {
            index_t a = edges[e].a;
            index_t b = edges[e].b;

            if (a < b)
            {
                uint32_t opposite = find_edge(b, a);
                edges[e].opposite = opposite;

                if (opposite != -1)
                    edges[opposite].opposite = e;
            }
        }
    }

    //================================================================================================================================================================================================================
    // finds edge [a,b] and returns its index
    // if edge does not exist returns -1
    //================================================================================================================================================================================================================
    uint32_t find_edge(index_t a, index_t b)
    {
        int32_t l = 0;
        int32_t r = E - 1;
        while (l <= r)
        {
            int32_t m = (r + l) / 2;

            if (edges[m].b < b) { l = m + 1; continue; }
            if (edges[m].b > b) { r = m - 1; continue; }
            if (edges[m].a < a) { l = m + 1; continue; }
            if (edges[m].a > a) { r = m - 1; continue; }

            //========================================================================================================================================================================================================
            // edge found, m is its index
            //========================================================================================================================================================================================================
            return m;
        }
        return -1;
    }

    //================================================================================================================================================================================================================
    // replaces triangle pair ABC + ADB with pair ADC + BCD
    //================================================================================================================================================================================================================
    uint32_t flip_edge(uint32_t e)
    {
        uint32_t o = edges[e].opposite;

        index_t a = edges[e].a;
        index_t b = edges[e].b;

        index_t c = edges[edges[e].bc].b;
        index_t d = edges[edges[o].bc].b;

        faces[edges[e].face] = glm::tvec3<index_t>(a, d, c);
        faces[edges[o].face] = glm::tvec3<index_t>(b, c, d);

        int32_t l = 0;
        int32_t r = E - 1;
        while (l <= r)
        {
            int32_t m = (r + l) / 2;

            if (edges[m].b < b) { l = m + 1; continue; }
            if (edges[m].b > b) { r = m - 1; continue; }
            if (edges[m].a < a) { l = m + 1; continue; }
            if (edges[m].a > a) { r = m - 1; continue; }

            //========================================================================================================================================================================================================
            // edge found, m is its index
            //========================================================================================================================================================================================================
            return m;
        }
        return -1;
    }

    double angle_defect(double cos_A, double cos_B, double cos_C)
    {
        return glm::abs(cos_A - 0.5) + glm::abs(cos_B - 0.5) + glm::abs(cos_C - 0.5);
    }

    void flip_edges(double threshold)
    {
        for(GLuint e = 0; e < E; ++e)
        {
            index_t a = edges[e].a;
            index_t b = edges[e].b;
            index_t c = edges[edges[e].bc].b;
            index_t d = edges[edges[o].bc].b;

            glm::dvec3 A = positions[a];
            glm::dvec3 B = positions[b];
            glm::dvec3 C = positions[c];
            glm::dvec3 D = positions[d];


            glm::dvec3 AB = glm::normalize(B - A);
            glm::dvec3 BC = glm::normalize(C - B);
            glm::dvec3 CA = glm::normalize(A - C);

            glm::dvec3 AD = glm::normalize(D - A);
            glm::dvec3 DB = glm::normalize(B - D);

            glm::dvec3 DC = glm::normalize(C - D);

            //========================================================================================================================================================================================================
            // triangle ABC
            //========================================================================================================================================================================================================
            double cos_CAB = glm::dot(CA, AB);
            double cos_ABC = glm::dot(AB, BC);
            double cos_BCA = glm::dot(BC, CA);
            double degeneracy_ABC = angle_defect(cos_CAB, cos_ABC, cos_BCA);

            //========================================================================================================================================================================================================
            // triangle ADB
            //========================================================================================================================================================================================================
            double cos_DBA = -glm::dot(DB, AB);
            double cos_ADB =  glm::dot(AD, DB);
            double cos_BAD = -glm::dot(AB, AD);
            double degeneracy_ADB = angle_defect(cos_DBA, cos_ADB, cos_BAD);

            //========================================================================================================================================================================================================
            // triangle ADC
            //========================================================================================================================================================================================================
            double cos_DCA = glm::dot(DC, CA);
            double cos_ADC = glm::dot(AD, DC);
            double cos_CAD = glm::dot(CA, AD);
            double degeneracy_ADC = angle_defect(cos_DCA, cos_ADC, cos_CAD);

            //========================================================================================================================================================================================================
            // triangle DBC
            //========================================================================================================================================================================================================
            double cos_BCD = -glm::dot(BC, DC);
            double cos_DBC =  glm::dot(DB, BC);
            double cos_CDB = -glm::dot(DC, DB);
            double degeneracy_ADC = angle_defect(cos_DCA, cos_ADC, cos_CAD);



        }    
    }    

    ~he_manifold_t()
        { free(edges); }
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
    // load model and build it edge-face structure
    //===================================================================================================================================================================================================================
    hqs_model_t model("../../../resources/manifolds/demon.obj");

    he_manifold_t<GLuint> manifold_struct(model.indices.data(), model.F, model.positions.data(), model.V);

                                                         
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