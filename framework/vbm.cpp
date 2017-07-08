#include <cstring>
#include <cstdio>

#include "vbm.hpp"
#include "log.hpp"


bool VBObject::LoadFromVBM(const char * filename, int vertexIndex, int normalIndex, int texCoord0Index)
{
    FILE * f = fopen(filename, "rb");
    if (!f)
    {
        debug_msg("Cannot open %s. Exiting ... ", filename);
        return false;
    };

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char * data = new unsigned char [filesize];
    unsigned char * raw_data;
    fread(data, filesize, 1, f);
    fclose(f);

    VBM_HEADER* header = (VBM_HEADER *) data;
    raw_data = data + sizeof(VBM_HEADER) + header->num_attribs * sizeof(VBM_ATTRIB_HEADER) + header->num_frames * sizeof(VBM_FRAME_HEADER);
    VBM_ATTRIB_HEADER * attrib_header = (VBM_ATTRIB_HEADER *)(data + sizeof(VBM_HEADER));
    VBM_FRAME_HEADER * frame_header = (VBM_FRAME_HEADER *)(data + sizeof(VBM_HEADER) + header->num_attribs * sizeof(VBM_ATTRIB_HEADER));
    std::size_t total_data_size = 0;

    memcpy(&m_header, header, sizeof(VBM_HEADER));
    m_attrib = new VBM_ATTRIB_HEADER[header->num_attribs];
    memcpy(m_attrib, attrib_header, header->num_attribs * sizeof(VBM_ATTRIB_HEADER));
    m_frame = new VBM_FRAME_HEADER[header->num_frames];
    memcpy(m_frame, frame_header, header->num_frames * sizeof(VBM_FRAME_HEADER));

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

    debug_msg("Number of attributes : %d. Number of vertices : %d. Number of indices : %d.", header->num_attribs, header->num_vertices, header->num_indices);
    for (unsigned int i = 0; i < header->num_attribs; i++)
    {
        int attribIndex = (i == 0) ? vertexIndex :
                          (i == 1) ? normalIndex :
                          (i == 2) ? texCoord0Index : i;

        glVertexAttribPointer(attribIndex, m_attrib[i].components, m_attrib[i].type, GL_FALSE, 0, (GLvoid *)total_data_size);
        glEnableVertexAttribArray(attribIndex);
        total_data_size += m_attrib[i].components * sizeof(GLfloat) * header->num_vertices;
    };

    glBufferData(GL_ARRAY_BUFFER, total_data_size, raw_data, GL_STATIC_DRAW);

    if (header->num_indices) 
    {
        glGenBuffers(1, &ibo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
        unsigned int element_size;
        element_size = (header->index_type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLuint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, header->num_indices * element_size, data + total_data_size, GL_STATIC_DRAW);
    }

    delete [] data;
    return true;
}

void VBObject::Render(unsigned int frame_index, unsigned int instances)
{
    if (frame_index >= m_header.num_frames) return;

    glBindVertexArray(vao_id);
    if (instances) 
    {
        if (m_header.num_indices)
            glDrawElementsInstanced(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)), instances);
        else
            glDrawArraysInstanced(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count, instances);
    }
    else
    {
        if (m_header.num_indices)
            glDrawElements(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)));
        else
            glDrawArrays(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count);
    }
}

VBObject::~VBObject(void)
{
    glDeleteBuffers(1, &ibo_id);
    glDeleteBuffers(1, &vbo_id);
    glDeleteVertexArrays(1, &vao_id);
    delete [] m_attrib;
    delete [] m_frame;
}
