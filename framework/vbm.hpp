#ifndef _vbm_included_289032512785062375231423543609238154268762124904693287342
#define _vbm_included_289032512785062375231423543609238154268762124904693287342

#include <GL/glew.h>
#include <glm/glm.hpp>

typedef struct VBM_HEADER_t
{
    unsigned int magic;
    unsigned int size;
    char name[64];
    unsigned int num_attribs;
    unsigned int num_frames;
    unsigned int num_vertices;
    unsigned int num_indices;
    unsigned int index_type;
} VBM_HEADER;

typedef struct VBM_ATTRIB_HEADER_t
{
    char name[64];
    unsigned int type;
    unsigned int components;
    unsigned int flags;
} VBM_ATTRIB_HEADER;

typedef struct VBM_FRAME_HEADER_t
{
    unsigned int first;
    unsigned int count;
    unsigned int flags;
} VBM_FRAME_HEADER;

struct VBObject
{
    GLuint vao_id, vbo_id, ibo_id;

    VBM_HEADER m_header;
    VBM_ATTRIB_HEADER* m_attrib;
    VBM_FRAME_HEADER* m_frame;


    VBObject() : vao_id(0), vbo_id(0), ibo_id(0), m_attrib(0), m_frame(0) {};

    bool LoadFromVBM(const char * filename, int vertexIndex, int normalIndex, int texCoord0Index);
    void Render(unsigned int frame_index = 0, unsigned int instances = 0);

    ~VBObject(void);

    unsigned int GetAttributeCount(void) const
        { return m_header.num_attribs; };

    const char * GetAttributeName(unsigned int index) const
        { return index < m_header.num_attribs ? m_attrib[index].name : 0; };

    unsigned int GetVertexCount(unsigned int frame = 0)
        { return frame < m_header.num_frames ? m_frame[frame].count : 0; };        

};

#endif // _vbm_included_289032512785062375231423543609238154268762124904693287342
