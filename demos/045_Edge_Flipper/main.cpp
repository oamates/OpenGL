//========================================================================================================================================================================================================================
// DEMO 045 : Mesh Edge Flipper
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

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
#include "vertex.hpp"
#include "momenta.hpp"
#include "vao.hpp"

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





template<typename index_t> struct halfedge_t
{
    index_t a, b;
    uint32_t face;

    uint32_t next;
    uint32_t opposite;

    halfedge_t(index_t a, index_t b, uint32_t face)
        : a(a), b(b), face(face), next(-1), opposite(-1)
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
    glm::dvec3* normals;

    //===================================================================================================================================================================================================================
    // main data structure for fast mesh traversing
    //===================================================================================================================================================================================================================
    GLuint E;
    halfedge_t<index_t>* edges;


    double bbox_max;


    he_manifold_t(glm::tvec3<index_t>* faces, uint32_t F, glm::dvec3* positions, uint32_t V)
        : faces(faces), F(F), positions(positions), normals(0), V(V) 
    {
        init();
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

    void init()
    {
        E = F + F + F;
        edges = (halfedge_t<index_t>*) malloc(E * sizeof(halfedge_t<index_t>));

        //================================================================================================================================================================================================================
        // create half-edge structure array from the input data                                                                                                                                                                    
        //================================================================================================================================================================================================================
        for (uint32_t e = 0, f = 0; f < F; ++f)
        {
            debug_msg("Creating halfedge #%u", e);


            index_t a = faces[f].x,
                    b = faces[f].y,
                    c = faces[f].z;

            uint32_t i_ab = e++;
            uint32_t i_bc = e++;
            uint32_t i_ca = e++;

            edges[i_ab].a = a;
            edges[i_ab].b = b;
            edges[i_ab].face = f;
            edges[i_ab].next = i_bc;

            edges[i_bc].a = b;
            edges[i_bc].b = c;
            edges[i_bc].face = f;
            edges[i_bc].next = i_ca;

            edges[i_ca].a = c;
            edges[i_ca].b = a;
            edges[i_ca].face = f;
            edges[i_ca].next = i_ab;


        }

        debug_msg("Array filled\n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next);
        }


        //================================================================================================================================================================================================================
        // quick sort edges lexicographically
        //================================================================================================================================================================================================================
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

        struct
        {
            int32_t l;                                                                              // left index of the sub-array that needs to be sorted
            int32_t r;                                                                              // right index of the sub-array to sort
        } _stack[STACK_SIZE];

        int sp = 0;                                                                                 // stack pointer, stack grows up not down
        _stack[sp].l = 0;
        _stack[sp].r = E - 1;

        do
        {
            int32_t l = _stack[sp].l;
            int32_t r = _stack[sp].r;
            --sp;
            do
            {
                int32_t i = l;
                int32_t j = r;
                int32_t m = (i + j) / 2;
                index_t a = edges[m].a;
                index_t b = edges[m].b;
                do
                {
                    while ((edges[i].b < b) || ((edges[i].b == b) && (edges[i].a < a))) i++;        // lexicographic compare and proceed forward if less
                    while ((edges[j].b > b) || ((edges[j].b == b) && (edges[j].a > a))) j--;        // lexicographic compare and proceed backward if greater

                    if (i <= j)
                    {
                        std::swap(edges[i].a, edges[j].a);
                        std::swap(edges[i].b, edges[j].b);
                        std::swap(edges[i].face, edges[j].face);

                        uint32_t i_prev = edges[edges[i].next].next;
                        uint32_t j_prev = edges[edges[j].next].next;

                        edges[i_prev].next = j;
                        edges[j_prev].next = i;

                        std::swap(edges[i].next, edges[j].next);

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

        debug_msg("Edges sorted\n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next);
        }

        //============================================================================================================================================================================================================
        // fill opposite edge indices, -1 will indicate boundary edges                                                                                                                                                        
        //============================================================================================================================================================================================================

        for (uint32_t e = 0; e < E; ++e)
        {
            index_t a = edges[e].a;
            index_t b = edges[e].b;

            if (a < b)
            {
                int32_t l = 0;
                int32_t r = E - 1;
                while (l <= r)
                {
                    int32_t o = (r + l) / 2;

                    if (edges[o].b < a) { l = o + 1; continue; }
                    if (edges[o].b > a) { r = o - 1; continue; }
                    if (edges[o].a < b) { l = o + 1; continue; }
                    if (edges[o].a > b) { r = o - 1; continue; }

                    //================================================================================================================================================================================================
                    // opposite edge found, o is its index
                    //================================================================================================================================================================================================
                    edges[e].opposite = o;
                    edges[o].opposite = e;
                    break;
                }
            }
        }

        debug_msg("Edges opposite filled :: \n\n");

        for(uint32_t e = 0; e < E; ++e)
        {
            debug_msg("edges[%u] = halfedge(a = %u, b = %u, face = %u, next = %u, opposite = %u)", e, edges[e].a, edges[e].b, edges[e].face, edges[e].next, edges[e].opposite);
        }
    }


    bool validate()
    {
        debug_msg("Validating half edge-face structure :: \n\n");

        for (uint32_t e = 0; e < E; ++e)
        {
            uint32_t o = edges[e].opposite;
            bool edge_degeneracy_test = (edges[e].a != edges[e].b);

            bool face_cycle_test = (edges[edges[edges[e].next].next].next == e);

            bool next_face_test = (edges[edges[e].next].a == edges[e].b);

            bool opposite_edge_test = (o != e) &&
                                      (edges[o].a == edges[e].b) && 
                                      (edges[o].b == edges[e].a) &&
                                      (edges[o].opposite == e);

            index_t x = faces[edges[e].face].x,
                    y = faces[edges[e].face].y,
                    z = faces[edges[e].face].z;

            bool face_test = ((edges[e].a == x) && (edges[e].b == y)) || 
                             ((edges[e].a == y) && (edges[e].b == z)) ||
                             ((edges[e].a == z) && (edges[e].b == x));

            if (!edge_degeneracy_test)
            {
                debug_msg("degenerate edge #%u = [%u, %u]", e, edges[e].a, edges[e].b);
                return false;
            }

            if (!face_cycle_test)
            {
                debug_msg("face_cycle_test failed for edge e = %u", e);
                return false;
            }
            if (!next_face_test)
            {
                debug_msg("next_face_test failed for edge e = %u", e);
                debug_msg("\tedges[edges[e].next].a = %u", edges[edges[e].next].a);
                debug_msg("\tedges[e].b = %u", edges[e].b);
                return false;
            }
            if (!opposite_edge_test)
            {
                debug_msg("opposite_edge_test failed for edge e = %u", e);
                debug_msg("invalid opposite edge #%u = [%u, %u], opposite #%u = [%u, %u]", e, edges[e].a, edges[e].b, o, edges[o].a, edges[o].b);
                return false;
            }
            if (!face_test)
            {
                debug_msg("face_test failed for edge e = [%u, %u]. Face = {%u, %u, %u}", edges[e].a, edges[e].b, x, y, z);
                return false;
            }
            return true;
        }        
    }

    //================================================================================================================================================================================================================
    // the edge AB is flippable iff for the two adjacent triangles ABC and BAD with pair ADC + BCD
    // the edge CD is not an edge of the triangulation
    //================================================================================================================================================================================================================
    bool flippable(uint32_t e)
    {
        uint32_t o = edges[e].opposite;

        index_t c = edges[edges[e].next].b;
        index_t d = edges[edges[o].next].b;

        return c != d;
    }

    //================================================================================================================================================================================================================
    // replaces triangle pair ABC + ADB with pair ADC + BCD
    //================================================================================================================================================================================================================
    void flip_edge(uint32_t e)
    {
        uint32_t e_ab = e;
        uint32_t e_ba = edges[e].opposite;

        uint32_t e_bc = edges[e_ab].next;
        uint32_t e_ca = edges[e_bc].next;
        uint32_t e_ad = edges[e_ba].next;
        uint32_t e_db = edges[e_ad].next;

        index_t a = edges[e_ab].a;
        index_t b = edges[e_ab].b;
        index_t c = edges[e_bc].b;
        index_t d = edges[e_ad].b;

        uint32_t e_dc = e_ab;
        uint32_t e_cd = e_ba;

        edges[e_dc].a = d;
        edges[e_dc].b = c;
        edges[e_cd].a = c;
        edges[e_cd].b = d;

        edges[e_dc].next = e_ca;
        edges[e_ca].next = e_ad;
        edges[e_ad].next = e_dc;

        edges[e_cd].next = e_db;
        edges[e_db].next = e_bc;
        edges[e_bc].next = e_cd;

        uint32_t f_adc = edges[e_ab].face;
        uint32_t f_bcd = edges[e_ba].face;

        edges[e_ad].face = f_adc;
        edges[e_bc].face = f_bcd;

        faces[f_adc] = glm::tvec3<index_t>(a, d, c);
        faces[f_bcd] = glm::tvec3<index_t>(b, c, d);
    }

    double angle_defect(double cos_A, double cos_B, double cos_C)
    {
        return glm::abs(cos_A - 0.5) + glm::abs(cos_B - 0.5) + glm::abs(cos_C - 0.5);
    }

    void flip_edges(double threshold)
    {
        for(GLuint e = 0; e < E; ++e)
        {
            uint32_t o = edges[e].opposite;

            index_t a = edges[e].a;
            index_t b = edges[e].b;
            index_t c = edges[edges[e].next].b;
            index_t d = edges[edges[o].next].b;

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
            double degeneracy_DBC = angle_defect(cos_BCD, cos_DBC, cos_CDB);


            if(degeneracy_ABC + degeneracy_ADB > degeneracy_ADC + degeneracy_DBC)
            {
            }

        }    
    }

    void calculate_area_weighted_normals()
    {
        if (!normals)
            normals = (glm::dvec3*) calloc(V, sizeof(glm::dvec3));

        for(GLuint f = 0; f < F; ++f)
        {
            glm::tvec3<index_t> face = faces[f];

            glm::dvec3 A = positions[face.x];
            glm::dvec3 B = positions[face.y];
            glm::dvec3 C = positions[face.z];
            glm::dvec3 n = glm::cross(B - A, C - A);

            normals[face.x] += n;
            normals[face.y] += n;
            normals[face.z] += n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
    }

    void calculate_angle_weighted_normals()
    {
        if (!normals)
            normals = (glm::dvec3*) calloc(V, sizeof(glm::dvec3));

        for(GLuint f = 0; f < F; ++f)
        {
            glm::tvec3<index_t> face = faces[f];

            glm::dvec3 A = positions[face.x];
            glm::dvec3 B = positions[face.y];
            glm::dvec3 C = positions[face.z];

            glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));

            glm::dvec3 AB = glm::normalize(B - A);
            glm::dvec3 BC = glm::normalize(C - B);
            glm::dvec3 CA = glm::normalize(A - C);

            double angle_A = glm::acos(dot(CA, AB));
            double angle_B = glm::acos(dot(AB, BC));
            double angle_C = glm::acos(dot(BC, CA));

            normals[face.x] += angle_A * n;
            normals[face.y] += angle_B * n;
            normals[face.z] += angle_C * n;
        }

        for(GLuint v = 0; v < V; ++v)
            normals[v] = glm::normalize(normals[v]);
    }


    void export_vao(const char* file_name, bool include_normals = true)
    {
        if (include_normals)
        {
            calculate_angle_weighted_normals();
            vertex_pn_t* vertices = (vertex_pn_t*) malloc(sizeof(vertex_pn_t) * V);
            for(uint32_t v = 0; v < V; ++v)
                vertices[v] = vertex_pn_t(glm::vec3(positions[v]), glm::vec3(normals[v]));
            vao_t::store(file_name, GL_TRIANGLES, vertices, V, (index_t*) faces, 3 * F);
            free(vertices);
        }
        else
        {
            vertex_p_t* vertices = (vertex_p_t*) malloc(sizeof(vertex_p_t) * V);
            for(uint32_t v = 0; v < V; ++v)
                vertices[v] = vertex_p_t(glm::vec3(positions[v]));
            vao_t::store(file_name, GL_TRIANGLES, vertices, V, (index_t*) faces, 3 * F);
            free(vertices);
        }
    }

    void export_obj(const char* file_name, bool include_normals = true)
    {
        FILE* f = fopen(file_name, "w");

        fprintf(f, "# Manifold mesh exporter\n# vertices = %u\n# triangles = %u.\n\n", V, F);

        for(uint32_t v = 0; v < V; ++v)
            fprintf(f, "v %.10f, %.10f, %.10f\n", positions[v].x, positions[v].y, positions[v].z);

        fprintf(f, "\n");

        if (include_normals)
        {
            for(uint32_t v = 0; v < V; ++v)
                fprintf(f, "vn %.10f, %.10f, %.10f\n", normals[v].x, normals[v].y, normals[v].z);

            fprintf(f, "\n");

            for(uint32_t t = 0; t < F; ++t)
            {
                uint32_t x = faces[t].x + 1;
                uint32_t y = faces[t].y + 1;
                uint32_t z = faces[t].z + 1;
                fprintf(f, "f %u/%u %u/%u %u/%u\n", x, x, y, y, z, z);
            }
        }
        else
        {
            for(uint32_t t = 0; t < F; ++t)
                fprintf(f, "f %u %u %u\n", faces[t].x + 1, faces[t].y + 1, faces[t].z + 1);
        }

        fprintf(f, "\n");
        fclose(f);
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

    ~he_manifold_t()
        { free(edges); }
};


struct hqs_model_t
{
    uint32_t V;
    uint32_t F;

    std::vector<glm::dvec3> positions;
    std::vector<glm::uvec3> faces;
    std::vector<glm::dvec3> normals;

    double bbox_max;

    hqs_model_t(const char* file_name)
    {
        debug_msg("Loading %s model", file_name);

        std::vector<glm::dvec3> positions;
        std::vector<glm::uvec3> indices;

        FILE* file = fopen(file_name, "rb");
        char buf[4096];

        while(fgets (buf, sizeof(buf), file))
        {
            char token = buf[0];
            //===========================================================================================================================================================================================================
            // skip any line that does not begin with 'v' or 'f'
            //===========================================================================================================================================================================================================
            if ((token != 'v') && (token != 'f')) continue;

            //===========================================================================================================================================================================================================
            // is it a new face? 
            //===========================================================================================================================================================================================================
            if ('v' == token)
            {
                glm::dvec3 vertex;
                if (3 != sscanf(&buf[2], "%lf %lf %lf", &vertex[0], &vertex[1], &vertex[2])) continue;
                vertices.push_back(vertex);
                continue;
            }

            //===========================================================================================================================================================================================================
            // if not, then it is a new vertex position
            //===========================================================================================================================================================================================================
            glm::uvec3 triangle;
            if (3 != sscanf(&buf[2], "%i %i %i", &triangle[0], &triangle[1], &triangle[2])) continue;
            faces.push_back(triangle - glm::uvec3(1));
        }
        fclose(file);

        V = positions.size();
        F = faces.size();
        debug_msg("File %s parsed : vertices = %u, faces = %u", file_name, V, F);
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

};    


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // load model and build it edge-face structure
    //===================================================================================================================================================================================================================
    const int F = 20;
    const int V = 12;

    glm::uvec3 triangles[F] = 
    {
        glm::uvec3(2,  0,  8),
        glm::uvec3(0,  2,  9),
        glm::uvec3(4,  0,  6),
        glm::uvec3(0,  4,  8),
        glm::uvec3(6,  0,  9),
        glm::uvec3(1,  3, 10),
        glm::uvec3(3,  1, 11),
        glm::uvec3(1,  4,  6),
        glm::uvec3(4,  1, 10),
        glm::uvec3(1,  6, 11),
        glm::uvec3(2,  5,  7),
        glm::uvec3(5,  2,  8),
        glm::uvec3(2,  7,  9),
        glm::uvec3(5,  3,  7),
        glm::uvec3(3,  5, 10),
        glm::uvec3(7,  3, 11),
        glm::uvec3(8,  4, 10),
        glm::uvec3(5,  8, 10),
        glm::uvec3(6,  9, 11),
        glm::uvec3(9,  7, 11) 
    };

    const double phi = constants::phi_d;

    glm::dvec3 positions[V] = 
    {
        glm::dvec3(  0.0, -1.0, -phi),
        glm::dvec3(  0.0, -1.0,  phi),
        glm::dvec3(  0.0,  1.0, -phi),
        glm::dvec3(  0.0,  1.0,  phi),
        glm::dvec3( -1.0, -phi,  0.0),
        glm::dvec3( -1.0,  phi,  0.0),
        glm::dvec3(  1.0, -phi,  0.0),
        glm::dvec3(  1.0,  phi,  0.0),
        glm::dvec3( -phi,  0.0, -1.0),
        glm::dvec3(  phi,  0.0, -1.0),
        glm::dvec3( -phi,  0.0,  1.0),
        glm::dvec3(  phi,  0.0,  1.0)
    };

    he_manifold_t<GLuint> manifold_struct(triangles, F, positions, V);

    manifold_struct.validate();

    manifold_struct.export_vao("icosahedron.vao", true);
    manifold_struct.export_obj("icosahedron.obj", false);
    manifold_struct.export_obj("icosahedron_n.obj", true);

    uint32_t seed = 12365407;
    for(uint32_t q = 0; q < 5000; ++q)
    {
        seed = 3259123461 * seed + 123526539;
        uint32_t e = seed % manifold_struct.E;

        debug_msg("Flip test #%u :: edge #%u = [%u, %u].\n", q, e, manifold_struct.edges[e].a, manifold_struct.edges[e].b);

        if (manifold_struct.flippable(e))
        {
            manifold_struct.flip_edge(e);
            if (!manifold_struct.validate())
                break;

        }

    }

    return 0;
}
/*

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
    // load model and build it edge-face structure
    //===================================================================================================================================================================================================================
    hqs_model_t model("../../../resources/manifolds/demon.obj");
    model.normalize(1.0);
    model.calculate_angle_weighted_normals();
    model.test_normals();

    edge_face_struct<GLuint> manifold_struct(model.indices.data(), model.F, model.positions.data(), model.V);

    vao_t model_vao = model.create_vao();

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
        model_vao.render();
           
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================

    glfw::terminate();
    return 0;
}
*/