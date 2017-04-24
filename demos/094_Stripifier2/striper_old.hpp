
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

template<typename index_t> uint32_t strip_length_no_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags);
template<typename index_t> uint32_t strip_length_with_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags);
template<typename index_t> std::vector<index_t> track_strip_no_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags);
template<typename index_t> std::vector<index_t> track_strip_with_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags, uint32_t& turns);
template<typename index_t> uint8_t* calculate_vertex_degree (const edge_t<index_t>* edges, uint32_t E, uint32_t V);
template<typename index_t> edge_t<index_t>* build_edge_struct(const glm::tvec3<index_t>* faces, uint32_t F, uint32_t& V);

template<typename index_t> struct trimesh_t
{
    uint32_t V, E, F, gE;
    const glm::tvec3<index_t>* faces;                                                     
    edge_t<index_t>* edges;
    uint8_t* face_flags;
    uint8_t* vertex_degree;

    trimesh_t(const glm::tvec3<index_t>* faces, uint32_t F) : F(F), faces(faces)
    {
        E = 3 * F;
        edges = build_edge_struct(faces, F, V);
        face_flags = (uint8_t*) calloc(sizeof(uint8_t), F);
        vertex_degree = calculate_vertex_degree(edges, E, V);
        gE = 0;  
        for(uint32_t e = 0; e < E; ++e)
            if ((edges[e].inverse == -1) || ((edges[e].inverse != -1) && (edges[e].a < edges[e].b))) gE++;
    }

    void print_statistics()
    {
        printf("V = %u. E = %u. F = %u. Euler characteristic = %u.\n", V, gE, F, V - gE + F);

        std::map<uint32_t, uint32_t> counters;
        for(index_t v = 0; v < V; ++v)
        {
            std::map<uint32_t, uint32_t>::iterator it = counters.find(vertex_degree[v]);
            if (it != counters.end())
                it->second++;
            else
                counters[vertex_degree[v]] = 1;        
        }

        printf("\nVertex degree statistics :: \n\n");

        for(std::map<uint32_t, uint32_t>::iterator it = counters.begin(); it != counters.end(); it++)
            printf("\t\tcounter[%u] = %u.\n", it->first, it->second);
        printf("\n\n");
    }

    void print_all_strips()
    {
        for (uint32_t e = 0; e < E; ++e)
        {
            printf("Edge #%u \t:: [%u, %u] \t:: face = %d, \tinverse = %d, \tleft = %d, \tright = %d\n", e, edges[e].a, edges[e].b, edges[e].face, edges[e].inverse, edges[e].left, edges[e].right);

            std::vector<index_t> strip0 = track_strip_no_turns(edges, &edges[e], face_flags);
            memset(face_flags, 0, sizeof(uint8_t) * F);

            uint32_t turns;
            std::vector<index_t> strip1 = track_strip_with_turns(edges, &edges[e], face_flags, turns);
            memset(face_flags, 0, sizeof(uint8_t) * F);

            printf("\t\tRight-Left strip (%u) :: ", (uint32_t) strip0.size());
            for(uint32_t i = 0; i < strip0.size(); ++i)
                printf(" %u", strip0[i]);
            printf("\n\t\t\tStrip length :: %u.\n", strip_length_no_turns(edges, &edges[e], face_flags));
            memset(face_flags, 0, sizeof(uint8_t) * F);

            printf("\t\tStrip with turns (%u) :: ", (uint32_t) strip1.size());
            for(uint32_t i = 0; i < strip1.size(); ++i)
                printf(" %u", strip1[i]);

            uint32_t l1 = strip_length_with_turns(edges, &edges[e], face_flags);
            printf("\n\t\t\tStrip length :: %u. Turns :: %u. Turns (calculated) :: %u.\n", l1, strip1.size() - l1, turns);
            memset(face_flags, 0, sizeof(uint8_t) * F);

            printf("\n\n");

        }        
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

    ~trimesh_t()
    {
        free(edges);
        free(face_flags);
        free(vertex_degree);
    }
};

template<typename index_t> uint8_t* calculate_vertex_degree (const edge_t<index_t>* edges, uint32_t E, uint32_t V)
{
    uint8_t* vertex_degree = (uint8_t*) calloc(V, sizeof(uint8_t));
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
    return vertex_degree;
}



template<typename index_t> uint32_t strip_length_no_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags)
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

template<typename index_t> uint32_t strip_length_with_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags)
{
    bool right_turn = true;
    uint32_t l = 1;

    while(true)
    {
        face_flags[edge->face] = 1;
        if (right_turn)                                                                                 // must turn right
        {            
            if ((edge->right != -1) && (face_flags[edges[edge->right].face] == 0))                      // does the right room exist and is it free?
            {
                edge = &edges[edge->right];
                right_turn = false;
            }
            else                                                                                        // try to turn left                                                                        
            {
                if ((edge->left == -1) || (face_flags[edges[edge->left].face] != 0)) break;             // left room does not exist or is occupied, the strip must be finished.
                edge = &edges[edge->left];
            }
        }
        else                                                                                            // must turn left                                                                                           
        {
            if ((edge->left != -1) && (face_flags[edges[edge->left].face] == 0))                        // does the left room exist and is it free?
            {
                edge = &edges[edge->left];
                right_turn = true;
            }
            else                                                                                        // try to turn right
            {
                if ((edge->right == -1) || (face_flags[edges[edge->right].face] != 0)) break;           // does the right room exist and is it free?
                edge = &edges[edge->right];
            }
        }
        ++l;
    }
    return l;
}

template<typename index_t> std::vector<index_t> track_strip_no_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags)
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
            if ((edge->right == -1) || (face_flags[edges[edge->right].face] != 0)) break;               // does the right room exist and is it free?
            edge = &edges[edge->right];
            strip.push_back(edge->a);                
        }
        else                                                                                            // must turn left                                                                                           
        {
            if ((edge->left == -1) || (face_flags[edges[edge->left].face] != 0)) break;                 // does the left room exist and is it free?
            edge = &edges[edge->left];
            strip.push_back(edge->b);      
        }
        right_turn = !right_turn;
    }
    return strip;
}


template<typename index_t> std::vector<index_t> track_strip_with_turns(const edge_t<index_t>* edges, const edge_t<index_t>* edge, uint8_t* face_flags, uint32_t& turns)
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
            if ((edge->right != -1) && (face_flags[edges[edge->right].face] == 0))                      // does the right room exist and is it free?
            {
                edge = &edges[edge->right];
                strip.push_back(edge->a);                
                right_turn = false;
            }
            else                                                                                        // try to turn left                                                                        
            {
                if ((edge->left == -1) || (face_flags[edges[edge->left].face] != 0))                    // left room does not exist or is occupied, the strip must be finished.
                {
                    //                                                                                  // store the last strip vertex
                    break;    
                }        
                turns++;                                                                               
                strip.push_back(edge->a);
                edge = &edges[edge->left];
                strip.push_back(edge->b);
            }
        }
        else                                                                                            // must turn left                                                                                           
        {
            if ((edge->left != -1) && (face_flags[edges[edge->left].face] == 0))                        // does the left room exist and is it free?
            {
                edge = &edges[edge->left];
                strip.push_back(edge->b);      
                right_turn = true;
            }
            else                                                                                        // try to turn right
            {
                if ((edge->right == -1) || (face_flags[edges[edge->right].face] != 0))                  // does the right room exist and is it free?
                {
                    //                                                                                  // store the last strip vertex
                    break;
                }
                turns++;                                                                               
                strip.push_back(edge->b);
                edge = &edges[edge->right];
                strip.push_back(edge->a);
            }

        }
    }
    return strip;
}

//============================================================================================================================================================================================================================
// index_t should be one of GLubyte, GLushort or GLuint OpenGL unsigned integral index types
//============================================================================================================================================================================================================================

template<typename index_t> edge_t<index_t>* build_edge_struct(const glm::tvec3<index_t>* faces, uint32_t F, uint32_t& V)
{
    //========================================================================================================================================================================================================================
    // create edge structure array from the input data
    //========================================================================================================================================================================================================================
    uint32_t E = 3 * F;
    edge_t<index_t>* edges = (edge_t<index_t>*) malloc(E * sizeof(edge_t<index_t>));

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

    //========================================================================================================================================================================================================================
    // quick sort edges lexicographically, leaving unfilled (-1) members untouched 
    //========================================================================================================================================================================================================================

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

            if (j - l < r - i)                                                                  // push the larger one to stack and continue sorting the smaller one
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

    //========================================================================================================================================================================================================================
    // fill adjacent edge indices, -1 will indicate boundary edges
    //========================================================================================================================================================================================================================

    for (uint32_t e = 0; e < E; ++e)
    {
        edge_t<index_t>& edge = edges[e];

        if ((edge.inverse != -1) && (edge.left != -1) && (edge.right != -1)) continue;

        glm::tvec3<index_t> face = faces[edge.face];
        index_t a = edge.a;
        index_t b = edge.b;
        index_t c = face.x + face.y + face.z - a - b;
        int32_t l, r;

        //====================================================================================================================================================================================================================
        // triangle has edges [ab] [bc] [ca]
        // binary search opposite [ba], left [ac] and right [cb] edges
        //====================================================================================================================================================================================================================

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
    return edges;
}