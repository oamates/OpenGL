#ifndef _sdf_included_015145034650764357365074365343417534721056415465105143173
#define _sdf_included_015145034650764357365074365343417534721056415465105143173

#include <atomic>
#include <thread>

#include "tex3d.hpp"
#include "log.hpp"

const double INTEGRAL_SCALE = 268435456.0;
const double INV_INT_SCALE = 1.0 / INTEGRAL_SCALE;
const double diameter = 3.464101615137754587054892683011744734;         // = 2sqrt(3)

template<typename real_t> real_t volume(const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
    { return glm::determinant(glm::tmat3x3<real_t>(A, B, C)); }

//=======================================================================================================================================================================================================================
// unsigned distance-to-triangle function
//=======================================================================================================================================================================================================================
template<typename real_t> real_t triangle_udf(const glm::tvec3<real_t>& P, const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
{
    const real_t zero = 0.0;
    const real_t  one = 1.0;

    glm::tvec3<real_t> AB = B - A; glm::tvec3<real_t> AP = P - A;
    glm::tvec3<real_t> BC = C - B; glm::tvec3<real_t> BP = P - B;
    glm::tvec3<real_t> CA = A - C; glm::tvec3<real_t> CP = P - C;

    glm::tvec3<real_t> n = glm::cross(CA, AB);

    if ((volume(n, AB, AP) >= zero) && (volume(n, BC, BP) >= zero) && (volume(n, CA, CP) >= zero))
        return glm::sqrt(glm::dot(n, AP) * glm::dot(n, AP) / glm::length2(n));

    return glm::sqrt(
        glm::min(
            glm::min(
                glm::length2(AB * glm::clamp(glm::dot(AB, AP) / glm::length2(AB), zero, one) - AP),
                glm::length2(BC * glm::clamp(glm::dot(BC, BP) / glm::length2(BC), zero, one) - BP)
            ),
            glm::length2(CA * glm::clamp(glm::dot(CA, CP) / glm::length2(CA), zero, one) - CP)
        )
    );
}

//=======================================================================================================================================================================================================================
// extended unsigned distance-to-triangle function
// returns both the vector from closest point in ABC to P (in xyz components) and the distance (in w component)
//=======================================================================================================================================================================================================================
template<typename real_t> glm::tvec4<real_t> tri_closest_point(const glm::tvec3<real_t>& P, const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
{
    const real_t zero = 0.0;
    const real_t  one = 1.0;

    glm::tvec3<real_t> AB = B - A; glm::tvec3<real_t> AP = P - A;
    glm::tvec3<real_t> BC = C - B; glm::tvec3<real_t> BP = P - B;
    glm::tvec3<real_t> CA = A - C; glm::tvec3<real_t> CP = P - C;

    glm::tvec3<real_t> n = glm::cross(CA, AB);

    if ((volume(n, AB, AP) >= zero) && (volume(n, BC, BP) >= zero) && (volume(n, CA, CP) >= zero))
    {
        n = normalize(n);
        real_t dp = glm::dot(n, AP);
        return glm::tvec4<real_t> (dp * n, glm::abs(dp));
    }

    glm::tvec3<real_t> proj_AB = AP - AB * glm::clamp(glm::dot(AB, AP) / glm::length2(AB), zero, one);
    glm::tvec3<real_t> proj_BC = BP - BC * glm::clamp(glm::dot(BC, BP) / glm::length2(BC), zero, one);
    glm::tvec3<real_t> proj_CA = CP - CA * glm::clamp(glm::dot(CA, CP) / glm::length2(CA), zero, one);

    real_t dAB = glm::length(proj_AB);
    real_t dBC = glm::length(proj_BC);
    real_t dCA = glm::length(proj_CA);

    if (dAB > dBC)
    {
        if (dCA > dBC) return glm::tvec4<real_t>(proj_BC, dBC);
    }
    else
    {
        if (dCA > dAB) return glm::tvec4<real_t>(proj_AB, dAB);
    }
    return glm::tvec4<real_t>(proj_CA, dCA);
}

//=======================================================================================================================================================================================================================
// atomic minimum operations via CAS loops
//=======================================================================================================================================================================================================================
unsigned int atomic_min(std::atomic_uint& atomic_var, unsigned int value)
{
    unsigned int previous_value = atomic_var;
    while(previous_value > value && !atomic_var.compare_exchange_weak(previous_value, value));
    return previous_value;
}

double atomic_min(std::atomic<double>& atomic_var, double value)
{
    double previous_value = atomic_var;
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

template<typename index_t> struct sdf_compute_t
{
    GLuint V;
    GLuint F;
    glm::tvec3<index_t>* faces;

    glm::dvec3* positions;
    glm::dvec3* normals;

    sdf_compute_t(glm::tvec3<index_t>* faces, uint32_t F, glm::dvec3* positions, uint32_t V)
        : faces(faces), F(F), positions(positions), V(V), normals(0)
    { }

    //===================================================================================================================================================================================================================
    // auxiliary structure to be passed to all mesh udf computation threads
    //===================================================================================================================================================================================================================
    struct tri_udf_compute_data_t
    {
        uint32_t max_level;

        glm::dvec3* positions;
        glm::tvec3<index_t>* faces;
        uint32_t F;

        std::atomic_uint triangle_index;
        std::atomic<double>* octree;
        std::atomic<double>* udf_texture;
    };

    //===================================================================================================================================================================================================================
    // auxiliary structure to be passed to all point udf computation threads
    //===================================================================================================================================================================================================================
    struct pnt_udf_compute_data_t
    {
        uint32_t max_level;

        glm::dvec3* positions;
        uint32_t points;

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
    template<int threads> GLuint tri_udf_compute(int max_level, GLenum texture_unit, const char* file_name = 0)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic<double>) == sizeof(double), "Atomic double has larger size than double. Not good.");

        tri_udf_compute_data_t compute_data;

        compute_data.max_level = max_level;
        compute_data.positions = positions;
        compute_data.faces = faces;
        compute_data.F = F;
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

        const double diameter = 3.464101615137754587054892683011744734;         // = 2sqrt(3)

        //===============================================================================================================================================================================================================
        // to avoid dealing with std::atomic<double>, to gain some speed and to save some space
        // the distance field values (bounded by the length of the [-1, 1] 3d-cube diagonal) are scaled and result is stored in an integer atomic array
        //===============================================================================================================================================================================================================

        compute_data.octree      = (std::atomic<double>*) malloc( octree_size * sizeof(double));
        compute_data.udf_texture = (std::atomic<double>*) malloc(texture_size * sizeof(double));

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

        double* udf_data = (double*) compute_data.udf_texture;
        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = (float) udf_data[p];

        if (file_name)
        {
            tex3d_header_t header
            {
                .target = GL_TEXTURE_3D,
                .internal_format = GL_R32F,
                .format = GL_RED,
                .type = GL_FLOAT,
                .size = glm::ivec3(p2, p2, p2),
                .data_size = (uint32_t) texture_size * sizeof(GLfloat)
            };

            FILE* f = fopen(file_name, "wb");
            fwrite(&header, sizeof(tex3d_header_t), 1, f);
            fwrite(texture_data, header.data_size, 1, f);
            fclose(f);
        }


        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

        free(compute_data.octree);
        free(compute_data.udf_texture);
        free(texture_data);

        return texture_id;
    }


    #define SINGLE_CHANNEL_SDF

    //===================================================================================================================================================================================================================
    // computes approximate signed distance function from a triangle mesh to a discrete lattice in 3D-space
    // the function runs multiple threads executing the main unsigned distance function computation algorithm
    // on both external and internal layers of the model
    // no modification is necessary to increase the amount of threads -- so let it be equal to the number of processor (logical) cores
    // the implementation will work for distance textures up to 1024 x 1024 x 1024 dimension
    //===================================================================================================================================================================================================================
    template<int threads> GLuint tri_sdf_compute(int max_level, GLenum texture_unit, double delta, const char* file_name = 0)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic<double>) == sizeof(double), "Atomic double has larger size than double. Not good.");

        const double diameter = 3.464101615137754587054892683011744734;         // = 2sqrt(3)

        tri_udf_compute_data_t compute_data;

        //===============================================================================================================================================================================================================
        // step 1 :: create common compute structure with atomic counter to be used by all threads
        //===============================================================================================================================================================================================================
        compute_data.max_level = max_level;
        compute_data.positions = positions;
        compute_data.faces = faces;
        compute_data.F = F;
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

        compute_data.octree      = (std::atomic<double>*) malloc( octree_size * sizeof(double));
        compute_data.udf_texture = (std::atomic<double>*) malloc(texture_size * sizeof(double));

        for(unsigned int i = 0; i < octree_size; ++i)  compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;
        //===============================================================================================================================================================================================================
        // step 2 :: launch threads to compute udf for the mesh itself
        //===============================================================================================================================================================================================================
        std::thread computation_thread[threads];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

        //===============================================================================================================================================================================================================
        // step 3 :: compute udf for the external shell of the mesh
        //===============================================================================================================================================================================================================
        glm::dvec3* external_layer = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V);
        for(unsigned int v = 0; v < V; ++v)
            external_layer[v] = positions[v] + delta * normals[v];

        compute_data.positions = external_layer;
        compute_data.triangle_index = 0;

        double* udf = (double*) compute_data.udf_texture;
        compute_data.udf_texture = (std::atomic<double>*) malloc(texture_size * sizeof(double));

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

        double* external_udf = (double*) compute_data.udf_texture;

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

    #ifdef SINGLE_CHANNEL_SDF
        float* texture_data = (float*) malloc(texture_size * sizeof(float));

        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = (external_udf[p] < udf[p]) ? udf[p] : -udf[p];

        //===============================================================================================================================================================================================================
        // afoksha
        //===============================================================================================================================================================================================================

        if (file_name)
        {
            tex3d_header_t header
            {
                .target = GL_TEXTURE_3D,
                .internal_format = GL_R32F,
                .format = GL_RED,
                .type = GL_FLOAT,
                .size = glm::ivec3(p2, p2, p2),
                .data_size = (uint32_t) texture_size * sizeof(float)
            };


            FILE* f = fopen(file_name, "wb");
            fwrite(&header, sizeof(tex3d_header_t), 1, f);
            fwrite(texture_data, header.data_size, 1, f);
            fclose(f);
        }

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, p2, p2, p2, 0, GL_RED, GL_FLOAT, texture_data);

    #else
        glm::vec2* texture_data = (glm::vec2*) malloc(texture_size * sizeof(glm::vec2));

        for(unsigned int p = 0; p < texture_size; ++p)
            texture_data[p] = glm::vec2(udf[p], external_udf[p]);

        //===============================================================================================================================================================================================================
        // afoksha
        //===============================================================================================================================================================================================================

        if (file_name)
        {
            tex3d_header_t header
            {
                .target = GL_TEXTURE_3D,
                .internal_format = GL_RG32F,
                .format = GL_RG,
                .type = GL_FLOAT,
                .size = glm::ivec3(p2, p2, p2),
                .data_size = (uint32_t) texture_size * sizeof(glm::vec2)
            };


            FILE* f = fopen(file_name, "wb");
            fwrite(&header, sizeof(tex3d_header_t), 1, f);
            fwrite(texture_data, header.data_size, 1, f);
            fclose(f);
        }

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, p2, p2, p2, 0, GL_RG, GL_FLOAT, texture_data);
    #endif

        free(external_layer);
        free(compute_data.octree);
        free(udf);
        free(external_udf);
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
    template<int threads> GLuint pnt_sdf_compute(int max_level, GLenum texture_unit, const char* file_name = 0)
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
        if (file_name)
        {
            tex3d_header_t header
            {
                .target = GL_TEXTURE_3D,
                .internal_format = GL_R32F,
                .format = GL_RED,
                .type = GL_FLOAT,
                .size = glm::ivec3(p2, p2, p2),
                .data_size = (uint32_t) texture_size * sizeof(GLfloat)
            };


            FILE* f = fopen(file_name, "wb");
            fwrite(&header, sizeof(tex3d_header_t), 1, f);
            fwrite(texture_data, header.data_size, 1, f);
            fclose(f);
        }


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
        unsigned int f = compute_data->triangle_index++;

        while (f < compute_data->F)
        {
            debug_msg("Processing triangle #%u", f);
            //===========================================================================================================================================================================================================
            // get the indices and the vertices of the triangle
            //===========================================================================================================================================================================================================
            unsigned int iA = compute_data->faces[f].x;
            unsigned int iB = compute_data->faces[f].y;
            unsigned int iC = compute_data->faces[f].z;

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

                double current_distance = atomic_min(compute_data->octree[node_index], distance_to_node);
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

                            atomic_min(compute_data->udf_texture[tex3d_index], distance_to_leaf);
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
            f = compute_data->triangle_index++;
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
            //===========================================================================================================================================================================================================
            // done ... proceed to next triangle
            //===========================================================================================================================================================================================================
            point = compute_data->point_index++;
        }
    }



    //===================================================================================================================================================================================================================
    // triangle extended unsigned distance field function computation data structure
    //===================================================================================================================================================================================================================
    struct tri_eudf_compute_data_t
    {
        uint32_t max_level;

        glm::dvec3* positions;
        glm::tvec3<index_t>* faces;
        uint32_t F;

        std::atomic_uint triangle_index;
        std::atomic<double>* octree;
        double* udf_texture;
        uint32_t* closest_tri;
    };

    //===================================================================================================================================================================================================================
    // triangle sdf -- atomic free implementation
    //===================================================================================================================================================================================================================
    template<int threads> GLuint tri_esdf_compute(int max_level, GLenum texture_unit, double delta, const char* file_name = 0)
    {
        //===============================================================================================================================================================================================================
        // step 1 :: create common compute structure with atomic counter to be used by all threads
        //===============================================================================================================================================================================================================
        tri_eudf_compute_data_t ext_compute_data;

        ext_compute_data.max_level = max_level;
        ext_compute_data.positions = positions;
        ext_compute_data.faces = faces;
        ext_compute_data.F = F;
        ext_compute_data.triangle_index = 0;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;

        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        ext_compute_data.octree      = (std::atomic<double>*) malloc(octree_size * sizeof(double));
        ext_compute_data.udf_texture = (double*) malloc(texture_size * threads * sizeof(double));
        ext_compute_data.closest_tri = (uint32_t*) malloc(texture_size * threads * sizeof(uint32_t));

        for(unsigned int i = 0; i < octree_size; ++i) ext_compute_data.octree[i] = diameter;

        //===============================================================================================================================================================================================================
        // step 2 :: launch threads to compute extended udf for the mesh itself
        //===============================================================================================================================================================================================================
        std::thread computation_thread[threads - 1];

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_eudf_compute_thread, &ext_compute_data, thread_id);

        tri_eudf_compute_thread(&ext_compute_data, threads - 1);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

        //===============================================================================================================================================================================================================
        // step 3 :: process and assemble computed distance fields into one
        //===============================================================================================================================================================================================================

        double* udf = ext_compute_data.udf_texture;
        uint32_t* closest_tri = ext_compute_data.closest_tri;

        for(unsigned int thread_id = 1; thread_id < threads; ++thread_id)
        {
            double* udf_chunk = ext_compute_data.udf_texture + texture_size * thread_id;
            uint32_t* closest_tri_chunk = ext_compute_data.closest_tri + texture_size * thread_id;

            for(uint32_t p = 0; p < texture_size; ++p)
            {
                if (udf_chunk[p] < udf[p])
                {
                    udf[p] = udf_chunk[p];
                    closest_tri[p] = closest_tri_chunk[p];
                }
            }
        }

        //===============================================================================================================================================================================================================
        // step 3 :: compute udf for the external shell of the mesh
        //===============================================================================================================================================================================================================
        glm::dvec3* external_layer = (glm::dvec3*) malloc(sizeof(glm::dvec3) * V);
        for(unsigned int v = 0; v < V; ++v)
            external_layer[v] = positions[v] + delta * normals[v];

        tri_udf_compute_data_t compute_data;

        double* external_udf = udf + texture_size;

        compute_data.max_level = max_level;
        compute_data.positions = external_layer;
        compute_data.faces = faces;
        compute_data.F = F;
        compute_data.triangle_index = 0;
        compute_data.udf_texture = (std::atomic<double>*) external_udf;
        compute_data.octree = ext_compute_data.octree;

        for(unsigned int i = 0; i < octree_size; ++i) compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id] = std::thread(sdf_compute_t::tri_udf_compute_thread, &compute_data);

        tri_udf_compute_thread(&compute_data);

        for (unsigned int thread_id = 0; thread_id < threads - 1; ++thread_id)
            computation_thread[thread_id].join();

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
        glm::vec4* texture_data = (glm::vec4*) malloc(texture_size * sizeof(glm::vec4));

        int index = 0;
        double scale = 1.0 / p2;
        int bound = p2 - 1;

        for(int w = -bound; w <= bound; w += 2)
        {
            for(int v = -bound; v <= bound; v += 2)
            {
                for(int u = -bound; u <= bound; u += 2)
                {
                    glm::dvec3 position = scale * glm::dvec3(u, v, w);
                    glm::uvec3 triangle = faces[closest_tri[index]];
                    glm::dvec3 A = positions[triangle.x];
                    glm::dvec3 B = positions[triangle.y];
                    glm::dvec3 C = positions[triangle.z];

                    glm::dvec4 g = tri_closest_point(position, A, B, C);

                    double inv_length = 1.0 / g.w;
                    glm::dvec3 n = inv_length * glm::dvec3(g);
                    glm::dvec4 q = glm::dvec4(n, g.w - glm::dot(n, position));

                    if (external_udf[index] > udf[index]) q = -q;

                    texture_data[index++] = glm::vec4(q);
                }
            }
        }

        if (file_name)
        {
            tex3d_header_t header
            {
                .target = GL_TEXTURE_3D,
                .internal_format = GL_RGBA32F,
                .format = GL_RGBA,
                .type = GL_FLOAT,
                .size = glm::ivec3(p2, p2, p2),
                .data_size = (uint32_t) texture_size * sizeof(glm::vec4)
            };


            FILE* f = fopen(file_name, "wb");
            fwrite(&header, sizeof(tex3d_header_t), 1, f);
            fwrite(texture_data, header.data_size, 1, f);
            fclose(f);
        }

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, p2, p2, p2, 0, GL_RGBA, GL_FLOAT, texture_data);

        free(external_layer);
        free(compute_data.octree);
        free(udf);
        free(texture_data);
        free(closest_tri);

        return texture_id;
    }

    //===================================================================================================================================================================================================================
    // implementation of the extended version of the main algorithm for unsigned distance function computation ::
    // the function takes a triangle and traverses (simultaneously modifying it) distance octree avoiding octree branches
    // that distances to this particular triangle will certainly not modify and updating relevant ones using atomic minimum operation
    // for the leaves (texture elements) the algorithm also saves the index of the closest triangle
    // to be later used for finding not just the value of the distance function but also its gradient
    // and storing linearized part of the distance function in a texel
    //===================================================================================================================================================================================================================

    static void tri_eudf_compute_thread(tri_eudf_compute_data_t* ext_compute_data, int thread_id)
    {
        const unsigned int MAX_LEVEL = 10;
        unsigned int max_level = ext_compute_data->max_level;
        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        double p2m1 = double(1 << (max_level - 1));
        double inv_p2 = 1.0 / p2;
        double cube_diameter = 2.0 * constants::sqrt3_d;

        double* field = ext_compute_data->udf_texture + texture_size * thread_id;
        uint32_t* closest_tri = ext_compute_data->closest_tri + texture_size * thread_id;

        for(uint32_t i = 0; i < texture_size; ++i)
            field[i] = diameter;

        //===============================================================================================================================================================================================================
        // get the index of the triangle this invocation will work on
        //===============================================================================================================================================================================================================
        unsigned int f = ext_compute_data->triangle_index++;

        while (f < ext_compute_data->F)
        {
            debug_msg("Processing triangle #%u", f);
            //===========================================================================================================================================================================================================
            // get the indices and the vertices of the triangle
            //===========================================================================================================================================================================================================
            unsigned int iA = ext_compute_data->faces[f].x;
            unsigned int iB = ext_compute_data->faces[f].y;
            unsigned int iC = ext_compute_data->faces[f].z;

            glm::dvec3 vA = ext_compute_data->positions[iA];
            glm::dvec3 vB = ext_compute_data->positions[iB];
            glm::dvec3 vC = ext_compute_data->positions[iC];

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

                double current_distance = atomic_min(ext_compute_data->octree[node_index], distance_to_node);
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

                            //===========================================================================================================================================================================================
                            // note : every thread has its own array for field values and closest face indices
                            // so we do not use atomics here
                            //===========================================================================================================================================================================================
                            if (field[tex3d_index] > distance_to_leaf)
                            {
                                field[tex3d_index] = distance_to_leaf;
                                closest_tri[tex3d_index] = f;
                            }
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
            f = ext_compute_data->triangle_index++;
        }
    }

};

#endif // _sdf_included_015145034650764357365074365343417534721056415465105143173
