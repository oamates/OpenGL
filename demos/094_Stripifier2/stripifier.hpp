//=======================================================================================================================================================================================================================
// Simple triangle striper algoritm
//=======================================================================================================================================================================================================================

#ifndef _stripifier_included_24525864234690704368529702369836458726245347452872
#define _stripifier_included_24525864234690704368529702369836458726245347452872




//template<typename index_t> 
struct stripifier_t
{


    const uint8_t USED_BIT = 0x80;

    struct vertex_face_t
    {
        uint32_t vertex;
        uint32_t face;

        vertex_face_t(uint32_t vertex, uint32_t face) : vertex(vertex), face(face) {}
    };

    struct edge_face_t
    {
        glm::uvec2 edge;
        uint32_t face;
        
        edge_face_t(uint32_t A, uint32_t B, uint32_t face) : edge(A, B), face(face) {}
        edge_face_t(glm::uvec2 edge, uint32_t face) : edge(edge), face(face) {}
    };

    struct strip_info_t
    {
        uint32_t forward, backward;
        uint32_t neighbour_count[4];
        uint32_t evaluation;

        strip_info_t() : forward(0), backward(0) 
        {
            forward = 0; 
            backward = 0;
            for (int i = 0; i < 4; ++i) neighbour_count[i] = 0; 
            evaluation = 0;
        }

        int32_t weight()
        {
            uint32_t F = backward + forward;                                    // the number of triangular faces covered by the strip
            F += F;                                                    
            if (((backward & 1) == 0) || ((forward & 1) != 0)) return F;        
            return F - 2;                                                       // add -3 as a penalty if the first vertex has to be duplicated
        }
    };

    strip_info_t build_strip(vertex_face_t* strip, const glm::uvec3* faces, uint8_t* usage_flags, uint8_t* connectivity, const edge_face_t* first, const edge_face_t* last, uint32_t A, uint32_t B, uint32_t C, const uint8_t BITMASK, uint32_t F)
    {
        strip_info_t strip_info;

        const edge_face_t* l = first;
        const edge_face_t* r = last;
        bool flip = true;
        uint32_t x = C, y = B; 
        while (l <= r)                                                                           
        {                                               
            const edge_face_t* m = l + (r - l) / 2;

            if (m->edge.y < y) { l = m + 1; continue; } 
            if (m->edge.y > y) { r = m - 1; continue; }
            if (m->edge.x < x) { l = m + 1; continue; } 
            if (m->edge.x > x) { r = m - 1; continue; }

            uint32_t f = m->face;                                                         
            if (usage_flags[f] & (USED_BIT | BITMASK)) break;
                                         
            uint32_t z = faces[f].x + faces[f].y + faces[f].z - x - y;      
            strip[strip_info.forward++] = vertex_face_t(z, f);                                                                      
            usage_flags[f] |= BITMASK;                                                    

            //================================================================================================================================================================================================================
            // compute triangle evaluation
            //================================================================================================================================================================================================================
            


            uint32_t P, Q;

            if (flip)
                { P = z; Q = y; y = z; }
            else
                { P = x; Q = z; x = z; }

            //================================================================================================================================================================================================================
            // compute connectivity of the neighboring face
            //================================================================================================================================================================================================================
            l = first; r = last;
            while (l <= r)                                                                           
            {                                               
                const edge_face_t* m = l + (r - l) / 2;

                if (m->edge.y < Q) { l = m + 1; continue; } 
                if (m->edge.y > Q) { r = m - 1; continue; }
                if (m->edge.x < P) { l = m + 1; continue; } 
                if (m->edge.x > P) { r = m - 1; continue; }

                uint32_t f = m->face;

                if ((usage_flags[f] & USED_BIT) == 0)
                {
                    int32_t c = connectivity[f];
                    if (c > 3) c = 3;
                    strip_info.neighbour_count[c]++;
                }
                break;
            }

            flip = !flip;
            l = first; r = last;
        }

        //====================================================================================================================================================================================================================
        // compute connectivity of the face, adjacent to AC
        //====================================================================================================================================================================================================================
        l = first; r = last;
        while (l <= r)                                                                           
        {                                               
            const edge_face_t* m = l + (r - l) / 2;

            if (m->edge.y < C) { l = m + 1; continue; } 
            if (m->edge.y > C) { r = m - 1; continue; }
            if (m->edge.x < A) { l = m + 1; continue; } 
            if (m->edge.x > A) { r = m - 1; continue; }

            uint32_t f = m->face;

            if ((usage_flags[f] & USED_BIT) == 0)
            {
                int32_t c = connectivity[f];
                if (c > 3) c = 3;
                strip_info.neighbour_count[c]++;
            }
            break;
        }


        l = first; r = last; flip = false; x = B; y = A;
        while (l <= r)                                                                           
        {                                               
            const edge_face_t* m = l + (r - l) / 2;

            if (m->edge.y < y) { l = m + 1; continue; } 
            if (m->edge.y > y) { r = m - 1; continue; }
            if (m->edge.x < x) { l = m + 1; continue; } 
            if (m->edge.x > x) { r = m - 1; continue; }

            uint32_t f = m->face;                                                         
            if (usage_flags[f] & (USED_BIT | BITMASK)) break;
                                         
            uint32_t z = faces[f].x + faces[f].y + faces[f].z - x - y;      
            strip_info.backward++;
            strip[F - strip_info.backward] = vertex_face_t(z, f);                                                                      
            usage_flags[f] |= BITMASK;                                                    

            uint32_t P, Q;
            if (flip)
                { P = z; Q = y; y = z; }
            else
                { P = x; Q = z; x = z; }

            //================================================================================================================================================================================================================
            // compute connectivity of the neighboring face
            //================================================================================================================================================================================================================
            l = first; r = last;
            while (l <= r)                                                                           
            {                                               
                const edge_face_t* m = l + (r - l) / 2;

                if (m->edge.y < Q) { l = m + 1; continue; } 
                if (m->edge.y > Q) { r = m - 1; continue; }
                if (m->edge.x < P) { l = m + 1; continue; } 
                if (m->edge.x > P) { r = m - 1; continue; }

                uint32_t f = m->face;

                if ((usage_flags[f] & USED_BIT) == 0)
                {
                    int32_t c = connectivity[f];
                    if (c > 3) c = 3;
                    strip_info.neighbour_count[c]++;
                }
                break;
            }

            flip = !flip;
            l = first; r = last;
        }
        strip_info.neighbour_count[0] = strip_info.forward + strip_info.backward + 1 - strip_info.neighbour_count[1] - strip_info.neighbour_count[2] - strip_info.neighbour_count[3];
        return strip_info;
    }




  





/*
    std::vector<uint32_t> compute(const glm::uvec3* faces, uint32_t F)
    {       
        std::vector<uint32_t> strips;
        uint32_t E = 3 * F;
        strips.reserve(E);

        //====================================================================================================================================================================================================================
        // create edge-face list from the input data ...
        //====================================================================================================================================================================================================================
        edge_face_t* edge_face_list = (edge_face_t*) malloc(E * sizeof(edge_face_t));
        uint32_t index = 0;
        for (uint32_t f = 0; f < F; ++f)
        {
            edge_face_list[index++] = edge_face_t(faces[f].x, faces[f].y, f);
            edge_face_list[index++] = edge_face_t(faces[f].y, faces[f].z, f);
            edge_face_list[index++] = edge_face_t(faces[f].z, faces[f].x, f);
        }

        //====================================================================================================================================================================================================================
        // ... and sort it lexicographically for fast search
        //====================================================================================================================================================================================================================
        struct 
        {
            bool operator() (const edge_face_t& lhs, const edge_face_t& rhs)
            {   
                if (lhs.edge.y < rhs.edge.y) return true;
                if (lhs.edge.y > rhs.edge.y) return false;
                return lhs.edge.x < rhs.edge.x;
            }   
        } edge_compare;

        std::sort(edge_face_list, edge_face_list + 3 * F, edge_compare);

        //====================================================================================================================================================================================================================
        // allocate temporary memory to store two strips -- both forward and backward tails
        //====================================================================================================================================================================================================================
        vertex_face_t* strip_data = (vertex_face_t*) malloc(4 * F * sizeof(vertex_face_t));
        vertex_face_t* strip0f = &strip_data[0 * F];
        vertex_face_t* strip0b = &strip_data[1 * F];
        vertex_face_t* strip1f = &strip_data[2 * F];
        vertex_face_t* strip1b = &strip_data[3 * F];

        //====================================================================================================================================================================================================================
        // face usage flags
        //====================================================================================================================================================================================================================
        uint8_t* usage_flags = (uint8_t*) calloc (F, sizeof(uint8_t));
        const uint8_t BIT_A = 0x01, BIT_B = 0x02, BIT_C = 0x04;
        uint32_t unused = F;
        uint32_t f = 0;

        while(unused)
        {
            //================================================================================================================================================================================================================
            // get the first unused triangle
            //================================================================================================================================================================================================================
            while (usage_flags[f]) f++;

            uint32_t A = faces[f].x;
            uint32_t B = faces[f].y;
            uint32_t C = faces[f].z;

            usage_flags[f++] = USED_BIT;

            uint32_t w0, w1;
            uint32_t P = C, Q = A, R = B;

            strip_size_t size_0 = build_strip(strip0f, strip0b, faces, usage_flags, edge_face_list, edge_face_list + E - 1, C, A, B, BIT_A);
            w0 = size_0.weight();

            strip_size_t size_1 = build_strip(strip1f, strip1b, faces, usage_flags, edge_face_list, edge_face_list + E - 1, A, B, C, BIT_B);
            w1 = size_1.weight(); 

            if (w1 > w0)
            {
                w0 = w1;
                std::swap(size_0, size_1);
                std::swap(strip0b, strip1b);
                std::swap(strip0f, strip1f);
                P = A, Q = B, R = C;
            }
            for (uint32_t i = 0; i < size_1.backward; ++i) usage_flags[strip1b[i].face] = 0;
            for (uint32_t i = 0; i < size_1.forward;  ++i) usage_flags[strip1f[i].face] = 0;

            size_1 = build_strip(strip1f, strip1b, faces, usage_flags, edge_face_list, edge_face_list + E - 1, B, C, A, BIT_C);
            w1 = size_1.weight(); 

            if(w1 > w0)
            {
                std::swap(size_0, size_1);
                std::swap(strip0b, strip1b);
                std::swap(strip0f, strip1f);
                P = B, Q = C, R = A;
            }
            for (uint32_t i = 0; i < size_1.backward; ++i) usage_flags[strip1b[i].face] = 0;
            for (uint32_t i = 0; i < size_1.forward;  ++i) usage_flags[strip1f[i].face] = 0;

            if (size_0.forward & 1)                                                                     // is forward strip tail odd ?
            {
                for (uint32_t i = size_0.forward; i != 0; --i)
                { 
                    strips.push_back(strip0f[i - 1].vertex);
                    usage_flags[strip0f[i - 1].face] = USED_BIT;
                }
                strips.push_back(R);                    
                strips.push_back(Q);                    
                strips.push_back(P);                    
                for (uint32_t i = 0; i < size_0.backward; ++i)
                { 
                    strips.push_back(strip0b[i].vertex);
                    usage_flags[strip0b[i].face] = USED_BIT;
                }
            }
            else
            { 
                if ((size_0.backward & 1) != 0)
                    strips.push_back(strip0b[size_0.backward - 1].vertex);                              // duplicate first vertex if backward strip tail is even

                for (uint32_t i = size_0.backward; i != 0; --i)
                { 
                    strips.push_back(strip0b[i - 1].vertex); 
                    usage_flags[strip0b[i - 1].face] = USED_BIT;
                }
                strips.push_back(P);                     
                strips.push_back(Q);                     
                strips.push_back(R);                     
                for (uint32_t i = 0; i < size_0.forward; ++i)
                { 
                    strips.push_back(strip0f[i].vertex); 
                    usage_flags[strip0f[i].face] = USED_BIT;
                }
            }
            strips.push_back(-1);                                                                       // PRIMITIVE_RESTART_INDEX
            unused -= (size_0.forward + size_0.backward + 1);
        }

        free(strip_data);
        free(usage_flags);
        free(edge_face_list);
        strips.pop_back();
        return strips;
    }
*/



    std::vector<uint32_t> compute(const glm::uvec3* faces, uint32_t F)
    {       
        std::vector<uint32_t> strips;
        uint32_t E = 3 * F;
        strips.reserve(E);

        //====================================================================================================================================================================================================================
        // create edge-face list from the input data ...
        //====================================================================================================================================================================================================================
        edge_face_t* edge_face_list = (edge_face_t*) malloc(E * sizeof(edge_face_t));
        uint32_t index = 0;
        for (uint32_t f = 0; f < F; ++f)
        {
            edge_face_list[index++] = edge_face_t(faces[f].x, faces[f].y, f);
            edge_face_list[index++] = edge_face_t(faces[f].y, faces[f].z, f);
            edge_face_list[index++] = edge_face_t(faces[f].z, faces[f].x, f);
        }

        //====================================================================================================================================================================================================================
        // ... and sort it lexicographically for fast search
        //====================================================================================================================================================================================================================
        struct 
        {
            bool operator() (const edge_face_t& lhs, const edge_face_t& rhs)
            {   
                if (lhs.edge.y < rhs.edge.y) return true;
                if (lhs.edge.y > rhs.edge.y) return false;
                return lhs.edge.x < rhs.edge.x;
            }   
        } edge_compare;

        std::sort(edge_face_list, edge_face_list + 3 * F, edge_compare);

        //====================================================================================================================================================================================================================
        // build face connectivity info
        //====================================================================================================================================================================================================================

        uint8_t* connectivity = (uint8_t*) malloc(F * sizeof(uint8_t));
        uint32_t* one_connect = (uint32_t*) malloc(F * sizeof(uint32_t));

        uint32_t conn1_idx = 0;
        uint32_t conn1_cnt = 0; 
        
        for (uint32_t f = 0; f < F; ++f)
        {
            uint32_t A = faces[f].x;
            uint32_t B = faces[f].y;
            uint32_t C = faces[f].z;

            int32_t l, r;
            uint32_t x, y; 
            uint8_t n = 0;

            l = 0; r = E - 1; x = B; y = A; 

            while (l <= r)                                                                           
            {                                               
                int32_t m = (r + l) / 2;
                if (edge_face_list[m].edge.y < y) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.y > y) { r = m - 1; continue; }
                if (edge_face_list[m].edge.x < x) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.x > x) { r = m - 1; continue; }
                n++; break;
            }

            l = 0; r = E - 1; x = C; y = B; 
            while (l <= r)                                                                           
            {                                               
                int32_t m = (l + r) / 2;
                if (edge_face_list[m].edge.y < y) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.y > y) { r = m - 1; continue; }
                if (edge_face_list[m].edge.x < x) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.x > x) { r = m - 1; continue; }
                n++; break;
            }

            l = 0; r = E - 1; x = A; y = C; 
            while (l <= r)                                                                           
            {                                               
                int32_t m = (l + r) / 2;
                if (edge_face_list[m].edge.y < y) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.y > y) { r = m - 1; continue; }
                if (edge_face_list[m].edge.x < x) { l = m + 1; continue; } 
                if (edge_face_list[m].edge.x > x) { r = m - 1; continue; }
                n++; break;
            }

            connectivity[f] = n;
            if (n == 1) one_connect[conn1_cnt++] = f;
        }

        //====================================================================================================================================================================================================================
        // allocate temporary memory to store two strips -- both forward and backward tails
        //====================================================================================================================================================================================================================
        vertex_face_t* strip_data = (vertex_face_t*) malloc(2 * F * sizeof(vertex_face_t));
        vertex_face_t* strip0 = &strip_data[0];
        vertex_face_t* strip1 = &strip_data[F];

        //====================================================================================================================================================================================================================
        // face usage flags
        //====================================================================================================================================================================================================================
        uint8_t* usage_flags = (uint8_t*) calloc (F, sizeof(uint8_t));
        const uint8_t BIT_A = 0x01, BIT_B = 0x02, BIT_C = 0x04;
        uint32_t unused = F;
        uint32_t f = 0;

        while(unused)
        {
            //================================================================================================================================================================================================================
            // get the first unused triangle with connectivity 1
            //================================================================================================================================================================================================================
            while ((conn1_idx < conn1_cnt) && (usage_flags[one_connect[conn1_idx]] != 0)) conn1_idx++;
            while (usage_flags[f]) f++;

            uint32_t q = (conn1_idx < conn1_cnt) ? one_connect[conn1_idx++] : f++;
            usage_flags[q] = USED_BIT;

            uint32_t A = faces[q].x;
            uint32_t B = faces[q].y;
            uint32_t C = faces[q].z;

            int32_t w0, w1;
            uint32_t P = C, Q = A, R = B;

            //================================================================================================================================================================================================================
            // build ...CAB... strip
            //================================================================================================================================================================================================================
            strip_info_t info_0 = build_strip(strip0, faces, usage_flags, connectivity, edge_face_list, edge_face_list + E - 1, C, A, B, BIT_A, F);
            w0 = info_0.weight();

            //================================================================================================================================================================================================================
            // build ...ABC... strip
            //================================================================================================================================================================================================================
            strip_info_t info_1 = build_strip(strip1, faces, usage_flags, connectivity, edge_face_list, edge_face_list + E - 1, A, B, C, BIT_B, F);
            w1 = info_1.weight(); 

            //================================================================================================================================================================================================================
            // compare them, save the better one and discard the other + cleanup usage flags
            //================================================================================================================================================================================================================
            if (w1 > w0)
            {
                w0 = w1;
                std::swap(info_0, info_1);
                std::swap(strip0, strip1);
                P = A; Q = B; R = C;
            }
            for (uint32_t i = info_1.backward; i != 0; --i) usage_flags[strip1[F - i].face] = 0;
            for (uint32_t i = 0; i < info_1.forward; ++i) usage_flags[strip1[i].face] = 0;

            //================================================================================================================================================================================================================
            // build ...BCA... strip
            //================================================================================================================================================================================================================
            info_1 = build_strip(strip1, faces, usage_flags, connectivity, edge_face_list, edge_face_list + E - 1, B, C, A, BIT_C, F);
            w1 = info_1.weight(); 

            //================================================================================================================================================================================================================
            // compare it with the best of the first two, save the better one and discard the other + cleanup usage flags
            //================================================================================================================================================================================================================
            if(w1 > w0)
            {
                std::swap(info_0, info_1);
                std::swap(strip0, strip1);
                P = B; Q = C; R = A;
            }
            for (uint32_t i = info_1.backward; i != 0; --i) usage_flags[strip1[F - i].face] = 0;
            for (uint32_t i = 0; i < info_1.forward; ++i) usage_flags[strip1[i].face] = 0;

            //================================================================================================================================================================================================================
            // store the best strip to the vector
            //================================================================================================================================================================================================================
            uint32_t size = info_0.forward + info_0.backward + 1;                                          // the number of faces
            uint32_t s = strips.size();
            bool flip = true;

            if (info_0.forward & 1)                                                                     // is forward strip tail odd ?
            {
                for (uint32_t i = info_0.forward; i != 0; --i)
                { 
                    strips.push_back(strip0[i - 1].vertex);
                    usage_flags[strip0[i - 1].face] = USED_BIT;
                }
                strips.push_back(R);                    
                strips.push_back(Q);                    
                strips.push_back(P);                    
                for (uint32_t i = 0; i < info_0.backward; ++i)
                { 
                    strips.push_back(strip0[F - i - 1].vertex);
                    usage_flags[strip0[F - i - 1].face] = USED_BIT;
                }
            }
            else
            { 
                if (info_0.backward & 1)
                {
                    strips.push_back(strip0[F - info_0.backward].vertex);                              // duplicate first vertex if backward strip tail is even
                    s++; 
                    flip = false;
                }
                for (uint32_t i = info_0.backward; i != 0; --i)
                { 
                    strips.push_back(strip0[F - i].vertex); 
                    usage_flags[strip0[F - i].face] = USED_BIT;
                }
                strips.push_back(P);                     
                strips.push_back(Q);                     
                strips.push_back(R);                     
                for (uint32_t i = 0; i < info_0.forward; ++i)
                { 
                    strips.push_back(strip0[i].vertex); 
                    usage_flags[strip0[i].face] = USED_BIT;
                }
            }

            //================================================================================================================================================================================================================
            // update connectivity info
            //================================================================================================================================================================================================================
            for (uint32_t i = 0; i < size; ++i)
            {
                int32_t l = 0, r = E - 1;
                uint32_t x = strips[s + i + 2];
                uint32_t y = strips[s + i];
                if (flip) std::swap(x, y);
                while (l <= r)                                                                           
                {                                               
                    int32_t m = (l + r) / 2;
                    if (edge_face_list[m].edge.y < y) { l = m + 1; continue; } 
                    if (edge_face_list[m].edge.y > y) { r = m - 1; continue; }
                    if (edge_face_list[m].edge.x < x) { l = m + 1; continue; } 
                    if (edge_face_list[m].edge.x > x) { r = m - 1; continue; }
                    uint32_t w = edge_face_list[m].face;
                    if ((connectivity[w] == 2) && (usage_flags[w] == 0)) 
                        one_connect[conn1_cnt++] = w;
                    connectivity[w]--;
                    break;
                }
                flip = !flip;
            }
            strips.push_back(-1);                                                                       // PRIMITIVE_RESTART_INDEX
            unused -= size;
        }

        free(strip_data);
        free(usage_flags);
        free(edge_face_list);
        strips.pop_back();
        return strips;
    }
};

#endif // _stripifier_included_24525864234690704368529702369836458726245347452872