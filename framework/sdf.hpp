#ifndef _sdf_included_015145034650764357365074365343417534721056415465105143173
#define _sdf_included_015145034650764357365074365343417534721056415465105143173

#include "log.hpp"

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


/*
    void load_model(const char* file_name, double bbox_max)
    {
        vao_t::header_t header;
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

*/
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
        std::atomic_uint* octree;
        std::atomic_uint* udf_texture;
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
    template<int threads> GLuint tri_udf_compute(int max_level, GLenum texture_unit)
    {
        //===============================================================================================================================================================================================================
        // this must hold unless someone decided to put an extra auxiliary data to atomic structures
        //===============================================================================================================================================================================================================
        static_assert(sizeof(std::atomic_uint) == sizeof(unsigned int), "Atomic uint has larger size than uint. Not good.");

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
debug_msg("QQQQQQQQQQQQQQ1111111111111142532476987867543 :: V = %u", V); fflush(stdout);        
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

debug_msg("QQQQQQQQQQQQQQ111111111111114253247698 :: "); fflush(stdout);

        compute_data.positions = layer;
        compute_data.faces = faces;
        compute_data.F = F;
        compute_data.triangle_index = 0;

        unsigned int p2 = 1 << max_level;
        unsigned int texture_size = 1 << (3 * max_level);

        unsigned int octree_size = 0;
        unsigned int mip_size = 8;
debug_msg("QQQQQQQQQQQQQQ1111111111111"); fflush(stdout);
        for(unsigned int i = 0; i < max_level - 1; ++i)
        {
            octree_size += mip_size;
            mip_size <<= 3;
        }

        compute_data.octree      = (std::atomic_uint*) malloc( octree_size * sizeof(unsigned int));
        compute_data.udf_texture = (std::atomic_uint*) malloc(texture_size * sizeof(unsigned int));

        for(unsigned int i = 0; i < octree_size; ++i)  compute_data.octree[i] = diameter;
        for(unsigned int i = 0; i < texture_size; ++i) compute_data.udf_texture[i] = diameter;
debug_msg("QQQQQQQQQQQQQQ"); fflush(stdout);
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

        //===============================================================================================================================================================================================================
        // afoksha
        //===============================================================================================================================================================================================================
        tex3d_header_t header 
        {
            .target = GL_TEXTURE_3D,
            .internal_format = GL_R32F,
            .format = GL_RED,
            .type = GL_FLOAT,
            .size = glm::ivec3(p2, p2, p2),
            .data_size = (uint32_t) texture_size * sizeof(GLfloat)
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
            //===============================================================================================================================================================================================================
            // done ... proceed to next triangle
            //===============================================================================================================================================================================================================
            point = compute_data->point_index++;
        }
    }

};

#endif // _sdf_included_015145034650764357365074365343417534721056415465105143173
