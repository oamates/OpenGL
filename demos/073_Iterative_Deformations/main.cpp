//========================================================================================================================================================================================================================
// DEMO 073 : Fractal Surface and Vertex/Fragment ambient occlusion
//========================================================================================================================================================================================================================

#include <random>

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT
#define GLM_FORCE_INLINE

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_aux.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "plato.hpp"
#include "attribute.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    bool wireframe_mode = false;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_aux::dump_info(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_ENTER) && (action == GLFW_RELEASE))
        {
            wireframe_mode = !wireframe_mode;            

            if (wireframe_mode)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

std::default_random_engine generator;
std::normal_distribution<float> gaussRand(0.0, 1.0);


//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
glm::vec3 tri(const glm::vec3& x)
{
    return glm::abs(glm::fract(x) - glm::vec3(0.5f));
}

float sdf(const glm::vec3& p)
{    
    glm::vec3 op = tri(1.1f * p + tri(1.1f * glm::vec3(p.z, p.x, p.y)));
    glm::vec3 q = p + (op - glm::vec3(0.25f)) * 0.3f;
    q = glm::cos(0.444f * q + glm::sin(1.112f * glm::vec3(p.z, p.x, p.y)));
    return glm::length(p) - 1.05f;
}

glm::vec3 grad(glm::vec3 p)
{
    const float delta = 0.0125f;
    return glm::normalize(glm::vec3( delta, -delta, -delta) * sdf(p + glm::vec3( delta, -delta, -delta)) + 
                          glm::vec3(-delta, -delta,  delta) * sdf(p + glm::vec3(-delta, -delta,  delta)) + 
                          glm::vec3(-delta,  delta, -delta) * sdf(p + glm::vec3(-delta,  delta, -delta)) + 
                          glm::vec3( delta,  delta,  delta) * sdf(p + glm::vec3( delta,  delta,  delta)));
}



float noise_func(const glm::vec3& v, const glm::vec3& n, GLuint l)
{
    glm::vec3 g = grad(12.0f * v);

    const float magnitudes[] = {0.25138f, 0.093573f, 0.042512f, 0.027535f, 0.014214f, 0.009539f, 0.004125f, 0.001525f, 0.000825f, 0.000225f};
    
    float d = 5.51f * magnitudes[l];

    

    return (glm::dot(g, n) > 0.0f) ? -d : d;

}

template<typename vertex_t> vao_t generate_fractal_vbo(const glm::vec3* positions, GLuint V, const glm::uvec3* triangles, GLuint F, GLuint levels)
{
    //===================================================================================================================================================================================================================
    // preliminary step 1 :: calculate final number of vertices, edges and faces
    //===================================================================================================================================================================================================================

    GLuint pow4 = 1 << (2 * levels);
    GLuint nV = pow4 * (V - 2) + 2;
    GLuint nF = pow4 * F;
    GLuint nE = nF + nF + nF;

    struct edge_t
    {
        GLuint a, b;
        GLuint face;
        GLuint midpoint; 
        edge_t(GLuint a, GLuint b, GLuint face, GLuint midpoint) : a(a), b(b), face(face), midpoint(midpoint) {}      
    };

    GLuint v = 0, e = 0, f = 0;

    //===================================================================================================================================================================================================================
    // preliminary step 2 :: allocate memory for surface construction, copy and set up initial data
    //===================================================================================================================================================================================================================
    edge_t* edges = (edge_t*) malloc(nE * sizeof(edge_t));
    glm::uvec3* faces = (glm::uvec3*) malloc(nF * sizeof(glm::uvec3));
    vertex_t* vertices = (vertex_t*) malloc(nV * sizeof(vertex_t));
    
    while(v < V)
    {
        vertices[v].position = positions[v];
        vertices[v].normal = glm::vec3(0.0f);
        ++v;
    }

    while(f < F)                                                                                                                                                                                      
    {                                                
        faces[f] = triangles[f];
        edges[e++] = edge_t(faces[f].x, faces[f].y, f, -1);                                                                                                                                                                        
        edges[e++] = edge_t(faces[f].y, faces[f].z, f, -1);                                                                                                                                                                        
        edges[e++] = edge_t(faces[f].z, faces[f].x, f, -1);
        ++f;                                                                                                                                                                        
    }

    float lambda = 0.5;

    for(int level = 0; level < levels; ++level)
    {
        //================================================================================================================================================================================================================
        // 1 :: quick sort edges lexicographically
        //================================================================================================================================================================================================================
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests
        
        struct                                                                                                                                                                                                                
        {                                                                                                                                                                                                                     
            edge_t* l;                                                                              // left index of the sub-array that needs to be sorted                                                                    
            edge_t* r;                                                                              // right index of the sub-array to sort                                                                                   
        } _stack[STACK_SIZE];                                                                                                                                                                                                 
                                                                                                                                                                                                                              
        int sp = 0;                                                                                 // stack pointer, stack grows up, not down                                                                                 
        _stack[sp].l = edges;                                                                                                                                                                                                 
        _stack[sp].r = edges + e - 1;                                                                                                                                                                                         
                                                                                                                                                                                                                              
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            edge_t* l = _stack[sp].l;                                                                                                                                                                                
            edge_t* r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                edge_t* i = l;                                                                                                                                                                                       
                edge_t* j = r;                                                                                                                                                                                       
                edge_t* m = i + (j - i) / 2;                                                                                                                                                                         
                GLuint a = m->a;                                                                                                                                                                                             
                GLuint b = m->b;                                                                                                                                                                                             
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while ((i->b < b) || ((i->b == b) && (i->a < a))) i++;                          // lexicographic compare and proceed forward if less                                                                      
                    while ((j->b > b) || ((j->b == b) && (j->a > a))) j--;                          // lexicographic compare and proceed backward if greater                                                                    
                                                                                                                                                                                                                              
                    if (i <= j)                                                                                                                                                                                               
                    {                                                                                                                                                                                                         
                        std::swap(i->a, j->a);                                                                                                                                                                                
                        std::swap(i->b, j->b);                                                                                                                                                                                
                        std::swap(i->face, j->face);                                                                                                                                                                                
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
        
        //================================================================================================================================================================================================================
        // 2 :: go over all the edges and assign indices to the new vertices, filling their positions
        //================================================================================================================================================================================================================
        for(GLuint ee = 0; ee < e; ++ee)
        {
            edge_t& edge = edges[ee];
            if (edge.midpoint != -1) continue;

            //============================================================================================================================================================================================================
            // 2.1 :: find edge ba 
            //============================================================================================================================================================================================================
            GLint l = 0;                                                                                                                                                                                                        
            GLint r = e - 1;
            GLint m;
            while (l <= r)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                m = (r + l) / 2;                                                                                                                                                                                  
                                                                                                                                                                                                                      
                if (edges[m].b < edge.a) { l = m + 1; continue; }                                                                                                                                                              
                if (edges[m].b > edge.a) { r = m - 1; continue; }                                                                                                                                                              
                if (edges[m].a < edge.b) { l = m + 1; continue; }                                                                                                                                                              
                if (edges[m].a > edge.b) { r = m - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 2.2 :: update midpoint index for both [ab] and [ba] 
            //============================================================================================================================================================================================================
            edge.midpoint = v++;
            edges[m].midpoint = edge.midpoint;
            glm::uvec3& abc = faces[edge.face];
            glm::uvec3& bad = faces[edges[m].face];

            GLuint a = edge.a;
            GLuint b = edge.b;
            GLuint s = a + b;
            GLuint c = abc.x + abc.y + abc.z - s;
            GLuint d = bad.x + bad.y + bad.z - s;

            glm::vec3 mid_ab = 0.5f * (vertices[a].position + vertices[b].position);
            glm::vec3 mid_cd = 0.5f * (vertices[c].position + vertices[d].position);

            vertices[edge.midpoint].position = (1.0f + lambda) * mid_ab - lambda * mid_cd;
            vertices[edge.midpoint].normal = glm::vec3(0.0f);
        }

        lambda *= 0.37f;

        //================================================================================================================================================================================================================
        // 3 :: rebuild faces array and generate new edges at the same time
        //================================================================================================================================================================================================================
        GLuint F = f;

        for(GLuint ff = 0; ff < F; ++ff)
        {
            glm::uvec3 face = faces[ff];
            GLuint a = face.x;
            GLuint b = face.y;
            GLuint c = face.z;
            GLint ll, rr;
            GLint e_ab, e_bc, e_ca;

            //============================================================================================================================================================================================================
            // 3.1 :: find edge [ab]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_ab = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_ab].b < b) { ll = e_ab + 1; continue; }                                                                                                                                                              
                if (edges[e_ab].b > b) { rr = e_ab - 1; continue; }                                                                                                                                                              
                if (edges[e_ab].a < a) { ll = e_ab + 1; continue; }                                                                                                                                                              
                if (edges[e_ab].a > a) { rr = e_ab - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 3.2 :: find edge [bc]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_bc = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_bc].b < c) { ll = e_bc + 1; continue; }                                                                                                                                                              
                if (edges[e_bc].b > c) { rr = e_bc - 1; continue; }                                                                                                                                                              
                if (edges[e_bc].a < b) { ll = e_bc + 1; continue; }                                                                                                                                                              
                if (edges[e_bc].a > b) { rr = e_bc - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 3.3 :: find edge [ca]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_ca = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_ca].b < a) { ll = e_ca + 1; continue; }                                                                                                                                                              
                if (edges[e_ca].b > a) { rr = e_ca - 1; continue; }                                                                                                                                                              
                if (edges[e_ca].a < c) { ll = e_ca + 1; continue; }                                                                                                                                                              
                if (edges[e_ca].a > c) { rr = e_ca - 1; continue; }                                                                                                                                                              
                break;
            }
            
            GLuint p = edges[e_bc].midpoint;
            GLuint q = edges[e_ca].midpoint;
            GLuint r = edges[e_ab].midpoint;
                             

            faces[ff] = glm::uvec3(p, q, r);
            GLuint f_arq = f++; faces[f_arq] = glm::uvec3(a, r, q);
            GLuint f_bpr = f++; faces[f_bpr] = glm::uvec3(b, p, r);
            GLuint f_cqp = f++; faces[f_cqp] = glm::uvec3(c, q, p);
        }

        //================================================================================================================================================================================================================
        // 4 :: fill edge array
        //================================================================================================================================================================================================================
        e = 0;
        for(GLuint ff = 0; ff < f; ++ff)
        {
            glm::uvec3& face = faces[ff];
            edges[e++] = edge_t(face.x, face.y, ff, -1);
            edges[e++] = edge_t(face.y, face.z, ff, -1);
            edges[e++] = edge_t(face.z, face.x, ff, -1);
        }

        //================================================================================================================================================================================================================
        // 5 :: recalculate normals
        //================================================================================================================================================================================================================
        for(GLuint ff = 0; ff < f; ++ff)
        {
            glm::uvec3& face = faces[ff];
            glm::vec3 normal = glm::cross(vertices[face.y].position - vertices[face.x].position, vertices[face.z].position - vertices[face.x].position);
            vertices[face.x].normal += normal;
            vertices[face.y].normal += normal;
            vertices[face.z].normal += normal;
        }
        
        //================================================================================================================================================================================================================
        // 6 :: displace vertices along normals, zero the normals
        //================================================================================================================================================================================================================
        for(GLuint vv = 0; vv < v; ++vv)
        {
            glm::vec3 normal = glm::normalize(vertices[vv].normal);
            vertices[vv].position = vertices[vv].position + noise_func(vertices[vv].position, normal, level) * normal;
            vertices[vv].normal = glm::vec3(0.0f);
        }
    }

    //====================================================================================================================================================================================================================
    // mesh construction is over --- recalculate normals
    //====================================================================================================================================================================================================================
    for(GLuint f = 0; f < nF; ++f)
    {
        glm::uvec3& face = faces[f];
        glm::vec3 normal = glm::cross(vertices[face.y].position - vertices[face.x].position, vertices[face.z].position - vertices[face.x].position);
        vertices[face.x].normal += normal;
        vertices[face.y].normal += normal;
        vertices[face.z].normal += normal;
    }

    for(GLuint vv = 0; vv < v; ++vv)
        vertices[vv].normal = glm::normalize(vertices[vv].normal);

    free(edges);


    attribute_data_t<vertex_t, GLuint, GL_TRIANGLES> attribute_data(nV, nF * 3, vertices, (GLuint*) faces);

    //====================================================================================================================================================================================================================
    // create OpenGL position-normal-occlusion VAO and release the memory allocated                                                                                       
    //====================================================================================================================================================================================================================
    vao_t vao;
    vao.init(GL_TRIANGLES, vertices, nV, glm::value_ptr(faces[0]), nF * 3);
    
    return vao;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Fractal Surface", 4, 3, 3, 1920, 1080);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t occlusion_renderer(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/occlusion.vs"),
                                      glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/occlusion.fs"));

    occlusion_renderer.dump_info();
    uniform_t uni_projection_view_matrix = occlusion_renderer["projection_view_matrix"];
    uniform_t uni_camera_ws              = occlusion_renderer["camera_ws"];
    occlusion_renderer.enable();

    //===================================================================================================================================================================================================================
    // Generate the surface
    //===================================================================================================================================================================================================================

    glm::vec3 positions[plato::icosahedron::V];

    for (int i = 0; i < plato::icosahedron::V; ++i)
    {
        glm::vec3 q = plato::icosahedron::vertices[i];
        glm::vec3 p = q * glm::vec3(7.17f, 6.83f, 3.11f);
        positions[i] = q;
    }

    vao_t deformed_icosahedron = generate_fractal_vbo<vertex_pn_t>(positions, plato::icosahedron::V, plato::icosahedron::triangles, plato::icosahedron::F, 7);

    glEnable(GL_DEPTH_TEST);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.new_frame();
        glm::vec3 camera_ws = window.camera.position();

        //===============================================================================================================================================================================================================
        // render the generated surface
        //===============================================================================================================================================================================================================
            
        uni_projection_view_matrix = window.camera.projection_view_matrix();
        uni_camera_ws = camera_ws;
        deformed_icosahedron.render();

        //===============================================================================================================================================================================================================
        // done : show back buffer and process events
        //===============================================================================================================================================================================================================
        window.end_frame();
    } 

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}
