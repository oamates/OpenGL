#include <cstdio>
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "log.hpp"
#include "vao.hpp"
#include "plato.hpp"
#include "nvtristrip.hpp"

struct uvec2_lex : public glm::uvec2
{
    uvec2_lex(GLuint a, GLuint b) : glm::uvec2(a, b) {};

    friend bool operator < (const uvec2_lex a, const uvec2_lex b)
    {
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
        if (a.x < b.x) return true;
        return false;
    }
};

void print_strips(const std::vector<uint32_t>& strips)
{
    printf("\nTotal size : %u\nStrip #1 : ", (uint32_t) strips.size());
    uint32_t index = 0;
    uint32_t s = 0;

    for(uint32_t i = 0; i < strips.size(); ++i)
    {
        uint32_t q = strips[index++];
        if (q == -1)
            printf("\nStrip #%u : ", ++s);
        else
            printf("%i ", q);
    }
    printf("\nNumber of strips :: %u", s);
}

/*
enum PrimType
{
    PT_LIST,
    PT_STRIP,
    PT_FAN
};

struct PrimitiveGroup
{
    PrimType type;
    unsigned int numIndices;
    unsigned short* indices;

    PrimitiveGroup() : type(PT_STRIP), numIndices(0), indices(0) {}
    ~PrimitiveGroup()
        { if(indices) delete[] indices; }
};

*/

int main(int argc, char* argv[])
{
    //================================================================================================================================================================================================================
    // Common data for stripifier test
    //================================================================================================================================================================================================================
    glm::u16vec3* faces;                          // dynamically allocated index array
    uint32_t F;

    //================================================================================================================================================================================================================
    // TEST #1 : plain grid mesh
    //================================================================================================================================================================================================================
    const uint32_t size_x = 5;
    const uint32_t size_y = 3;

    F = 2 * size_x * size_y;
    faces = (glm::u16vec3*) malloc (F * sizeof(glm::u16vec3));

    uint32_t index = 0;
    uint32_t q = size_x + 1;
    uint32_t p = 0;

    for (uint32_t y = 0; y < size_y; ++y)
    {
        for (uint32_t x = 0; x < size_x; ++x)
        {
            faces[index++] = glm::u16vec3(p, p + 1, q + 1);
            faces[index++] = glm::u16vec3(p, q + 1, q);
            ++p; ++q;
        }
        ++p; ++q;
    }



    EnableRestart(-1);
    SetCacheSize(512);

    PrimitiveGroup* primGroups;
    unsigned short numGroups;

    bool success = GenerateStrips(glm::value_ptr(faces[0]), 3 * F, &primGroups, &numGroups, true);

    if (success)
        exit_msg("Error generating strips");

    printf("Strips ::\n");

    for (unsigned short s = 0; s < numGroups; ++s)
    {
        PrimitiveGroup& group = primGroups[s];
        printf("\ttype :: %s (%u)\n\t\t", (group.type == PT_LIST) ? "LIST" : ((group.type == PT_STRIP) ? "STRIP" : "FAN"), group.numIndices);
        for (unsigned int i = 0; i < group.numIndices; ++i)
            printf(" %u", (int) group.indices[i]);
        printf("\n");
    }
    return 0;
}
