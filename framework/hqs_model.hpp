#ifndef _hqs_model_included_103639641560235613475634786357156849375610563087634
#define _hqs_model_included_103639641560235613475634786357156849375610563087634

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
                positions.push_back(vertex);
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

    vao_t create_vao()
    {
        vertex_pn_t* vertices = (vertex_pn_t*) malloc(V * sizeof(vertex_pn_t));

        for (GLuint v = 0; v < V; ++v)
        {
            vertices[v].position = glm::vec3(positions[v]);
            vertices[v].normal = glm::vec3(normals[v]);
        }

        vao_t model_vao = vao_t(GL_TRIANGLES, vertices, V, glm::value_ptr(faces[0]), 3 * F);
        free(vertices);

        return model_vao;
    }

    void sort_faces_by_area()
    {
        double* area = (double*) malloc(F * sizeof(double));

        for (uint32_t f = 0; f < F; ++f)
        {
            glm::uvec3 triangle = faces[f];
            glm::dvec3 A = positions[triangle.x];
            glm::dvec3 B = positions[triangle.y];
            glm::dvec3 C = positions[triangle.z];

            area[f] = glm::length2(glm::cross(B - A, C - A));
        }    

        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

        struct                                                                                                                                                                                                                
        {                                                                                                                                                                                                                     
            uint32_t l;                                                                             // left index of the sub-array that needs to be sorted                                                                    
            uint32_t r;                                                                             // right index of the sub-array to sort                                                                                   
        } _stack[STACK_SIZE];                                                                                                                                                                                                 
                                                                                                                                                                                                                          
        int sp = 0;                                                                                 // stack pointer, stack grows up not down                                                                                 
        _stack[sp].l = 0;                                                                                                                                                                                                 
        _stack[sp].r = F - 1;                                                                                                                                                                                         
                                                                                                                                                                                                                          
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            //============================================================================================================================================================================================================
            // take the next subarray
            //============================================================================================================================================================================================================
            uint32_t l = _stack[sp].l;                                                                                                                                                                                
            uint32_t r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                //========================================================================================================================================================================================================
                // split it into 2 parts with elements less than m, and elements greater than m
                //========================================================================================================================================================================================================
                uint32_t i = l;                                                                                                                                                                                       
                uint32_t j = r;                                                                                                                                                                                       
                uint32_t m = (i + j) / 2;                                                                                                                                                                         
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while (area[i] > area[m]) i++;
                    while (area[j] < area[m]) j--;
                                                                                                                                                                                                                          
                    if (i <= j)                                                                                                                                                                                               
                    {                                                                                                                                                                                                         
                        std::swap(area[i], area[j]);                                                                                                                                                                                
                        std::swap(faces[i], faces[j]);                                                                                                                                                                                
                        i++;                                                                                                                                                                                                  
                        j--;                                                                                                                                                                                                  
                    }                                                                                                                                                                                                         
                }                                                                                                                                                                                                             
                while (i <= j);                                                                                                                                                                                               

                //========================================================================================================================================================================================================
                // push the larger interval to stack and continue sorting the smaller one
                // this way we will never need more than log2(length) stack entries
                //========================================================================================================================================================================================================
                if (j - l < r - i)                                                                  
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

            //============================================================================================================================================================================================================
            // check if anything in the stack is there to be sorted
            //============================================================================================================================================================================================================
        }                                                                                                                                                                                                                     
        while (sp >= 0);

        free(area);                                                                                                                                                                                                  
    }     

};

#endif // _hqs_model_included_103639641560235613475634786357156849375610563087634
