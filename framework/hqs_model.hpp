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
};

#endif // _hqs_model_included_103639641560235613475634786357156849375610563087634
