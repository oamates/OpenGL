
#include <cstdint>
#include <vector>
#include <map>

template<typename index_t> struct edge_t
{
    index_t a, b;                                                                                       // structure represents directed edge [ab]
    uint32_t face;                                                                                      // index of the face edge [ab] belongs to
    uint32_t inverse;                                                                                   // edge [ba]
    uint32_t left;                                                                                      // edge [ac]
    uint32_t right;                                                                                     // edge [cb]

    edge_t(index_t a, index_t b, uint32_t face) : a(a), b(b), face(face), inverse(-1), left(-1), right(-1) {}
};


template<typename index_t> struct trimesh_t
{
    uint32_t V, E, F, gE;
    const glm::tvec3<index_t>* faces;                                                     
    edge_t<index_t>* edges;
    uint8_t* face_flags;
    uint8_t* vertex_degree;

    trimesh_t(const glm::tvec3<index_t>* faces, uint32_t F) : F(F), faces(faces)
    {
        printf("Done0\n");
        fflush(stdout);        
        E = 3 * F;
        edges = (edge_t<index_t>*) malloc(E * sizeof(edge_t<index_t>));                                                                                                                                      
        face_flags = (uint8_t*) calloc(sizeof(uint8_t), F);
        vertex_degree = (uint8_t*) calloc(sizeof(uint8_t), V);

        build_edge_struct();
        printf("Done1\n");
        fflush(stdout);
        calculate_vertex_degree();

        printf("Done2\n");
        fflush(stdout);
        gE = 0;  
        for(uint32_t e = 0; e < E; ++e)
            if ((edges[e].inverse == -1) || ((edges[e].inverse != -1) && (edges[e].a < edges[e].b))) gE++;

        printf("Done3\n");
        fflush(stdout);
    }

    ~trimesh_t()
    {
        free(edges);
        free(face_flags);
        free(vertex_degree);
        printf("trimesh_t dtor\n");
        fflush(stdout);
    }    

    void count_edge_types()
    {
        uint32_t counters[3][3];
        for (uint32_t a = 0; a < 3; ++a)
            for (uint32_t b = 0; b < 3; ++b)
                counters[a][b] = 0;   

        for (uint32_t e = 0; e < E; ++e)
        {
            edge_t<index_t>& edge = edges[e];
            uint32_t deg_a = vertex_degree[edge.a];
            uint32_t deg_b = vertex_degree[edge.b];
            counters[deg_a % 3][deg_b % 3]++;
        }

        printf("\n\nEdge types :: \n");

        for (uint32_t a = 0; a < 3; ++a)
        {
            for (uint32_t b = 0; b < 3; ++b)
            {
                printf("counters[%u][%u] = %u ", a, b, counters[a][b]);
                counters[a][b] = 0;   
            }
            printf("\n");
        }
        printf("\n\n");
    }

    void print_vertex_statistics()
    {   
        printf("V = %u. E = %u. F = %u. Euler characteristic = %u.\n", V, gE, F, V - gE + F);
        std::map<uint8_t, uint32_t> counters;
        for(index_t v = 0; v < 1; ++v)
        {
            std::map<uint8_t, uint32_t>::iterator it = counters.find(vertex_degree[v]);
            if (it != counters.end())
                it->second++;
            else
                counters[vertex_degree[v]] = 1;        
        }

        for(std::map<uint8_t, uint32_t>::iterator it = counters.begin(); it != counters.end(); it++)
            printf("\t\tcounter[%u] = %u.\n", (uint32_t) it->first, it->second);
        printf("\n\n");

    }

    void print_all_strips()
    {
        for (uint32_t e = 0; e < E; ++e)
        {
            printf("Edge #%u \t:: [%u, %u] \t:: face = %d, \tinverse = %d, \tleft = %d, \tright = %d\n", e, edges[e].a, edges[e].b, edges[e].face, edges[e].inverse, edges[e].left, edges[e].right);

            int32_t boundary; 
            std::vector<index_t> strip0 = track_strip_no_turns_calc_boundary(&edges[e], boundary);
            memset(face_flags, 0, sizeof(uint8_t) * F);
            printf("\t\tboundary = %d.\n", boundary);

            uint32_t turns;
            std::vector<index_t> strip1 = track_strip_with_turns_calc_boundary(&edges[e], turns, boundary);
            memset(face_flags, 0, sizeof(uint8_t) * F);
            printf("\t\tturns = %d. boundary = %d.\n", turns, boundary);

            printf("\t\tRight-Left strip (%u) :: ", (uint32_t) strip0.size());
            for(uint32_t i = 0; i < strip0.size(); ++i)
                printf(" %u", strip0[i]);
            printf("\n\t\t\tStrip length :: %u.\n", strip_length_no_turns(&edges[e]));
            memset(face_flags, 0, sizeof(uint8_t) * F);

            printf("\t\tStrip with turns (%u) :: ", (uint32_t) strip1.size());
            for(uint32_t i = 0; i < strip1.size(); ++i)
                printf(" %u", strip1[i]);

            uint32_t l1 = strip_length_with_turns(&edges[e]);
            printf("\n\t\t\tStrip length :: %u. Turns :: %u. Turns (calculated) :: %u.\n", l1, strip1.size() - l1 - 2, turns);
            memset(face_flags, 0, sizeof(uint8_t) * F);

            printf("\n\n");

        }        
    }

    void print_edge_and_face_statistics()
    {
        uint32_t edge_counters[3][3];
        for (uint32_t a = 0; a < 3; ++a)
            for (uint32_t b = 0; b < 3; ++b)
                edge_counters[a][b] = 0;    

        for (uint32_t e = 0; e < E; ++e)
        {
            edge_t<index_t>& edge = edges[e];
            index_t a_mod3 = edge.a % 3;
            index_t b_mod3 = edge.b % 3;
            edge_counters[a_mod3][b_mod3]++;
        }
        for (uint32_t a = 0; a < 3; ++a)
        {            
            for (uint32_t b = 0; b < 3; ++b)
                printf("edge_counters[%u][%u] = %u.\t", a, b, edge_counters[a][b]);
            printf("\n");
        }
        printf("\n\n");
    }

    void print_ibo()
    {
        printf("\n\tIndex buffer = \n\t\t{ ");    
        for (uint32_t f = 0; f < F; ++f)
        {
            printf("(%u %u %u)", faces[f].x, faces[f].y, faces[f].z);
            if (((f + 1) & 0xF) == 0) printf("\n\t\t");
        }
        printf("}\n\n");
    }


    uint32_t strip_length_no_turns(const edge_t<index_t>* edge)
    {
        face_flags[edge->face] = 1;
        uint32_t l = 1;
        while ((edge->right != -1) && (face_flags[edges[edge->right].face] == 0))
        {
        
            edge = &edges[edge->right];
            face_flags[edge->face] = 1;
            ++l;
            if ((edge->left == -1) || (face_flags[edges[edge->left].face] != 0)) break;
            edge = &edges[edge->left];
            face_flags[edge->face] = 1;        
            ++l;
        }
        return l;
    }

    inline bool right_exit_free(const edge_t<index_t>* edge)
        { return (edge->right != -1) && (face_flags[edges[edge->right].face] == 0); }

    inline bool left_exit_free(const edge_t<index_t>* edge)
        { return (edge->left != -1) && (face_flags[edges[edge->left].face] == 0); }

    inline bool inverse_exit_free(const edge_t<index_t>* edge)
        { return (edge->inverse != -1) && (face_flags[edges[edge->inverse].face] == 0); }


    uint32_t strip_length_with_turns(const edge_t<index_t>* edge)
    {
        bool right_turn = true;
        uint32_t l = 1;
        
        while(true)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (right_exit_free(edge))                                                                  // does the right room exist and is it free?
                {
                    edge = &edges[edge->right];
                    right_turn = false;
                }
                else                                                                                        // try to turn left                                                                        
                {
                    if (!left_exit_free(edge)) break;                                                       // left room does not exist or is occupied, the strip must be terminated.
                    edge = &edges[edge->left];
                }
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (left_exit_free(edge))                                                                   // does the left room exist and is it free?
                {
                    edge = &edges[edge->left];
                    right_turn = true;
                }
                else                                                                                        // try to turn right
                {
                    if (!right_exit_free(edge)) break;                                                      // does the right room exist and is it free?
                    edge = &edges[edge->right];
                }
            }
            ++l;
        }
        return l;
    }

    std::vector<index_t> track_strip_no_turns(const edge_t<index_t>* edge)
    {
        std::vector<index_t> strip;
        
        bool right_turn = true;
        strip.push_back(edge->a);
        strip.push_back(edge->b);  
        
        while(true)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (!right_exit_free(edge)) break;                                                          // does the right room exist and is it free?
                edge = &edges[edge->right];
                strip.push_back(edge->a);                
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (!left_exit_free(edge)) break;                                                           // does the left room exist and is it free?
                edge = &edges[edge->left];
                strip.push_back(edge->b);      
            }
            right_turn = !right_turn;
        }
        uint32_t f = edge->face; 
        index_t last_vertex = faces[f].x + faces[f].y + faces[f].z - edge->a - edge->b;
        strip.push_back(last_vertex);

        return strip;
    }

    struct strip_record_t
    {
        uint32_t f;         // length of the forward part of the strip 
        uint32_t b;         // length of the backward part of the strip
        index_t* data;
    };
/*
    void track_strip_no_turns_double_sided(strip_record_t& strip, const edge_t<index_t>* edge)
    {
        std::vector<index_t> strip;
        
        bool right_turn = true;
        strip.f = 2;
        strip.data[0] = edge->a;
        strip.data[1] = edge->b;  
        
        while(true)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (!right_exit_free(edge)) break;                                                          // does the right room exist and is it free?
                edge = &edges[edge->right];
                strip.push_back(edge->a);                
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (!left_exit_free(edge)) break;                                                           // does the left room exist and is it free?
                edge = &edges[edge->left];
                strip.push_back(edge->b);      
            }
            right_turn = !right_turn;
        }
        uint32_t f = edge->face; 
        index_t last_vertex = faces[f].x + faces[f].y + faces[f].z - edge->a - edge->b;
        strip.push_back(last_vertex);

        return strip;
    }
*/
    std::vector<index_t> track_strip_no_turns_calc_boundary(const edge_t<index_t>* edge, int32_t& boundary)
    {
        std::vector<index_t> strip;
        
        bool right_turn = true;
        strip.push_back(edge->a);
        strip.push_back(edge->b);  

        boundary = inverse_exit_free(edge) ? 1 : -1;
        
        while(true)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (left_exit_free(edge))
                    boundary++;
                else
                    boundary--;    

                if (!right_exit_free(edge)) break;                                                          // does the right room exist and is it free?
                edge = &edges[edge->right];
                strip.push_back(edge->a);                
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (right_exit_free(edge))                                                                  
                    boundary++;
                else
                    boundary--;    

                if (!left_exit_free(edge)) break;                                                           // does the left room exist and is it free?
                edge = &edges[edge->left];
                strip.push_back(edge->b);      
            }
            right_turn = !right_turn;
        }

        uint32_t f = edge->face; 
        index_t last_vertex = faces[f].x + faces[f].y + faces[f].z - edge->a - edge->b;
        strip.push_back(last_vertex);
        boundary--;

        return strip;
    }

    std::vector<index_t> track_strip_with_turns_calc_boundary(const edge_t<index_t>* edge, uint32_t& turns, int32_t& boundary)
    {
        turns = 0;
        std::vector<index_t> strip;
        bool right_turn = true;
        
        strip.push_back(edge->a);
        strip.push_back(edge->b); 

        boundary = inverse_exit_free(edge) ? 1 : -1;
        
        while(true)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (right_exit_free(edge))                                                                  // does the right room exist and is it free?
                {
                    if (left_exit_free(edge))
                        boundary++;
                    else
                        boundary--;    

                    edge = &edges[edge->right];
                    strip.push_back(edge->a);                
                    right_turn = false;
                    continue;
                }

                boundary--;       
                if (!left_exit_free(edge)) break;                                                           // right exit is blocked, try to turn left and if it is also blocked the strip must be terminated
                turns++;                                                                               
                strip.push_back(edge->a);
                edge = &edges[edge->left];
                strip.push_back(edge->b);                
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (left_exit_free(edge))                                                                   // does the left room exist and is it free?
                {
                    if (right_exit_free(edge))
                        boundary++;
                    else
                        boundary--;    

                    edge = &edges[edge->left];
                    strip.push_back(edge->b);      
                    right_turn = true;
                    continue;
                }

                boundary--;       
                if (!right_exit_free(edge)) break;                                                          // left exit is blocked, try to turn right and if it is also blocked the strip must be terminated
                turns++;                                                                               
                strip.push_back(edge->b);
                edge = &edges[edge->right];
                strip.push_back(edge->a);
            }
        }

        uint32_t f = edge->face; 
        index_t last_vertex = faces[f].x + faces[f].y + faces[f].z - edge->a - edge->b;
        strip.push_back(last_vertex);
        boundary--;

        return strip;
    }


    std::vector<index_t> track_strip_with_turns(const edge_t<index_t>* edge, uint32_t& turns)
    {
        turns = 0;
        std::vector<index_t> strip;
        bool right_turn = true;
        
        strip.push_back(edge->a);
        strip.push_back(edge->b);    
        
        while(1)
        {
            face_flags[edge->face] = 1;
            if (right_turn)                                                                                 // must turn right
            {            
                if (right_exit_free(edge))                                                                  // does the right room exist and is it free?
                {
                    edge = &edges[edge->right];
                    strip.push_back(edge->a);                
                    right_turn = false;
                    continue;
                }

                if (!left_exit_free(edge)) break;                                                           // right exit is blocked, try to turn left and if it is also blocked the strip must be terminated
                turns++;                                                                               
                strip.push_back(edge->a);
                edge = &edges[edge->left];
                strip.push_back(edge->b);
            }
            else                                                                                            // must turn left                                                                                           
            {
                if (left_exit_free(edge))                                                                   // does the left room exist and is it free?
                {
                    edge = &edges[edge->left];
                    strip.push_back(edge->b);      
                    right_turn = true;
                    continue;
                }

                if (!right_exit_free(edge)) break;                                                          // left exit is blocked, try to turn right and if it is also blocked the strip must be terminated
                turns++;                                                                               
                strip.push_back(edge->b);
                edge = &edges[edge->right];
                strip.push_back(edge->a);
            }
        }

        uint32_t f = edge->face; 
        index_t last_vertex = faces[f].x + faces[f].y + faces[f].z - edge->a - edge->b;
        strip.push_back(last_vertex);
        return strip;
    }

    void build_edge_struct()
    {
        //====================================================================================================================================================================================================================
        // create edge structure array from the input data                                                                                                                                                                    
        //====================================================================================================================================================================================================================                                                                                                                                                                                                                              
        uint32_t index = 0;                                                                                                                                                                                                   
        V = 0;                                                                                                                                                                                                                
        for (uint32_t f = 0; f < F; ++f)                                                                                                                                                                                      
        {                                                                                                                                                                                                                     
            index_t a = faces[f].x,                                                                                                                                                                                           
                    b = faces[f].y,                                                                                                                                                                                           
                    c = faces[f].z;                                                                                                                                                                                           
            edges[index++] = edge_t<index_t>(a, b, f);                                                                                                                                                                        
            edges[index++] = edge_t<index_t>(b, c, f);                                                                                                                                                                        
            edges[index++] = edge_t<index_t>(c, a, f);                                                                                                                                                                        
            if (a > V) V = a;                                                                                                                                                                                                 
            if (b > V) V = b;                                                                                                                                                                                                 
            if (c > V) V = c;                                                                                                                                                                                                 
        }                                                                                                                                                                                                                     
        V++;                                                                                                                                                                                                                  
                                                                                                                                                                                                                              
        //====================================================================================================================================================================================================================
        // quick sort edges lexicographically, leaving unfilled (-1) members untouched                                                                                                                                        
        //====================================================================================================================================================================================================================                                                                                                                                                                                                                              
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests

        struct                                                                                                                                                                                                                
        {                                                                                                                                                                                                                     
            edge_t<index_t>* l;                                                                     // left index of the sub-array that needs to be sorted                                                                    
            edge_t<index_t>* r;                                                                     // right index of the sub-array to sort                                                                                   
        } _stack[STACK_SIZE];                                                                                                                                                                                                 
                                                                                                                                                                                                                              
        int sp = 0;                                                                                 // stack pointer, stack grows up not down                                                                                 
        _stack[sp].l = edges;                                                                                                                                                                                                 
        _stack[sp].r = edges + E - 1;                                                                                                                                                                                         
                                                                                                                                                                                                                              
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            edge_t<index_t>* l = _stack[sp].l;                                                                                                                                                                                
            edge_t<index_t>* r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                edge_t<index_t>* i = l;                                                                                                                                                                                       
                edge_t<index_t>* j = r;                                                                                                                                                                                       
                edge_t<index_t>* m = i + (j - i) / 2;                                                                                                                                                                         
                index_t a = m->a;                                                                                                                                                                                             
                index_t b = m->b;                                                                                                                                                                                             
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while ((i->b < b) || ((i->b == b) && (i->a < a))) i++;                          // lexicographic compare and proceed forward if less                                                                      
                    while ((j->b > b) || ((j->b == b) && (j->a > a))) j--;                          // lexicographic compare and proceed backward if less                                                                     
                                                                                                                                                                                                                              
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
                                                                                                                                                                                                                              
        //====================================================================================================================================================================================================================
        // fill adjacent edge indices, -1 will indicate boundary edges                                                                                                                                                        
        //====================================================================================================================================================================================================================
                                                                                                                                                                                                                              
        for (uint32_t e = 0; e < E; ++e)                                                                                                                                                                                      
        {                                                                                                                                                                                                                     
            edge_t<index_t>& edge = edges[e];                                                                                                                                                                                 
                                                                                                                                                                                                                              
            if ((edge.inverse != -1) && (edge.left != -1) && (edge.right != -1)) continue;                                                                                                                                    
                                                                                                                                                                                                                              
            glm::tvec3<index_t> face = faces[edge.face];                                                                                                                                                                      
            index_t a = edge.a;                                                                                                                                                                                               
            index_t b = edge.b;                                                                                                                                                                                               
            index_t c = face.x + face.y + face.z - a - b;                                                                                                                                                                     
            int32_t l, r;                                                                                                                                                                                                     
                                                                                                                                                                                                                              
            //================================================================================================================================================================================================================
            // triangle has edges [ab] [bc] [ca]                                                                                                                                                                              
            // binary search opposite [ba], left [ac] and right [cb] edges                                                                                                                                                    
            //================================================================================================================================================================================================================
                                                                                                                                                                                                                              
            if (edge.inverse == -1)                                                                 // search edge [ba]                                                                                                       
            {                                                                                                                                                                                                                 
                l = 0;                                                                                                                                                                                                        
                r = E - 1;                                                                                                                                                                                                    
                while (l <= r)                                                                                                                                                                                                
                {                                                                                                                                                                                                             
                    int32_t m = (r + l) / 2;                                                                                                                                                                                  
                                                                                                                                                                                                                              
                    if (edges[m].b < a) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].b > a) { r = m - 1; continue; }                                                                                                                                                              
                    if (edges[m].a < b) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].a > b) { r = m - 1; continue; }                                                                                                                                                              
                                                                                                                                                                                                                              
                    edge.inverse = m;                                                                                                                                                                                         
                    edges[m].inverse = e;                                                                                                                                                                                     
                    break;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
                                                                                                                                                                                                                              
            if (edge.left == -1)                                                                    // search edge [ac]                                                                                                       
            {                                                                                                                                                                                                                 
                l = 0;                                                                                                                                                                                                        
                r = E - 1;                                                                                                                                                                                                    
                while (l <= r)                                                                                                                                                                                                
                {                                                                                                                                                                                                             
                    int32_t m = (r + l) / 2;                                                                                                                                                                                  
                                                                                                                                                                                                                              
                    if (edges[m].b < c) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].b > c) { r = m - 1; continue; }                                                                                                                                                              
                    if (edges[m].a < a) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].a > a) { r = m - 1; continue; }                                                                                                                                                              
                                                                                                                                                                                                                              
                    edge.left = m;                                                                                                                                                                                            
                    uint32_t edge_ca = edges[m].inverse;                                                                                                                                                                      
                    if (edge_ca != -1)                                                                                                                                                                                        
                        edges[edge_ca].right = edge.inverse;                                                                                                                                                                  
                    break;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
                                                                                                                                                                                                                              
            if (edge.right == -1)                                                                   // search edge [cb]                                                                                                       
            {                                                                                                                                                                                                                 
                l = 0;                                                                                                                                                                                                        
                r = E - 1;                                                                                                                                                                                                    
                while (l <= r)                                                                                                                                                                                                
                {                                                                                                                                                                                                             
                    int32_t m = (r + l) / 2;                                                                                                                                                                                  
                                                                                                                                                                                                                              
                    if (edges[m].b < b) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].b > b) { r = m - 1; continue; }                                                                                                                                                              
                    if (edges[m].a < c) { l = m + 1; continue; }                                                                                                                                                              
                    if (edges[m].a > c) { r = m - 1; continue; }                                                                                                                                                              
                                                                                                                                                                                                                              
                    edge.right = m;                                                                                                                                                                                           
                    uint32_t edge_bc = edges[m].inverse;                                                                                                                                                                      
                    if (edge_bc != -1)                                                                                                                                                                                        
                        edges[edge_bc].left = edge.inverse;                                                                                                                                                                   
                    break;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
        }                                                                                                                                                                                                                     
    }

    void calculate_vertex_degree ()
    {
        for(uint32_t e = 0; e < E; ++e)
        {
            index_t a = edges[e].a;
            index_t b = edges[e].b;

            if ((edges[e].inverse == -1) || ((edges[e].inverse != -1) && (a < b)))
            {
                vertex_degree[a]++; 
                vertex_degree[b]++;
            }
        }
    }

};