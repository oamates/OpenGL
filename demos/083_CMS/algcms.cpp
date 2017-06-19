#include <ctime>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <algorithm>

#include "algcms.hpp"
#include "tables.hpp"
#include "util.hpp"

namespace cms
{

AlgCMS::AlgCMS(Isosurface *i_fn, const Range i_container[], unsigned int min_level, unsigned int max_level)
    : m_fn(i_fn)
{
    m_octMinLvl = min_level;
    m_octMaxLvl = max_level;

    for(int i = 0; i < 3; ++i) // xyz ranges of the container
        m_container[i] = i_container[i];

    int numOfSamples = (1 << m_octMaxLvl) + 1; // samples = cells+1
    m_samples[0] = numOfSamples;
    m_samples[1] = numOfSamples;
    m_samples[2] = numOfSamples;

    zero_search_iterations = 5;
    complex_surface_threshold = 0.6f;

    m_xMax = m_container[0].m_upper;
    m_xMin = m_container[0].m_lower;
    m_yMax = m_container[1].m_upper;
    m_yMin = m_container[1].m_lower;
    m_zMax = m_container[2].m_upper;
    m_zMin = m_container[2].m_lower;

    m_offsets[0] = fabs(m_xMax - m_xMin) / static_cast<float>(m_samples[0] - 1);
    m_offsets[1] = fabs(m_yMax - m_yMin) / static_cast<float>(m_samples[1] - 1);
    m_offsets[2] = fabs(m_zMax - m_zMin) / static_cast<float>(m_samples[2] - 1);
    
    m_sampleData.resize(m_samples);                                                                                     // Resizing the samplingData array and proceeding with the sampling
    m_sampleData.setBBox(m_container);

    m_edgeData.resize(m_samples);                                                                                       // todo :: check
    m_edgeData.setBBox(m_container);
    
    octree = new octree_t(m_samples, m_sampleData, m_octMinLvl, m_octMaxLvl, m_offsets, m_fn, complex_surface_threshold);     // Creating a pointer to an Octree on the heap

    assert((m_samples[0] > 8) && (m_samples[1] > 8) && (m_samples[2] > 8));

    for(int i = 0; i < m_samples.x; ++i)
    {
        const float tx = static_cast<float>(i) / static_cast<float>(m_samples.x - 1);
        const float xPos = m_xMin + (m_xMax - m_xMin) * tx;

        for(int j = 0; j < m_samples.y; ++j)
        {
            const float ty = static_cast<float>(j) / static_cast<float>(m_samples.y - 1);
            const float yPos = m_yMin + (m_yMax - m_yMin) * ty;

            for(int k = 0; k < m_samples.z; ++k)
            {
                const float tz = static_cast<float>(k) / static_cast<float>(m_samples.z - 1);
                const float zPos = m_zMin + (m_zMax - m_zMin) * tz;

                float val = (*m_fn)(xPos, yPos, zPos);
                m_sampleData(i, j, k, val);
            }
        }
    }

}

void AlgCMS::makeFaceSegments(const glm::ivec3 inds[], face_t* face)
{
    const uint8_t edges =                                                       // Aquiring the index of the edges based on the face corner samples
        (m_sampleData.getValueAt(inds[0]) < 0 ? 1 : 0) |
        (m_sampleData.getValueAt(inds[1]) < 0 ? 2 : 0) |
        (m_sampleData.getValueAt(inds[2]) < 0 ? 4 : 0) |
        (m_sampleData.getValueAt(inds[3]) < 0 ? 8 : 0);
  
    const int8_t e0a = EDGE_MAP[edges][0][0];                                   // The edges of the first strip are
    const int8_t e0b = EDGE_MAP[edges][0][1];
  
    if (e0a != -1)                                                              // If edge has data generate primary strip
        makeStrip(e0a, e0b, inds, face, 0);
  
    const int8_t e1a = EDGE_MAP[edges][1][0];                                   // The edges of the second strip are:
    const int8_t e1b = EDGE_MAP[edges][1][1];
  
    if (e1a != -1)                                                              // If Two Strips on a Face
        makeStrip(e1a, e1b, inds, face, 1);
}

int AlgCMS::getEdgesBetwixt(Range& o_range, const glm::ivec3& pt0, const glm::ivec3& pt1) const
{
    int direction = -1;                                                         // 0 - right(x), 1 - up(y), 2 - front(z)
    int diffX = abs(pt0.x - pt1.x);
    int diffY = abs(pt0.y - pt1.y);
    int diffZ = abs(pt0.z - pt1.z);
    
    if(diffX > 0)
    {
        int first = std::min(pt0.x, pt1.x);
        int last = std::max(pt0.x, pt1.x);
        o_range = Range(first, last);
        direction = 0;
    }
    else if(diffY > 0)
    {
        int first = std::min(pt0.y, pt1.y);
        int last = std::max(pt0.y, pt1.y);
        o_range = Range(first, last);
        direction = 1;
    }
    else if(diffZ > 0)
    {
        int first = std::min(pt0.z, pt1.z);
        int last = std::max(pt0.z, pt1.z);
        o_range = Range(first, last);
        direction = 2;
    }
    
    assert((direction >= 0) && (direction <= 2));
    return direction;
}

int AlgCMS::exactSignChangeIndex(const Range& range, int& dir, glm::ivec3& ind0, glm::ivec3& ind1) const
{
    glm::ivec3 firstIndex;                                                         // Checking for going from smaller to higher
    if(ind0[dir] == range.m_lower)
        firstIndex = ind0;
    else if(ind1[dir] == range.m_lower)
        firstIndex = ind1;
  
    if(fabs(range.m_lower-range.m_upper) == 1)                                  // If there are only two indices, return the first one
        return firstIndex[dir];

    glm::ivec3 indexer = firstIndex;                                               // Loop through all samples on the cell edge and find the sign change
    for(int i = range.m_lower; i < range.m_upper; ++i)
    {
        indexer[dir] = i;
        float thisValue = m_sampleData.getValueAt(indexer);

    
        indexer[dir] = i + 1;                                                   // incrementing the indexer so we get the value of the next pt on the edge
        float nextValue = m_sampleData.getValueAt(indexer);

    
        if(thisValue * nextValue <= 0.0f)                                       // Checking current value against next one, if negative, edge found return sign change index
            return i;
    }

    assert(true);
    return -1;                                                                  // Returning error value (no sign change found)
}





void AlgCMS::segmentFromTwin(face_t* face, std::vector<unsigned int> &o_comp, int lastData, int& currentEdge)
{
    assert(face->twin->state == BRANCH_FACE);
    assert(face->twin->transitSegs.size() > 0);
  
    for(unsigned i=0; i<face->twin->transitSegs.size(); ++i)                    // Looping through all transitional Face separate segments
    {
        if(face->twin->strips[i].skip == false)
        {
            if(lastData == face->twin->strips[i].data[0])
            {
                assert((face->twin->transitSegs[i][0] == (unsigned)face->twin->strips[i].data[0]) ||
                       (face->twin->transitSegs[i][0] == (unsigned)face->twin->strips[i].data[1]));

                for(unsigned int s = 1; s < face->twin->transitSegs[i].size(); ++s)
                    o_comp.push_back(face->twin->transitSegs[i][s]);
                currentEdge = face->twin->strips[i].edge[1];
                face->twin->strips[i].skip = true;
            }
            else if(lastData == face->twin->strips[i].data[1])
            {
                for(unsigned s=face->twin->transitSegs[i].size()-1; s>0; --s)
                    o_comp.push_back(face->twin->transitSegs[i][s-1]);
                currentEdge = face->twin->strips[i].edge[0];
                face->twin->strips[i].skip = true;
            }
        }
    }
}

void AlgCMS::resolveTransitionalFace(face_t* face)
{
    assert(octree->cells[face->twin->cell_index]->state == BRANCH);             // Check if twin face belongs to a Branch cell as it should!
    assert(face->twin->state != TRANSIT_FACE);
  
    std::vector< std::vector<unsigned int> > transitSegs;                       // Get twin and traverse all it's children collecting all non-empty strips
    std::vector<strip_t> allStrips;                                             // todo :: get only addresses??
    traverseFace(face->twin, allStrips);
  
    if(allStrips.size() == 0)                                                   // If there are no strips on twin face
    {
        face->state = LEAF_FACE;
        return;
    }

    do
    {
        std::vector<unsigned int> vertInds;
        strip_t longStrip;
        vertInds.push_back(allStrips[0].data[0]);
        vertInds.push_back(allStrips[0].data[1]);
        longStrip = allStrips[0];
        allStrips.erase(allStrips.begin());
        int addedInIteration;

        do                                                                      // Loop through all the segments, removing the visited ones until there are no more segments / there is a loop / or there are 0 new segments added
        {
            addedInIteration = 0;
            for(unsigned int i = 0; i < allStrips.size(); ++i)                  // Checking Forward
            {
                if(vertInds[vertInds.size()-1] == (unsigned int) allStrips[i].data[0])
                {
                    vertInds.push_back(allStrips[i].data[1]);
                    longStrip.changeBack(allStrips[i], 1);                      // adding info to longStrip end
                    ++addedInIteration;
                }
                else if(vertInds[vertInds.size()-1] == (unsigned int) allStrips[i].data[1])
                {
                    vertInds.push_back(allStrips[i].data[0]);
                    longStrip.changeBack(allStrips[i], 0);                      // adding info to longStrip end
                    ++addedInIteration;
                }
                else
                    continue;

                allStrips.erase(allStrips.begin() + i);                         // delete the currently added strip

                if(vertInds[0] == vertInds[vertInds.size()-1])
                {
                    vertInds.erase(vertInds.begin());                           // delete last vertex as it is duplex
                    longStrip.loop = true;
                }
            }

        }
        while((allStrips.size() > 0) && (addedInIteration > 0) && (longStrip.loop == false));
        
        if((longStrip.loop == false)&&(allStrips.size() > 0))                   // Continue if it is a full or looping strip
        {
            do                                                                  // Check Backward
            {
                addedInIteration = 0;

                for(unsigned int i = 0; i < allStrips.size(); ++i)
                {
                    if(vertInds[0] == (unsigned) allStrips[i].data[0])
                    {
                        vertInds.insert(vertInds.begin(), allStrips[i].data[1]);
                        longStrip.changeFront(allStrips[i], 1);                 // adding info to longStrip beginning
                        ++addedInIteration;
                    }
                    else if(vertInds[0] == (unsigned) allStrips[i].data[1])
                    {
                        vertInds.insert(vertInds.begin(), allStrips[i].data[0]);
                        longStrip.changeFront(allStrips[i], 0);                 // adding info to longStrip beginning
                        ++addedInIteration;
                    }
                    else
                        continue;

                    allStrips.erase(allStrips.begin()+i);                       // delete the currently added strip

                    if(vertInds[0] == vertInds[vertInds.size() - 1])
                    {
                        vertInds.erase(vertInds.begin());                       // delete last vertex as it is duplex
                        longStrip.loop = true;
                    }
                }
            }
            while((allStrips.size() > 0) && (addedInIteration > 0) && (longStrip.loop == false));
        }
    
        longStrip.skip = false;                                                 // Push the segment onto the face seg array
        face->twin->strips.push_back(longStrip);
        transitSegs.push_back(vertInds);
    }
    while(allStrips.size() != 0);
  
    if(transitSegs.size() != 0)                                                 // Load them segments onto the twin face
        face->twin->transitSegs = transitSegs;

    transitSegs.clear();                                                        // Clear the vectors
    allStrips.clear();
}


void AlgCMS::collectStrips(cell_t* c, std::vector<strip_t>& o_cellStrips, std::vector<std::vector<unsigned int>>& o_transitSegs)
{
    for(int f = 0; f < 6; ++f)                                                  // Looping through all faces of the cell
    {
        if(c->faces[f]->state == LEAF_FACE)                                     // If it is a leaf face, just copy the full strips
        {
            for(unsigned int i = 0; i < c->faces[f]->strips.size(); ++i)
            {
                if(c->faces[f]->strips[i].data[0] != -1)                        // Check there is valid data
                    o_cellStrips.push_back(c->faces[f]->strips[i]);             // Create a temp strip and store the cell edges in it
            }
        }
        else if(c->faces[f]->state == TRANSIT_FACE)                             // For a Transitional Face, we must take the transit segment too as it contains all the data (vertices in between the start and end of strip)
        {
            if(!c->faces[f]->twin)                                              // Check if there is a valid twin
                break;

            assert(c->faces[f]->twin->strips.size() == c->faces[f]->twin->transitSegs.size());
      
            for(unsigned int i = 0; i < c->faces[f]->twin->strips.size(); ++i)            
            {
                if(c->faces[f]->twin->strips[i].data[0] != -1)                  // Push the current strips and transit
                {
                    o_transitSegs.push_back(c->faces[f]->twin->transitSegs[i]);
                    o_cellStrips.push_back(c->faces[f]->twin->strips[i]);
                }
            }
        }
    }
    
    assert(o_cellStrips.size() > 0);                                            // A Leaf cell must have at least 3 strips to form a component unless it is has just a single looping component from a transit face
}

void AlgCMS::linkStrips(std::vector<unsigned int>& o_comp, std::vector<strip_t>& strips, std::vector<std::vector<unsigned int>> &transitSegs)
{
    assert(o_comp.size() == 0);

    int addedInIteration;
    bool backwards;
    
    o_comp.push_back(strips[0].data[0]);                                        // add a new value to the beginning

    do
    {
        addedInIteration = 0;
    
        for(unsigned int i = 0; i < strips.size(); ++i)                         // Loop through all current strips
        {
            int s_d0 = strips[i].data[0];
            int s_d1 = strips[i].data[1];

            // todo :: add a check for front and back like above?

            if(((int)o_comp.back() == s_d0) || ((int)o_comp.back() == s_d1))
            {
                if((int)o_comp.back() == s_d0)
                {
                    backwards = false;
                    bool transit = false;
          
                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(o_comp, transitSegs, strips[i], transit, addedInIteration, backwards);
          
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        o_comp.push_back(s_d1);
                        ++addedInIteration;
                    }
                }
                else if((int)o_comp.back() == s_d1)
                {
                    backwards = true;
                    bool transit = false;
          
                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(o_comp, transitSegs, strips[i], transit, addedInIteration, backwards); 
          
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        o_comp.push_back(s_d0);
                        ++addedInIteration;
                    }
                }
            }
            else
                continue;                                                       // skip to next iteration
      
            strips.erase(strips.begin() + i);                                   // Delete the currently added strip
        }
    
        if(o_comp.front() == o_comp.back())                                     // Check whether the component closes on itself
            o_comp.erase(o_comp.begin());                                       // delete first vertex as it is duplex with last


        for(unsigned int i = 0; i < o_comp.size(); ++i)
            assert(o_comp[i] < m_vertices.size());
    }
    while(addedInIteration > 0);

    do
    {
        addedInIteration = 0;

    
        for(unsigned int i = 0; i < strips.size(); ++i)                         // Loop through all current strips
        {
            int s_d0 = strips[i].data[0];
            int s_d1 = strips[i].data[1];

      
            if(((int)o_comp.front() == s_d0) || ((int)o_comp.front() == s_d1))  // Check Adding to Front
            {
                if((int)o_comp.front() == s_d0)
                {
                    backwards = false;
                    bool transit = false;

                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, checks for matching segment and insert from twin if found
                        insertDataFromTwin(o_comp, transitSegs, strips[i], transit, addedInIteration, backwards);

                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        o_comp.insert(o_comp.begin(), s_d1);
                        ++addedInIteration;
                    }
                }
                else if((int)o_comp.front() == s_d1)
                {
                    backwards = true;
                    bool transit = false;

                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(o_comp, transitSegs, strips[i], transit, addedInIteration, backwards);
          
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        o_comp.insert(o_comp.begin(), s_d0);
                        ++addedInIteration;
                    }
                }
            }
            else
                continue;                                                       // skip to next iteration
      
            strips.erase(strips.begin() + i);                                   // Delete the currently added strip
        }

    
        if(o_comp.front() == o_comp.back())                                     // Check whether the component closes on itself
            o_comp.erase(o_comp.begin());                                       // delete first vertex as it is duplex with last

        for(unsigned i=0;i<o_comp.size();++i)
            assert(o_comp[i] < m_vertices.size());
    }
    while(addedInIteration > 0);

    assert(o_comp.front() != o_comp.back());
    assert(o_comp.size() >= 3);
}




} // namespace cms