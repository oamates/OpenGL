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

AlgCMS::AlgCMS(Isosurface* i_fn)
    : m_fn(i_fn), m_sampled(false)
{
    m_octMinLvl = 2;
    m_octMaxLvl = 6;

    m_container[0] = Range(-1.0f, 1.0f);
    m_container[1] = Range(-1.0f, 1.0f);
    m_container[2] = Range(-1.0f, 1.0f);

    initSamples();
    initialize();
}

AlgCMS::AlgCMS(Isosurface *i_fn, const Range i_container[], unsigned int i_octreeDepth)
    : m_fn(i_fn), m_sampled(false)
{
    m_octMinLvl = 2;
    m_octMaxLvl = std::max(m_octMinLvl, i_octreeDepth);

    for(int i = 0; i < 3; ++i)
        m_container[i] = i_container[i];

    initSamples();
    initialize();
}

AlgCMS::AlgCMS(Isosurface *i_fn, const Range i_container[], unsigned int i_octreeBase, unsigned int i_octreeDepth)
    : m_fn(i_fn), m_sampled(false)
{
    m_octMaxLvl = std::max(static_cast<unsigned int>(4), i_octreeDepth);
    m_octMinLvl = std::min(i_octreeBase, m_octMaxLvl);

    for(int i = 0; i < 3; ++i) // xyz ranges of the container
        m_container[i] = i_container[i];

    initSamples();
    initialize();
}

void AlgCMS::initSamples()
{
    int numOfSamples = util::intPower(2, m_octMaxLvl) + 1; // samples = cells+1
    m_samples[0] = numOfSamples;
    m_samples[1] = numOfSamples;
    m_samples[2] = numOfSamples;
}

void AlgCMS::initialize()
{
    m_zeroApproximation = 2;
    m_complexSurfThresh = 0.6f;
    m_snapMedian = false;

    m_xMax = m_container[0].m_upper;
    m_xMin = m_container[0].m_lower;
    m_yMax = m_container[1].m_upper;
    m_yMin = m_container[1].m_lower;
    m_zMax = m_container[2].m_upper;
    m_zMin = m_container[2].m_lower;

    m_offsets[0] = fabs(m_xMax - m_xMin) / static_cast<float>(m_samples[0] - 1);
    m_offsets[1] = fabs(m_yMax - m_yMin) / static_cast<float>(m_samples[1] - 1);
    m_offsets[2] = fabs(m_zMax - m_zMin) / static_cast<float>(m_samples[2] - 1);

    // Resizing the samplingData array and proceeding with the sampling
#if CMS_DEBUG_LOG
    debug_msg("Sampling at (%u, %u, %u)", m_samples.m_x - 1, m_samples.m_y - 1, m_samples.m_z - 1);
#endif
    m_sampleData.resize(m_samples);
    m_sampleData.setBBox(m_container);

    m_edgeData.resize(m_samples); ///todo check
    m_edgeData.setBBox(m_container);

    // Creating a pointer to an Octree on the heaps
    m_octree = new Octree(m_samples, m_sampleData, m_octMinLvl, m_octMaxLvl, m_offsets, m_fn, m_complexSurfThresh);
}

void AlgCMS::setOctreeLevels(unsigned int i_min, unsigned int i_max)
{
    m_octMinLvl = i_min;
    m_octMaxLvl = i_max;

    initSamples();
    initialize();
}

void AlgCMS::getOctreeLevels(unsigned int* o_lvls)
{
    o_lvls[0] = m_octMinLvl;
    o_lvls[1] = m_octMaxLvl;
}

void AlgCMS::setSamples(int i_xSamp, int i_ySamp, int i_zSamp)
{
    m_samples[0] = i_xSamp;
    m_samples[1] = i_ySamp;
    m_samples[2] = i_zSamp;
    initialize();
}

void AlgCMS::getSamples(int *o_samps) const
{
    o_samps[0] = m_samples.m_x;
    o_samps[1] = m_samples.m_y;
    o_samps[2] = m_samples.m_z;
}

void AlgCMS::setZeroApproximation(unsigned int i_zeroApproximation)
{
    m_zeroApproximation = i_zeroApproximation;
}

int AlgCMS::getZeroApproximation() const
{
    return m_zeroApproximation;
}

void AlgCMS::setComplexSurfThresh(float i_complexSurfThresh)
{
    // Should I be clamping the value b/n -1 and 1
    m_complexSurfThresh = i_complexSurfThresh;
}

float AlgCMS::getComplexSurfThresh() const
{
    return m_complexSurfThresh;
}

bool AlgCMS::snapMedian() const
{
    return m_snapMedian;
}

void AlgCMS::setSnapMedian(bool snapMedian)
{
    m_snapMedian = snapMedian;
}

bool AlgCMS::extractSurface(Mesh& o_mesh)
{
    debug_msg("********************  CMS LOG BEGIN  ********************");

    time_t begin, end;
    time(&begin);                                                               // Sample the function if it is not already sampled

    if(!m_sampled)
    {
        sampleFunction();
        m_sampled = true;
    }

    time(&end);
    double sampleTime = static_cast<int>(difftime(end, begin));
    const char* sampleMsg = "Sampling time: ";
    util::printTime(sampleTime, sampleMsg);

    time(&begin);
  
    m_octree->buildOctree();                                                    // Calling the function that would recursively generate the octree
    time(&end);
    double octreeTime = static_cast<int>(difftime(end, begin));
    const char* octreeMsg = "Octree build time: ";
    util::printTime(octreeTime, octreeMsg);
  
    m_octreeRoot = m_octree->getRoot();                                         // Getting the octree root cell
  
    if(m_desiredCells.size() > 0)                                               // Only mesh certain cells if applicable
        fixDesiredChildren();

    time(&begin);
    cubicalMarchingSquaresAlg();                                                // Traversing the octree and creating components from each leaf cell
    tessellationTraversal(m_octreeRoot, o_mesh);                                // Traversing the tree again and meshing all components

#if CMS_DEBUG_LOG
    debug_msg("verts :: %u",  << (unsigned int) m_vertices.size());
#endif

  
    createMesh(o_mesh);                                                         // Loading the vertices onto the mesh

    time(&end);
    double algorithmTime = static_cast<int>(difftime(end, begin));
    const char* algorithmMsg = "Algorithm Time: ";
    util::printTime(algorithmTime, algorithmMsg);

    debug_msg("********************  CMS LOG END  ********************");
    return true;
}

bool AlgCMS::sampleFunction()
{
    assert((m_samples[0] > 8) && (m_samples[1] > 8) && (m_samples[2] > 8));

    for(int i = 0; i < m_samples.m_x; ++i)
    {
        const float tx = static_cast<float>(i) / static_cast<float>(m_samples.m_x - 1);
        const float xPos = m_xMin + (m_xMax - m_xMin) * tx;
        assert((xPos >= m_xMin) && (xPos <= m_xMax));

        for(int j = 0; j < m_samples.m_y; ++j)
        {
            const float ty = static_cast<float>(j) / static_cast<float>(m_samples.m_y - 1);
            const float yPos = m_yMin + (m_yMax - m_yMin) * ty;
            assert((yPos >= m_yMin) && (yPos <= m_yMax));

            for(int k = 0; k < m_samples.m_z; ++k)
            {
                const float tz = static_cast<float>(k) / static_cast<float>(m_samples.m_z - 1);
                const float zPos = m_zMin + (m_zMax - m_zMin) * tz;
                assert((zPos >= m_zMin) && (zPos <= m_zMax));

                float val = (*m_fn)(xPos, yPos, zPos);
                m_sampleData(i, j, k, val);
                assert(m_sampleData.getIndexAt(i, j, k) < m_sampleData.size());
            }
        }
    }

    m_sampled = true;
    return true;
}

void AlgCMS::makeFaceSegments(const Index3D inds[], Face *i_face)
{
    const uint8_t edges =                                                       // Aquiring the index of the edges based on the face corner samples
        (m_sampleData.getValueAt(inds[0]) < 0 ? 1 : 0) |
        (m_sampleData.getValueAt(inds[1]) < 0 ? 2 : 0) |
        (m_sampleData.getValueAt(inds[2]) < 0 ? 4 : 0) |
        (m_sampleData.getValueAt(inds[3]) < 0 ? 8 : 0);
  
    const int8_t e0a = EDGE_MAP[edges][0][0];                                   // The edges of the first strip are
    const int8_t e0b = EDGE_MAP[edges][0][1];
  
    if (e0a != -1)                                                              // If edge has data generate primary strip
        makeStrip(e0a, e0b, inds, i_face, 0);
  
    const int8_t e1a = EDGE_MAP[edges][1][0];                                   // The edges of the second strip are:
    const int8_t e1b = EDGE_MAP[edges][1][1];
  
    if (e1a != -1)                                                              // If Two Strips on a Face
        makeStrip(e1a, e1b, inds, i_face, 1);
}

Vec3 AlgCMS::findCrossingPoint(unsigned int quality, const Point& pt0, const Point& pt1)
{
    const float isoValue = m_fn->iso_level;

    Vec3 p0 = pt0.getPosition();
    float v0 = pt0.getValue();
    Vec3 p1 = pt1.getPosition();
    float v1 = pt1.getValue();

    float alpha = (isoValue - v0) / (v1 - v0);
  
    Vec3 pos;                                                                   // Interpolate
    pos.m_x = p0.m_x + alpha * (p1.m_x - p0.m_x);
    pos.m_y = p0.m_y + alpha * (p1.m_y - p0.m_y);
    pos.m_z = p0.m_z + alpha * (p1.m_z - p0.m_z);
  
    float val = (*m_fn)(pos.m_x, pos.m_y, pos.m_z);                             // Re-Sample
  
    Point pt(pos, val);                                                         // Save point

  
    if((fabs(isoValue - val) < EPSILON) || (quality == 0))                      // Return if good enough
        return pos;

    if(val < 0.0f)
    {
        if(v0 > 0.0f)
            pos = findCrossingPoint(quality - 1, pt, pt0);
        else if(v1 > 0.0f)
            pos = findCrossingPoint(quality - 1, pt, pt1);
    }
    else if(val > 0.0f)
    {
        if(v0 < 0.0f)
            pos = findCrossingPoint(quality - 1, pt0, pt);
        else if(v1 < 0.0f)
            pos = findCrossingPoint(quality - 1, pt1, pt);
    }

    return pos;
}

void AlgCMS::findGradient(Vec3& o_gradient, const Vec3 &i_dimensions, const Vec3& i_position)
{
    float val = (*m_fn)(i_position.m_x, i_position.m_y, i_position.m_z);
    float dx = (*m_fn)(i_position.m_x + i_dimensions.m_x, i_position.m_y, i_position.m_z);
    float dy = (*m_fn)(i_position.m_x, i_position.m_y + i_dimensions.m_y, i_position.m_z);
    float dz = (*m_fn)(i_position.m_x, i_position.m_y, i_position.m_z + i_dimensions.m_z);
    o_gradient = Vec3(dx - val, dy - val, dz - val);
}

void AlgCMS::findGradient(Vec3& o_gradient, const Vec3& i_dimensions, const Vec3& i_position, const float& i_value)
{
    float dx = (*m_fn)(i_position.m_x + i_dimensions.m_x, i_position.m_y, i_position.m_z);
    float dy = (*m_fn)(i_position.m_x, i_position.m_y + i_dimensions.m_y, i_position.m_z);
    float dz = (*m_fn)(i_position.m_x, i_position.m_y, i_position.m_z + i_dimensions.m_z);
    o_gradient = Vec3(dx - i_value, dy - i_value, dz - i_value);
}

int AlgCMS::getEdgesBetwixt(Range& o_range, const Index3D& pt0, const Index3D& pt1) const
{
    int direction = -1;                                                         // 0 - right(x), 1 - up(y), 2 - front(z)
    int diffX = abs(pt0.m_x - pt1.m_x);
    int diffY = abs(pt0.m_y - pt1.m_y);
    int diffZ = abs(pt0.m_z - pt1.m_z);
    
    if(diffX > 0)
    {
        int first = std::min(pt0.m_x, pt1.m_x);
        int last = std::max(pt0.m_x, pt1.m_x);
        o_range = Range(first, last);
        direction = 0;
    }
    else if(diffY > 0)
    {
        int first = std::min(pt0.m_y, pt1.m_y);
        int last = std::max(pt0.m_y, pt1.m_y);
        o_range = Range(first, last);
        direction = 1;
    }
    else if(diffZ > 0)
    {
        int first = std::min(pt0.m_z, pt1.m_z);
        int last = std::max(pt0.m_z, pt1.m_z);
        o_range = Range(first, last);
        direction = 2;
    }
    
    assert((direction >= 0) && (direction <= 2));
    return direction;
}

int AlgCMS::exactSignChangeIndex(const Range& range, int& dir, Index3D& ind0, Index3D& ind1) const
{
    Index3D firstIndex;                                                         // Checking for going from smaller to higher
    if(ind0[dir] == range.m_lower)
        firstIndex = ind0;
    else if(ind1[dir] == range.m_lower)
        firstIndex = ind1;
  
    if(fabs(range.m_lower-range.m_upper) == 1)                                  // If there are only two indices, return the first one
        return firstIndex[dir];

    Index3D indexer = firstIndex;                                               // Loop through all samples on the cell edge and find the sign change
    for(int i=range.m_lower; i<range.m_upper; ++i)
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

void AlgCMS::makeStrip(int edge0, int edge1, const Index3D inds[],  Face* i_face, int stripInd)
{
    assert((edge0 != -1) && (edge1 != -1));
    Strip s(false, edge0, edge1);
    populateStrip(s, inds, 0);                                                  // First edge of First Strip - e0a
    populateStrip(s, inds, 1);                                                  // Second edge of First Strip - e0b

    // todo :: Check for face sharp features here

    i_face->strips[stripInd] = s;                                               // Populate current face with the created strip
    i_face->skip = false;
}

void AlgCMS::populateStrip(Strip& o_s, const Index3D inds[], int index)
{
    const int8_t faceEdge = o_s.edge[index];                                    // Get the edge on the currently examined face
    Index3D ind_0 = inds[VERTEX_MAP[faceEdge][0]];
    Index3D ind_1 = inds[VERTEX_MAP[faceEdge][1]];
    
    Range range;                                                                // Get the range and the direction (of an edge block) which the edge represents
    int dir = getEdgesBetwixt(range, ind_0, ind_1);
    assert(abs(ind_0[dir] - ind_1[dir]) > 0);
    assert((ind_0[dir] == range.m_lower) || (ind_0[dir] == range.m_upper));
    assert((ind_1[dir] == range.m_lower) || (ind_1[dir] == range.m_upper));
    
    int signChange = exactSignChangeIndex(range, dir, ind_0, ind_1);            // Find the exact sign change on that bigger edge range, getting the index of the sample, just before the sign change = edge of change
    assert((signChange >= range.m_lower) && (signChange < range.m_upper));
    
    Index3D crossingIndex_0 = ind_0;                                            // Setting the exact two point indices between the zero crossing
    Index3D crossingIndex_1 = ind_0;
    crossingIndex_0[dir] = signChange;
    crossingIndex_1[dir] = signChange + 1;
    assert(m_sampleData.getValueAt(crossingIndex_0) * m_sampleData.getValueAt(crossingIndex_1) <= 0.0f);
    
    bool dupli = false;                                                         // Checking for duplicate vertices on the same edge
    if(m_edgeData.getValueAt(crossingIndex_0).empty == false)                   // check global datastructor edgeblock
    {
        if(m_edgeData.getValueAt(crossingIndex_0).m_edgeInds[dir] != -1)        // check exact global edge
        {
            o_s.data[index] = m_edgeData.getValueAt(crossingIndex_0).m_edgeInds[dir];
            o_s.block[index] = crossingIndex_0;
            o_s.dir[index] = dir;
            dupli = true;
        }
    }
    
    if(!dupli)                                                                  // If there is no previous vertex registered to that edge, proceed to find it.
        makeVertex(o_s, dir, crossingIndex_0, crossingIndex_1, index);
}

void AlgCMS::makeVertex(Strip& o_strip, const int& dir, const Index3D& crossingIndex0, const Index3D& crossingIndex1, int _i)
{
    Vec3 pos0 = m_sampleData.getPositionAt(crossingIndex0);                     // Make two points with the info provided and find the surface b/n them
    float val0 = m_sampleData.getValueAt(crossingIndex0);
    Point pt0(pos0, val0);
    
    Vec3 pos1 = m_sampleData.getPositionAt(crossingIndex1);
    float val1 = m_sampleData.getValueAt(crossingIndex1);
    Point pt1(pos1, val1);
    
    Vec3 crossingPoint = findCrossingPoint(m_zeroApproximation, pt0, pt1);      // Find the exact position and normal at the crossing point
    Vec3 normal;
    findGradient(normal, m_offsets, crossingPoint);
    normal.normalize();
    
    vertex_t vert;                                                              // Create a vertex from the info
    vert.position = crossingPoint;
    vert.normal = normal;
    m_vertices.push_back(vert);
    
    o_strip.data[_i] = m_vertices.size() - 1;                                   // Place the data onto the currect strip
    o_strip.block[_i] = crossingIndex0;
    o_strip.dir[_i] = dir;
    
    edge_block_t e = m_edgeData.getValueAt(crossingIndex0);                        // Put the data onto the global 3D array of edges
    if(e.empty)
        e.empty = false;
    assert(e.m_edgeInds[dir] == -1);
    e.m_edgeInds[dir] = m_vertices.size() - 1;
    m_edgeData.setValueAt(crossingIndex0, e);
}

void AlgCMS::segmentFromTwin(Face *face, std::vector<unsigned int> &o_comp, int lastData, int& currentEdge)
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

void AlgCMS::tessellateComponent(Mesh& o_mesh, std::vector<unsigned int>& component)
{
    vertex_t median;                                                            // todo :: maybe avg the normals too
    int numOfInds = component.size();
    assert(numOfInds >= 3);
    
    if(numOfInds == 3)                                                          // Three Indices - just tringulate
        makeTri(o_mesh, component);
    else if(numOfInds > 3)                                                      // More than three - find the median and make a fan
    {
                                                                                // todo :: use maybe if there are no sharp features???
        float x, y, z;
        x = y = z = 0.0f;

        for(unsigned int i = 0; i < component.size(); ++i)
        {
            Vec3 currentVert = m_vertices[component[i]].position;
            x += currentVert.m_x;
            y += currentVert.m_y;
            z += currentVert.m_z;
        }

        Vec3 medVert =  Vec3(x / static_cast<float>(numOfInds),
                             y / static_cast<float>(numOfInds),
                             z / static_cast<float>(numOfInds));
        
        if(m_snapMedian)                                                        // Snap the median point to the surface
        {
            float medVal = (*m_fn)(medVert.m_x,medVert.m_y,medVert.m_z);
            Vec3 medDimension = Vec3(0.5f * m_offsets[0], 0.5f * m_offsets[1], 0.5f * m_offsets[2]);
            Vec3 medGradient;
            findGradient(medGradient,medDimension,medVert,medVal);
            medGradient.normalize();
            medVert += -medGradient*medVal;
        }

        median.position = medVert;
        m_vertices.push_back(median);
        component.push_back(m_vertices.size() - 1);
        makeTriFan(o_mesh, component);                                          // Create a triangle fan based on the new mid point
    }
}

void AlgCMS::makeTri(Mesh& o_mesh, std::vector<unsigned int>& i_threeVertInds)
{
    for(int i = 0; i < 3; ++i)
        o_mesh.pushIndex(i_threeVertInds[i]);
}

void AlgCMS::makeTriFan(Mesh& o_mesh, std::vector<unsigned int>& i_cellVerts)
{
    for(unsigned i = 0; i < i_cellVerts.size() - 2; ++i)                        // -2 because median index is at (size-1) and we stich end to begin later
    {
        o_mesh.pushIndex(i_cellVerts[i_cellVerts.size() - 1]);
        o_mesh.pushIndex(i_cellVerts[i]);
        o_mesh.pushIndex(i_cellVerts[i + 1]);
    }
  
    o_mesh.pushIndex(i_cellVerts[i_cellVerts.size() - 1]);                      // Connecting the last and the first
    o_mesh.pushIndex(i_cellVerts[i_cellVerts.size() - 2]);
    o_mesh.pushIndex(i_cellVerts[0]);
}

void AlgCMS::makeTriSeq(Mesh& o_mesh, std::vector<unsigned int>& i_cellVertInds)
{
    std::vector<unsigned int> triVertInds;
    triVertInds.resize(3);
    for(unsigned int i = 0; i < i_cellVertInds.size() - 2; ++i)
    { 
        triVertInds[0] = i_cellVertInds[i + 1];
        triVertInds[1] = i_cellVertInds[i + 2];
        triVertInds[2] = i_cellVertInds[0];
        makeTri(o_mesh, triVertInds);
    }
}

void AlgCMS::tessellationTraversal(Cell* c, Mesh& m)
{
    if(!c) return;                                                              // Catching empty BRANCH nodes
  
    if(c->getState() == BRANCH)                                                 // If it is a BRANCH => go deeper
    {
        for(int i = 0; i < 8; ++i)
            tessellationTraversal(c->getChild(i), m);
    }
    else if(c->getState() == LEAF)                                              // If it is a LEAF => tessellate segment
    {
        if(m_desiredCells.size() > 0)                                           // Check for special cases when only certain cells need be tess.
            if(!isInDesired(c->m_id))
                return;
    
        for(unsigned i = 0; i < c->getComponents().size(); ++i)                 // todo :: just pass in references or pointers... (back and forth)
            tessellateComponent(m, c->getComponents()[i]);
    }
}

void AlgCMS::cubicalMarchingSquaresAlg()
{
    generateSegments(m_octree->getRoot());                                      // Traverse through the octree and generate segments for all LEAF cells
    editTransitionalFaces();                                                    // Resolving transitional faces
    traceComponent();                                                           // Trace the strips into segments and components
}

void AlgCMS::generateSegments(Cell* c)                                          // Generate the component data for a cell
{
    if(!c) return;                                                              // Catching empty BRANCH nodes
  
    if(c->getState() == BRANCH)                                                 // If it is a BRANCH => go deeper
    {
        for(int i = 0; i < 8; ++i)
            generateSegments(c->getChild(i));
    }
    else if(c->getState() == LEAF)                                              // If it is a LEAF => generate segment
    {
        Index3D indices[4];                                                     // For all the faces in this LEAF cell
        for(int f = 0; f < 6; ++f)
        {
            for(int v = 0; v < 4; ++v)                                          // Convert face vert to cell vert
            {
                const uint8_t vert = FACE_VERTEX[f][v];
                indices[v] = c->getPointInds()[vert];
            }
            c->getFaceAt(f)->strips.resize(2);
            makeFaceSegments(indices, c->getFaceAt(f));
        }
    }
}

void AlgCMS::editTransitionalFaces()
{
    std::vector<Cell*> cells = m_octree->getAllCells();
    
    for(unsigned int i = 0; i < cells.size(); ++i)                              // Loop through all cells and all faces and find every transitional face. Then pass it for getting the data from it's twin
        for(int j = 0; j < 6; ++j)
            if(cells[i]->getFaceAt(j)->state == TRANSIT_FACE)
                resolveTransitionalFace(cells[i]->getFaceAt(j));
}

void AlgCMS::traceComponent()
{
    std::vector<Cell*> cells = m_octree->getAllCells();
    
    for(unsigned int i = 0; i < m_octMaxLvl; ++i)                               // Trace the strips into segments and components. Loop through all cells and link components for all LEAF cells
    {
        for(unsigned int j = 0; j < cells.size(); ++j)
        {
            if(cells[j]->getSubdivLvl() == m_octMaxLvl-i)
            {
                if(cells[j]->getState() == LEAF)                                // Trace the Segments to form Component(s)
                {
                    std::vector<Strip> cellStrips;
                    std::vector< std::vector<unsigned int> > transitSegs;
                    std::vector<unsigned int> component;
                    
                    collectStrips(cells[j], cellStrips, transitSegs);           // Collect all the Strips from that cell
                    
                    while(cellStrips.size() > 0)                                // Link the strips into components
                    {
                        linkStrips(component, cellStrips, transitSegs);
                        cells[j]->pushComponent(component);
                        component.clear();
                    }
                }
            }
        }
    }
}

void AlgCMS::resolveTransitionalFace(Face *face)
{
    assert(m_octree->getCellAt(face->twin->cellInd)->getState() == BRANCH);     // Check if twin face belongs to a Branch cell as it should!
    assert(face->twin->state != TRANSIT_FACE);
  
    std::vector< std::vector<unsigned int> > transitSegs;                       // Get twin and traverse all it's children collecting all non-empty strips
    std::vector<Strip> allStrips;                                               // todo :: get only addresses??
    traverseFace(face->twin, allStrips);
  
    if(allStrips.size() == 0)                                                   // If there are no strips on twin face
    {
        face->state = LEAF_FACE;
        return;
    }

    do
    {
        std::vector<unsigned int> vertInds;
        Strip longStrip;
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

    for(unsigned int i = 0; i < transitSegs.size(); ++i)
    {
        for(unsigned j=0;j<transitSegs[i].size(); ++j)
            assert(transitSegs[i][j] < m_vertices.size());                      // Check for valid data
    }
    
    transitSegs.clear();                                                        // Clear the vectors
    allStrips.clear();
}

void AlgCMS::traverseFace(Face *face, std::vector<Strip>& transitStrips)
{
    if(!face) return;

    assert(face->state != TRANSIT_FACE);
    
    if(face->state == BRANCH_FACE)                                              // If it is a branch face, traverse through all it's children
    {
        for(int i= 0; i < 4; ++i)
            traverseFace(face->children[i], transitStrips);
    }
    else if(face->state == LEAF_FACE)                                           // If it is a LEAF face collect all valid strips
    {
    
        for(unsigned int i = 0; i < face->strips.size(); ++i)                   // Check all the strips in a face
        {
            if(face->strips[i].skip == false)
                transitStrips.push_back(face->strips[i]);
            else
                assert(face->strips[i].data[0] == -1);
        }
    }
}

void AlgCMS::collectStrips(Cell* c, std::vector<Strip> &o_cellStrips, std::vector<std::vector<unsigned int>>& o_transitSegs)
{
    for(int f = 0; f < 6; ++f)                                                  // Looping through all faces of the cell
    {
        if(c->getFaceAt(f)->state == LEAF_FACE)                                 // If it is a leaf face, just copy the full strips
        {
            for(unsigned int i = 0; i < c->getFaceAt(f)->strips.size(); ++i)
            {
                if(c->getFaceAt(f)->strips[i].data[0] != -1)                    // Check there is valid data
                    o_cellStrips.push_back(c->getFaceAt(f)->strips[i]);         // Create a temp strip and store the cell edges in it
            }
        }
        else if(c->getFaceAt(f)->state == TRANSIT_FACE)                         // For a Transitional Face, we must take the transit segment too as it contains all the data (vertices in between the start and end of strip)
        {
            if(!c->getFaceAt(f)->twin)                                          // Check if there is a valid twin
                break;

            assert(c->getFaceAt(f)->twin->strips.size() == c->getFaceAt(f)->twin->transitSegs.size());
      
            for(unsigned int i = 0; i < c->getFaceAt(f)->twin->strips.size(); ++i)            
            {
                if(c->getFaceAt(f)->twin->strips[i].data[0] != -1)              // Push the current strips and transit
                {
                    o_transitSegs.push_back(c->getFaceAt(f)->twin->transitSegs[i]);
                    o_cellStrips.push_back(c->getFaceAt(f)->twin->strips[i]);
                }
            }
        }
    }
    
    assert(o_cellStrips.size() > 0);                                            // A Leaf cell must have at least 3 strips to form a component unless it is has just a single looping component from a transit face
}

void AlgCMS::linkStrips(std::vector<unsigned int> &o_comp, std::vector<Strip> &strips, std::vector<std::vector<unsigned int>> &transitSegs)
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

void AlgCMS::insertDataFromTwin(std::vector<unsigned int>& o_comp, std::vector<std::vector<unsigned int>>& segs, Strip& str, bool& transit, int& addedInIter, const bool& backwards)
{
    for(unsigned int i = 0; i < segs.size(); ++i)                               // Loop through all the transitional strips and find the one corresponding to this strip
    {
        if(compareStripToSeg(str, segs[i]))                                     // Check if the strip's data matches the segment
        {
            if(backwards)
            {
                for(int j = segs[i].size() - 1; j > 0; --j)
                    o_comp.push_back(segs[i][j - 1]);
            }
            else
            {
                for(unsigned int j = 1; j < segs[i].size(); ++j)
                    o_comp.push_back(segs[i][j]);
            }

            segs.erase(segs.begin() + i);
            ++addedInIter;
            transit = true;
            break;
        }
    }
}

bool AlgCMS::compareStripToSeg(Strip& str, std::vector<unsigned int>& seg)
{
    int s0 = str.data[0];
    int s1 = str.data[1];
    return ((((int)seg.front() == s0) && ((int)seg.back() == s1)) || (((int)seg.front() == s1) && ((int)seg.back() == s0)));
}

void AlgCMS::createMesh(Mesh& o_mesh)
{
    for(unsigned i=0;i<m_vertices.size();++i)
        o_mesh.pushVertex(m_vertices[i].position.m_x, m_vertices[i].position.m_y, m_vertices[i].position.m_z);
}

bool AlgCMS::isInDesired(int _id)
{
    for(unsigned i = 0; i < m_desiredCells.size(); ++i)
        if(_id == m_desiredCells[i])
            return true;
    return false;
}

void AlgCMS::fixDesiredChildren()
{
    std::vector<int> temp = m_desiredCells;
    for(unsigned i = 0; i < temp.size(); ++i)
    {
        if(m_octree->getCellAt(temp[i])->getState() == BRANCH)
            traverseForDesired(m_octree->getCellAt(temp[i]));
    }
}

void AlgCMS::traverseForDesired(Cell* c)
{
    if(!c) return;

    if(c->getState() == BRANCH)
    {
        for(int i = 0; i < 8; ++i)
            traverseForDesired(c->getChild(i));
    }
    else if(c->getState() == LEAF)
        m_desiredCells.push_back(c->m_id);
}

} // namespace cms