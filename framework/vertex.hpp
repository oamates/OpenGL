#ifndef _vertex_included_097469105165845730195256723852015841010135213539578645
#define _vertex_included_097469105165845730195256723852015841010135213539578645

#include <GL/glew.h>
#include <glm/glm.hpp>
//===================================================================================================================================================================================================================
// Bitmask that encodes sizes of consecutive vertex attributes. 
// Attribute sizes acceptable by OpenGL are 1, 2, 3 or 4, so each attribute size is stored to occupy 4 bits
//===================================================================================================================================================================================================================
constexpr GLuint BUFFER_LAYOUT(GLuint a0)                                             
    { return a0; }

constexpr GLuint BUFFER_LAYOUT(GLuint a0, GLuint a1)                                  
    { return a0 + a1 * 0x10; }

constexpr GLuint BUFFER_LAYOUT(GLuint a0, GLuint a1, GLuint a2)                       
    { return a0 + a1 * 0x10 + a2 * 0x100; }

constexpr GLuint BUFFER_LAYOUT(GLuint a0, GLuint a1, GLuint a2, GLuint a3)            
    { return a0 + a1 * 0x10 + a2 * 0x100 + a3 * 0x1000; }

constexpr GLuint BUFFER_LAYOUT(GLuint a0, GLuint a1, GLuint a2, GLuint a3, GLuint a4) 
    { return a0 + a1 * 0x10 + a2 * 0x100 + a3 * 0x1000 + a4 * 0x10000; }

//===================================================================================================================================================================================================================
// Structures representing different types of vertex data :
//  - vertex_pft3_t : position + tangent frame + 3d texture coordinate
//  - vertex_pft2_t : position + tangent frame + 2d texture coordinate
//  - vertex_pf_t   : position + tangent frame
//  - vertex_pnt3_t : position + normal + 3d texture coordinate
//  - vertex_pnt2_t : position + normal + 2d texture coordinate
//  - vertex_pnc_t  : position + normal + color
//  - vertex_pn_t   : position + normal
//  - vertex_pno_t  : position + normal + occlusion
//  - vertex_pnoh_t : position + normal + occlusion + hue
//  - vertex_pc_t   : position + color
//  - vertex_p_t    : position
//  - vertex_p2t2   : 2d position + 2d texture coordinate --- useful for text rendering
//  - vertex_t3_t   : 3d (texture) coordinate
//  - vertex_t2_t   : 2d (texture) coordinate
//===================================================================================================================================================================================================================

struct vertex_pft3_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent_x;
    glm::vec3 tangent_y;
    glm::vec3 uvw;

    vertex_pft3_t() {};
    vertex_pft3_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent_x, const glm::vec3& tangent_y, const glm::vec3& uvw) 
        : position(position), normal(normal), tangent_x(tangent_x), tangent_y(tangent_y), uvw(uvw) {};

    vertex_pft3_t& operator = (const vertex_pft3_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 3, 3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 15;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 3, 3, 3);
};

struct vertex_pft2_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent_x;
    glm::vec3 tangent_y;
    glm::vec2 uv;

    vertex_pft2_t() {};
    vertex_pft2_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent_x, const glm::vec3& tangent_y, const glm::vec2& uv) 
        : position(position), normal(normal), tangent_x(tangent_x), tangent_y(tangent_y), uv(uv) {};
    vertex_pft2_t& operator = (const vertex_pft2_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 3, 3, 2};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 14;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 3, 3, 2);
};

struct vertex_pf_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent_x;
    glm::vec3 tangent_y;

    vertex_pf_t() {};
    vertex_pf_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent_x, const glm::vec3& tangent_y) 
        : position(position), normal(normal), tangent_x(tangent_x), tangent_y(tangent_y) {};
    vertex_pf_t& operator = (const vertex_pf_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 12;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 3, 3);
};

struct vertex_pnt3_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 uvw;

    vertex_pnt3_t() {};
    vertex_pnt3_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& uvw) : position(position), normal(normal), uvw(uvw) {};
    vertex_pnt3_t& operator = (const vertex_pnt3_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 9;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 3);
};

struct vertex_pnt2_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    vertex_pnt2_t() {};
    vertex_pnt2_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv) : position(position), normal(normal), uv(uv) {};
    vertex_pnt2_t& operator = (const vertex_pnt2_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 2};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 8;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 2);
};

struct vertex_pnc_t
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    vertex_pnc_t() {};
    vertex_pnc_t(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& color) : position(position), normal(normal), color(color) {};
    vertex_pnc_t& operator = (const vertex_pnc_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 9;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 3);
};

struct vertex_pn_t
{
    glm::vec3 position;
    glm::vec3 normal;

    vertex_pn_t() {};
    vertex_pn_t(const glm::vec3& position, const glm::vec3& normal) : position(position), normal(normal)  {};
    vertex_pn_t& operator = (const vertex_pn_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 6;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3);
};

struct vertex_pno_t
{
    glm::vec3 position;
    glm::vec3 normal;
    float occlusion;

    vertex_pno_t() {};
    vertex_pno_t(const glm::vec3& position, const glm::vec3& normal, float occlusion) : position(position), normal(normal), occlusion(occlusion) {};
    vertex_pno_t& operator = (const vertex_pno_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 1};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 7;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 1);
};

//  - vertex_pnoh_t : position + normal + occlusion + hue
struct vertex_pnoh_t
{
    glm::vec3 position;
    glm::vec3 normal;
    float occlusion;
    float hue;

    vertex_pnoh_t() {};
    vertex_pnoh_t(const glm::vec3& position, const glm::vec3& normal, float occlusion, float hue) : position(position), normal(normal), occlusion(occlusion), hue(hue) {};
    vertex_pnoh_t& operator = (const vertex_pnoh_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3, 1, 1};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 8;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3, 1, 1);
};


struct vertex_pc_t
{
    glm::vec3 position;
    glm::vec3 color;

    vertex_pc_t() {};
    vertex_pc_t(const glm::vec3& position, const glm::vec3& color) : position(position), color(color)  {};
    vertex_pc_t& operator = (const vertex_pc_t&) = default;

    static constexpr GLuint dimensions[] = {3, 3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 6;
    static constexpr GLuint layout = BUFFER_LAYOUT(3, 3);
};

struct vertex_p_t
{
    glm::vec3 position;

    vertex_p_t() {};
    vertex_p_t(const glm::vec3& position) : position(position) {};
    vertex_p_t& operator = (const vertex_p_t&) = default;

    static constexpr GLuint dimensions[] = {3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 3;
    static constexpr GLuint layout = BUFFER_LAYOUT(3);
};

struct vertex_p2t2_t
{
    glm::vec2 position;
    glm::vec2 uv;

    vertex_p2t2_t() {};
    vertex_p2t2_t(const glm::vec2& position, const glm::vec2& uv) : position(position), uv(uv) {};
    vertex_p2t2_t& operator = (const vertex_p2t2_t&) = default;

    static constexpr GLuint dimensions[] = {2, 2};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 4;
    static constexpr GLuint layout = BUFFER_LAYOUT(2, 2);
};

struct vertex_t3_t
{
    glm::vec3 uvw;

    vertex_t3_t() {};
    vertex_t3_t(const glm::vec3& uvw) : uvw(uvw) {};
    vertex_t3_t& operator = (const vertex_t3_t&) = default;

    static constexpr GLuint dimensions[] = {3};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 3;
    static constexpr GLuint layout = BUFFER_LAYOUT(3);
};

struct vertex_t2_t
{
    glm::vec2 uv;

    vertex_t2_t() {};
    vertex_t2_t(const glm::vec3& uv) : uv(uv) {};
    vertex_t2_t& operator = (const vertex_t2_t&) = default;

    static constexpr GLuint dimensions[] = {2};
    static constexpr GLuint attributes = sizeof(dimensions) / sizeof(GLuint);
    static constexpr GLuint total_dimension = 2;
    static constexpr GLuint layout = BUFFER_LAYOUT(2);
};


template<typename vertex_t> struct maps
{
    typedef vertex_t (*toral_func)     (const glm::vec2& uv);                  // (u, v) will run over the unit square
    typedef vertex_t (*surface_func)   (const glm::vec2& uv);                  // (u, v) will run over the unit square, the function must be double periodic
    typedef vertex_t (*spheric_func)   (const glm::vec3& uvw);                 // (u, v, w) will run over the unit sphere
    typedef vertex_t (*curve_func)     (const glm::vec2& uv);                  // (u, v) will parameterize the unit circle, u = cos(theta), v = sin(theta)
                                       
    typedef vertex_t (*toral_dfunc)    (const glm::dvec2& uv);                 // (u, v) will run over the unit square
    typedef vertex_t (*surface_dfunc)  (const glm::dvec2& uv);                 // (u, v) will run over the unit square, the function must be double periodic
    typedef vertex_t (*spheric_dfunc)  (const glm::dvec3& uvw);                // (u, v, w) will run over the unit sphere
    typedef vertex_t (*curve_dfunc)    (const glm::dvec2& uv);                 // (u, v) will parameterize the unit circle, u = cos(theta), v = sin(theta)

    typedef vertex_t (*face_tess_func) (const vertex_t& A, const vertex_t& B, const vertex_t& C, const glm::vec3& uvw);
    typedef vertex_t (*edge_tess_func) (const vertex_t& A, const vertex_t& B, const glm::vec2& uv);
};

#endif // _vertex_included_097469105165845730195256723852015841010135213539578645