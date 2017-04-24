#include <cstring>
#include <cstdio>
#include <cassert>
#include <set>
#include "nvtristripobjects.hpp"
#include "vertexcache.hpp"

#define CACHE_INEFFICIENCY 6


//============================================================================================================================================================================================================================
// FindEdgeInfo() : find the edge info for these two indices
//============================================================================================================================================================================================================================
NvEdgeInfo* NvStripifier::FindEdgeInfo(NvEdgeInfoVec &edgeInfos, int v0, int v1)
{
    NvEdgeInfo *infoIter = edgeInfos[v0];                                                                                           // we can get to it through either array because the edge infos have a v0 and v1
                                                                                                                                    // and there is no order except how it was first created.
    while (infoIter != 0)
    {
        if (infoIter->m_v0 == v0)
        {
            if (infoIter->m_v1 == v1) return infoIter;
            infoIter = infoIter->m_nextV0;
        }
        else
        {
            assert(infoIter->m_v1 == v0);
            if (infoIter->m_v0 == v1) return infoIter;
            infoIter = infoIter->m_nextV1;
        }
    }
    return 0;
}

//============================================================================================================================================================================================================================
// FindOtherFace() : find the other face sharing these vertices exactly like the edge info above 
//============================================================================================================================================================================================================================
NvFaceInfo * NvStripifier::FindOtherFace(NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo)
{
    NvEdgeInfo* edgeInfo = FindEdgeInfo(edgeInfos, v0, v1);
    if((!edgeInfo) && (v0 == v1)) return 0;                                                                                         // we've hit a degenerate
    assert(edgeInfo != 0);
    return (edgeInfo->m_face0 == faceInfo ? edgeInfo->m_face1 : edgeInfo->m_face0);
}

bool NvStripifier::AlreadyExists(NvFaceInfo* faceInfo, NvFaceInfoVec& faceInfos)
{
    for(int i = 0; i < faceInfos.size(); ++i)
    {
        if((faceInfos[i]->m_v0 == faceInfo->m_v0) &&
           (faceInfos[i]->m_v1 == faceInfo->m_v1) &&
           (faceInfos[i]->m_v2 == faceInfo->m_v2) )
            return true;
    }
    return false;
}

//============================================================================================================================================================================================================================
// BuildStripifyInfo() : builds the list of all face and edge infos
//============================================================================================================================================================================================================================
void NvStripifier::BuildStripifyInfo(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, const unsigned short maxIndex)
{
    int numIndices = indices.size();                                                                                                // reserve space for the face infos, but do not resize them.
    faceInfos.reserve(numIndices / 3);
    
    
    edgeInfos.resize(maxIndex + 1);                                                                                                 // we actually resize the edge infos, so we must initialize to NULL
    for (int i = 0; i < maxIndex + 1; i++)
        edgeInfos[i] = 0;
    
    int numTriangles = numIndices / 3;                                                                                              // iterate through the triangles of the triangle list
    int index = 0;
    bool bFaceUpdated[3];

    for (int i = 0; i < numTriangles; i++)
    {   
        bool bMightAlreadyExist = true;
        bFaceUpdated[0] = false;
        bFaceUpdated[1] = false;
        bFaceUpdated[2] = false;
        
        int v0 = indices[index++];                                                                                                  // grab the indices
        int v1 = indices[index++];
        int v2 = indices[index++];
        
        if (IsDegenerate(v0, v1, v2)) continue;                                                                                     // we disregard degenerates
        
        NvFaceInfo *faceInfo = new NvFaceInfo(v0, v1, v2);                                                                          // create the face info and add it to the list of faces, but only if this exact face doesn't already exist in the list
        
        NvEdgeInfo *edgeInfo01 = FindEdgeInfo(edgeInfos, v0, v1);                                                                   // grab the edge infos, creating them if they do not already exist
        if (edgeInfo01 == 0)
        {
            bMightAlreadyExist = false;                                                                                             // since one of it's edges isn't in the edge data structure, it can't already exist in the face structure
            edgeInfo01 = new NvEdgeInfo(v0, v1);                                                                                    // create the info
            edgeInfo01->m_nextV0 = edgeInfos[v0];                                                                                   // update the linked list on both 
            edgeInfo01->m_nextV1 = edgeInfos[v1];
            edgeInfos[v0] = edgeInfo01;
            edgeInfos[v1] = edgeInfo01;
            
            // set face 0
            edgeInfo01->m_face0 = faceInfo;
        }
        else 
        {
            if (edgeInfo01->m_face1 != 0)
                printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
            else
            {
                edgeInfo01->m_face1 = faceInfo;
                bFaceUpdated[0] = true;
            }
        }
                
        NvEdgeInfo *edgeInfo12 = FindEdgeInfo(edgeInfos, v1, v2);                                                                   // grab the edge infos, creating them if they do not already exist
        if (edgeInfo12 == 0)
        {
            bMightAlreadyExist = false;
            edgeInfo12 = new NvEdgeInfo(v1, v2);                                                                                    // create the info
            edgeInfo12->m_nextV0 = edgeInfos[v1];                                                                                   // update the linked list on both 
            edgeInfo12->m_nextV1 = edgeInfos[v2];
            edgeInfos[v1] = edgeInfo12;
            edgeInfos[v2] = edgeInfo12;
            edgeInfo12->m_face0 = faceInfo;                                                                                         // set face 0
        }
        else 
        {
            if (edgeInfo12->m_face1 != 0)
                printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
            else
            {
                edgeInfo12->m_face1 = faceInfo;
                bFaceUpdated[1] = true;
            }
        }
        
        NvEdgeInfo *edgeInfo20 = FindEdgeInfo(edgeInfos, v2, v0);                                                                   // grab the edge infos, creating them if they do not already exist
        if (edgeInfo20 == 0)
        {
            bMightAlreadyExist = false;
            edgeInfo20 = new NvEdgeInfo(v2, v0);                                                                                    // create the info
            edgeInfo20->m_nextV0 = edgeInfos[v2];                                                                                   // update the linked list on both 
            edgeInfo20->m_nextV1 = edgeInfos[v0];
            edgeInfos[v2] = edgeInfo20;
            edgeInfos[v0] = edgeInfo20;
            edgeInfo20->m_face0 = faceInfo;                                                                                         // set face 0
        }
        else 
        {
            if (edgeInfo20->m_face1 != 0)
                printf("BuildStripifyInfo: > 2 triangles on an edge... uncertain consequences\n");
            else
            {
                edgeInfo20->m_face1 = faceInfo;
                bFaceUpdated[2] = true;
            }
        }

        if(bMightAlreadyExist)
        {
            if(!AlreadyExists(faceInfo, faceInfos))
                faceInfos.push_back(faceInfo);
            else
            {
                delete faceInfo;
                if (bFaceUpdated[0]) edgeInfo01->m_face1 = 0;                                                                       // cleanup pointers that point to this deleted face
                if (bFaceUpdated[1]) edgeInfo12->m_face1 = 0;
                if (bFaceUpdated[2]) edgeInfo20->m_face1 = 0;
            }
        }
        else
            faceInfos.push_back(faceInfo);
    }
}

//============================================================================================================================================================================================================================
// FindStartPoint() : finds a good starting point, namely one which has only one neighbor
//============================================================================================================================================================================================================================
int NvStripifier::FindStartPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos)
{
    int bestCtr = -1;
    int bestIndex = -1;

    for(int i = 0; i < faceInfos.size(); i++)
    {
        int ctr = 0;

        if (FindOtherFace(edgeInfos, faceInfos[i]->m_v0, faceInfos[i]->m_v1, faceInfos[i]) == 0) ctr++;
        if (FindOtherFace(edgeInfos, faceInfos[i]->m_v1, faceInfos[i]->m_v2, faceInfos[i]) == 0) ctr++;
        if (FindOtherFace(edgeInfos, faceInfos[i]->m_v2, faceInfos[i]->m_v0, faceInfos[i]) == 0) ctr++;

        if(ctr > bestCtr)
        {
            bestCtr = ctr;
            bestIndex = i;
        }
    }

    if (bestCtr == 0) return -1;
    return bestIndex;
}

//============================================================================================================================================================================================================================
// FindGoodResetPoint()
// A good reset point is one near other commited areas so that
// we know that when we've made the longest strips its because we're stripifying in the same general orientation.
//============================================================================================================================================================================================================================
NvFaceInfo* NvStripifier::FindGoodResetPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos)
{
                                                                                                                                    // We hop into different areas of the mesh to try to get other large open spans done. 
                                                                                                                                    // Areas of small strips can just be left to triangle lists added at the end.
    NvFaceInfo* result = 0;
    int numFaces   = faceInfos.size();
    int startPoint;
    if (bFirstTimeResetPoint)
    {
        startPoint = FindStartPoint(faceInfos, edgeInfos);                                                                          // first time, find a face with few neighbors (look for an edge of the mesh)
        bFirstTimeResetPoint = false;
    }
    else
        startPoint = (int)(((float) numFaces - 1) * meshJump);
    if (startPoint == -1)
        startPoint = (int)(((float) numFaces - 1) * meshJump);

    int i = startPoint;
    do
    {
        if (faceInfos[i]->m_stripId < 0)                                                                                            // if this guy isn't visited, try him
        {
            result = faceInfos[i];
            break;
        }
        if (++i >= numFaces) i = 0;                                                                                                 // update the index and clamp to 0-(numFaces-1)
        
    }
    while (i != startPoint);
    
    meshJump += 0.1f;                                                                                                               // update the meshJump
    if (meshJump > 1.0f) meshJump = .05f;
    
    return result;                                                                                                                  // return the best face we found
}

//============================================================================================================================================================================================================================
// GetUniqueVertexInB() : returns the vertex unique to faceB
//============================================================================================================================================================================================================================
int NvStripifier::GetUniqueVertexInB(NvFaceInfo *faceA, NvFaceInfo *faceB){
    
    int facev0 = faceB->m_v0;
    if (facev0 != faceA->m_v0 && facev0 != faceA->m_v1 && facev0 != faceA->m_v2) return facev0;
    
    int facev1 = faceB->m_v1;
    if (facev1 != faceA->m_v0 && facev1 != faceA->m_v1 && facev1 != faceA->m_v2) return facev1;
    
    int facev2 = faceB->m_v2;
    if (facev2 != faceA->m_v0 && facev2 != faceA->m_v1 && facev2 != faceA->m_v2) return facev2;
        
    return -1;                                                                                                                      // nothing is different
}

//============================================================================================================================================================================================================================
// GetSharedVertices() : returns the (at most) two vertices shared between the two faces
//============================================================================================================================================================================================================================
void NvStripifier::GetSharedVertices(NvFaceInfo *faceA, NvFaceInfo *faceB, int* vertex0, int* vertex1)
{
    *vertex0 = -1;
    *vertex1 = -1;

    int facev0 = faceB->m_v0;
    if (facev0 == faceA->m_v0 || facev0 == faceA->m_v1 || facev0 == faceA->m_v2)
        *vertex0 = facev0;
    
    int facev1 = faceB->m_v1;
    if (facev1 == faceA->m_v0 || facev1 == faceA->m_v1 || facev1 == faceA->m_v2)
    {
        if(*vertex0 != -1)
        {
            *vertex1 = facev1;
            return;
        }
        *vertex0 = facev1;
    }   

    int facev2 = faceB->m_v2;
    if (facev2 == faceA->m_v0 || facev2 == faceA->m_v1 || facev2 == faceA->m_v2)
    {
        if (*vertex0 != -1)
        {
            *vertex1 = facev2;
            return;            
        }
        *vertex0 = facev2;
    }
}

//============================================================================================================================================================================================================================
// GetNextIndex() : returns vertex of the input face which is "next" in the input index list
//============================================================================================================================================================================================================================
inline int NvStripifier::GetNextIndex(const WordVec &indices, NvFaceInfo *face)
{    
    int numIndices = indices.size();
    assert(numIndices >= 2);
    
    int v0  = indices[numIndices-2];
    int v1  = indices[numIndices-1];
    
    int fv0 = face->m_v0;
    int fv1 = face->m_v1;
    int fv2 = face->m_v2;
    
    if (fv0 != v0 && fv0 != v1)
    {
        if ((fv1 != v0 && fv1 != v1) || (fv2 != v0 && fv2 != v1))
        {
            printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
            printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
        }
        return fv0;
    }
    if (fv1 != v0 && fv1 != v1)
    {
        if ((fv0 != v0 && fv0 != v1) || (fv2 != v0 && fv2 != v1))
        {
            printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
            printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
        }
        return fv1;
    }
    if (fv2 != v0 && fv2 != v1)
    {
        if ((fv0 != v0 && fv0 != v1) || (fv1 != v0 && fv1 != v1))
        {
            printf("GetNextIndex: Triangle doesn't have all of its vertices\n");
            printf("GetNextIndex: Duplicate triangle probably got us derailed\n");
        }
        return fv2;
    }
    
    if((fv0 == fv1) || (fv0 == fv2)) return fv0;                                                                                    // shouldn't get here, but let's try and fail gracefully
    if((fv1 == fv0) || (fv1 == fv2)) return fv1;
    if((fv2 == fv0) || (fv2 == fv1)) return fv2;
    return -1;
}


//============================================================================================================================================================================================================================
// IsMarked() :
// If either the faceInfo has a real strip index because it is already assign to a committed strip OR it is assigned in an
// experiment and the experiment index is the one we are building for, then it is marked and unavailable
//============================================================================================================================================================================================================================
inline bool NvStripInfo::IsMarked(NvFaceInfo *faceInfo)
    { return (faceInfo->m_stripId >= 0) || (IsExperiment() && faceInfo->m_experimentId == m_experimentId); }

//============================================================================================================================================================================================================================
// MarkTriangle() : marks the face with the current strip ID
//============================================================================================================================================================================================================================
inline void NvStripInfo::MarkTriangle(NvFaceInfo *faceInfo)
{
    assert(!IsMarked(faceInfo));
    if (IsExperiment())
    {
        faceInfo->m_experimentId = m_experimentId;
        faceInfo->m_testStripId  = m_stripId;
    }
    else
    {
        assert(faceInfo->m_stripId == -1);
        faceInfo->m_experimentId = -1;
        faceInfo->m_stripId      = m_stripId;
    }
}

bool NvStripInfo::Unique(NvFaceInfoVec& faceVec, NvFaceInfo* face)
{
    bool bv0 = false, bv1 = false, bv2 = false;                                                                                     // bools to indicate whether a vertex is in the faceVec or not

    for(int i = 0; i < faceVec.size(); i++)
    {
        if ((!bv0) && ((faceVec[i]->m_v0 == face->m_v0) || (faceVec[i]->m_v1 == face->m_v0) || (faceVec[i]->m_v2 == face->m_v0))) bv0 = true;
        if ((!bv1) && ((faceVec[i]->m_v0 == face->m_v1) || (faceVec[i]->m_v1 == face->m_v1) || (faceVec[i]->m_v2 == face->m_v1))) bv1 = true;
        if ((!bv2) && ((faceVec[i]->m_v0 == face->m_v2) || (faceVec[i]->m_v1 == face->m_v2) || (faceVec[i]->m_v2 == face->m_v2))) bv2 = true;
        if (bv0 && bv1 && bv2) return false;                                                                                        // the face is not unique, all it's vertices exist in the face vector
    }
    
    return true;                                                                                                                    // if we get out here, it's unique
}

//============================================================================================================================================================================================================================
// Build() : builds a strip forward as far as we can go, then builds backwards, and joins the two lists
//============================================================================================================================================================================================================================
void NvStripInfo::Build(NvEdgeInfoVec &edgeInfos, NvFaceInfoVec &faceInfos)
{
    WordVec scratchIndices;                                                                                                         // used in building the strips forward and backward
    
    NvFaceInfoVec forwardFaces, backwardFaces;                                                                                      // build forward... start with the initial face
    forwardFaces.push_back(m_startInfo.m_startFace);

    MarkTriangle(m_startInfo.m_startFace);
    
    int v0 = (m_startInfo.m_toV1 ? m_startInfo.m_startEdge->m_v0 : m_startInfo.m_startEdge->m_v1);
    int v1 = (m_startInfo.m_toV1 ? m_startInfo.m_startEdge->m_v1 : m_startInfo.m_startEdge->m_v0);
        
    scratchIndices.push_back(v0);                                                                                                   // easiest way to get v2 is to use this function which requires the other indices to already be in the list.
    scratchIndices.push_back(v1);
    int v2 = NvStripifier::GetNextIndex(scratchIndices, m_startInfo.m_startFace);
    scratchIndices.push_back(v2);
    
    int nv0 = v1;                                                                                                                   // build the forward list
    int nv1 = v2;

    NvFaceInfo *nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, m_startInfo.m_startFace);
    while (nextFace != 0 && !IsMarked(nextFace))
    {        
        int testnv0 = nv1;                                                                                                          // check to see if this next face is going to cause us to die soon
        int testnv1 = NvStripifier::GetNextIndex(scratchIndices, nextFace);
        
        NvFaceInfo* nextNextFace = NvStripifier::FindOtherFace(edgeInfos, testnv0, testnv1, nextFace);

        if ((nextNextFace == 0) || (IsMarked(nextNextFace)))
        {
            
            NvFaceInfo* testNextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, testnv1, nextFace);                              // uh, oh, we're following a dead end, try swapping

            if(((testNextFace != 0) && !IsMarked(testNextFace)))
            {                
                NvFaceInfo* tempFace = new NvFaceInfo(nv0, nv1, nv0, true);                                                         // we only swap if it buys us something, add a "fake" degenerate face
                forwardFaces.push_back(tempFace);
                MarkTriangle(tempFace);
                scratchIndices.push_back(nv0);
                testnv0 = nv0;
                ++m_numDegenerates;
            }
        }
        
        forwardFaces.push_back(nextFace);                                                                                           // add this to the strip
        MarkTriangle(nextFace);
        scratchIndices.push_back(testnv1);                                                                                          // add the index
        nv0 = testnv0;                                                                                                              // and get the next face
        nv1 = testnv1;
        nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, nextFace);
    }   
    
    NvFaceInfoVec tempAllFaces;                                                                                                     // tempAllFaces is going to be forwardFaces + backwardFaces. it's used for Unique()
    for(int i = 0; i < forwardFaces.size(); i++)
        tempAllFaces.push_back(forwardFaces[i]);

    scratchIndices.resize(0);                                                                                                       // reset the indices for building the strip backwards and do so
    scratchIndices.push_back(v2);
    scratchIndices.push_back(v1);
    scratchIndices.push_back(v0);
    nv0 = v1;
    nv1 = v0;
    nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, m_startInfo.m_startFace);
    while (nextFace != 0 && !IsMarked(nextFace))
    {
        
        if(!Unique(tempAllFaces, nextFace)) break;                                                                                  // this tests to see if a face is "unique", strips which "wrap-around" are not allowed
        
        int testnv0 = nv1;                                                                                                          // check to see if this next face is going to cause us to die soon
        int testnv1 = NvStripifier::GetNextIndex(scratchIndices, nextFace);
        NvFaceInfo* nextNextFace = NvStripifier::FindOtherFace(edgeInfos, testnv0, testnv1, nextFace);

        if ((nextNextFace == 0) || (IsMarked(nextNextFace)))
        {
            NvFaceInfo* testNextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, testnv1, nextFace);                              // uh, oh, we're following a dead end, try swapping
            if( ((testNextFace != NULL) && !IsMarked(testNextFace)) )
            {
                NvFaceInfo* tempFace = new NvFaceInfo(nv0, nv1, nv0, true);                                                         // we only swap if it buys us something, add a "fake" degenerate face
                backwardFaces.push_back(tempFace);
                MarkTriangle(tempFace);
                scratchIndices.push_back(nv0);
                testnv0 = nv0;
                ++m_numDegenerates;
            }
        }

        backwardFaces.push_back(nextFace);                                                                                          // add this to the strip
        tempAllFaces.push_back(nextFace);                                                                                           // this is just so Unique() will work
        MarkTriangle(nextFace);
        scratchIndices.push_back(testnv1);                                                                                          // add the index
        nv0 = testnv0;                                                                                                              // and get the next face
        nv1 = testnv1;
        nextFace = NvStripifier::FindOtherFace(edgeInfos, nv0, nv1, nextFace);
    }
    
    Combine(forwardFaces, backwardFaces);                                                                                           // Combine the forward and backwards stripification lists and put into our own face vector
}

//============================================================================================================================================================================================================================
// Combine() : combines the two input face vectors and puts the result into m_faces
//============================================================================================================================================================================================================================
void NvStripInfo::Combine(const NvFaceInfoVec &forward, const NvFaceInfoVec &backward)
{    
    int numFaces = backward.size();                                                                                                 // add backward faces
    for (int i = numFaces - 1; i >= 0; i--)
        m_faces.push_back(backward[i]);
    
    numFaces = forward.size();                                                                                                      // add forward faces
    for (int i = 0; i < numFaces; i++)
        m_faces.push_back(forward[i]);
}

//============================================================================================================================================================================================================================
// SharesEdge() : returns true if the input face and the current strip share an edge
//============================================================================================================================================================================================================================
bool NvStripInfo::SharesEdge(const NvFaceInfo* faceInfo, NvEdgeInfoVec &edgeInfos)
{
    NvEdgeInfo* currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v0, faceInfo->m_v1);                                   // check v0->v1 edge   
    if (IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1)) return true;
    
    currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v1, faceInfo->m_v2);                                               // check v1->v2 edge
    if (IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1)) return true;
    
    currEdge = NvStripifier::FindEdgeInfo(edgeInfos, faceInfo->m_v2, faceInfo->m_v0);                                               // check v2->v0 edge
    if(IsInStrip(currEdge->m_face0) || IsInStrip(currEdge->m_face1)) return true;
    
    return false;
}

//============================================================================================================================================================================================================================
// CommitStrips() : commits the input strips by setting their m_experimentId to -1 and adding to the allStrips vector
//============================================================================================================================================================================================================================
void NvStripifier::CommitStrips(NvStripInfoVec& allStrips, const NvStripInfoVec& strips)
{   
    int numStrips = strips.size();                                                                                                  // Iterate through strips
    for (int i = 0; i < numStrips; i++)
    {        
        NvStripInfo *strip = strips[i];                                                                                             // Tell the strip that it is now real
        strip->m_experimentId = -1;
        allStrips.push_back(strip);                                                                                                 // add to the list of real strips        

        const NvFaceInfoVec &faces = strips[i]->m_faces;                                                                            // tell the faces of the strip that they belong to a real strip now
        int numFaces = faces.size();

        for (int j = 0; j < numFaces; j++)
            strip->MarkTriangle(faces[j]);
    }
}


//============================================================================================================================================================================================================================
// FindTraversal() : finds the next face to start the next strip on.
//============================================================================================================================================================================================================================
bool NvStripifier::FindTraversal(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, NvStripInfo *strip, NvStripStartInfo &startInfo)
{
    int v = (strip->m_startInfo.m_toV1 ? strip->m_startInfo.m_startEdge->m_v1 : strip->m_startInfo.m_startEdge->m_v0);              // if the strip was v0->v1 on the edge, then v1 will be a vertex in the next edge.
    
    NvFaceInfo *untouchedFace = 0;
    NvEdgeInfo *edgeIter      = edgeInfos[v];

    while (edgeIter != 0)
    {
        NvFaceInfo *face0 = edgeIter->m_face0;
        NvFaceInfo *face1 = edgeIter->m_face1;
        if ((face0 != 0 && !strip->IsInStrip(face0)) && face1 != 0 && !strip->IsMarked(face1))
        {
            untouchedFace = face1;
            break;
        }
        if ((face1 != 0 && !strip->IsInStrip(face1)) && face0 != 0 && !strip->IsMarked(face0))
        {
            untouchedFace = face0;
            break;
        }
        edgeIter = (edgeIter->m_v0 == v ? edgeIter->m_nextV0 : edgeIter->m_nextV1);                                                 // find the next edgeIter
    }
    
    startInfo.m_startFace = untouchedFace;
    startInfo.m_startEdge = edgeIter;
    if (edgeIter != 0)
    {
        if(strip->SharesEdge(startInfo.m_startFace, edgeInfos))
            startInfo.m_toV1 = (edgeIter->m_v0 == v);                                                                               // note! used to be m_v1
        else
            startInfo.m_toV1 = (edgeIter->m_v1 == v);
    }
    return (startInfo.m_startFace != 0);
}

//============================================================================================================================================================================================================================
// RemoveSmallStrips()
// allStrips is the whole strip vector...all small strips will be deleted from this list, to avoid leaking mem
// allBigStrips is an out parameter which will contain all strips above minStripLength
// faceList is an out parameter which will contain all faces which were removed from the striplist
//============================================================================================================================================================================================================================
void NvStripifier::RemoveSmallStrips(NvStripInfoVec& allStrips, NvStripInfoVec& allBigStrips, NvFaceInfoVec& faceList)
{
    faceList.clear();
    allBigStrips.clear();                                                                                                           // make sure these are empty
    NvFaceInfoVec tempFaceList;
    
    for(int i = 0; i < allStrips.size(); i++)
    {
        if(allStrips[i]->m_faces.size() < minStripLength)
        {
            for(int j = 0; j < allStrips[i]->m_faces.size(); j++)                                                                   // strip is too small, add faces to faceList
                tempFaceList.push_back(allStrips[i]->m_faces[j]);            
            delete allStrips[i];                                                                                                    // and free memory
        }
        else
            allBigStrips.push_back(allStrips[i]);
    }
    
    if(tempFaceList.size())
    {
        bool *bVisitedList = new bool[tempFaceList.size()];
        memset(bVisitedList, 0, tempFaceList.size() * sizeof(bool));
        
        VertexCache* vcache = new VertexCache(cacheSize);
        
        int bestNumHits = -1;
        int numHits;
        int bestIndex;
        
        while(1)
        {
            bestNumHits = -1;            
            for (int i = 0; i < tempFaceList.size(); i++)                                                                           // find best face to add next, given the current cache
            {
                if(bVisitedList[i])
                    continue;
                
                numHits = CalcNumHitsFace(vcache, tempFaceList[i]);
                if(numHits > bestNumHits)
                {
                    bestNumHits = numHits;
                    bestIndex = i;
                }
            }

            if (bestNumHits == -1.0f) break;
            bVisitedList[bestIndex] = true;
            UpdateCacheFace(vcache, tempFaceList[bestIndex]);
            faceList.push_back(tempFaceList[bestIndex]);
        }
        delete vcache;
        delete[] bVisitedList;
    }
}

//============================================================================================================================================================================================================================
// NextIsCW() : returns true if the next face should be ordered in CW fashion
//============================================================================================================================================================================================================================
bool NvStripifier::NextIsCW(const int numIndices)
    { return ((numIndices & 1) == 0); }


//============================================================================================================================================================================================================================
// IsCW() : returns true if the face is ordered in CW fashion
//============================================================================================================================================================================================================================
bool NvStripifier::IsCW(NvFaceInfo *faceInfo, int v0, int v1)
{
    if (faceInfo->m_v0 == v0) return (faceInfo->m_v1 == v1);
    if (faceInfo->m_v1 == v0) return (faceInfo->m_v2 == v1);
    return (faceInfo->m_v0 == v1);
}

bool NvStripifier::FaceContainsIndex(const NvFaceInfo& face, const unsigned int index)
    { return ((face.m_v0 == index) || (face.m_v1 == index) || (face.m_v2 == index)); }

//============================================================================================================================================================================================================================
// CreateStrips() : generates actual strips from the list-in-strip-order.
//============================================================================================================================================================================================================================
void NvStripifier::CreateStrips(const NvStripInfoVec& allStrips, IntVec& stripIndices, const bool bStitchStrips, unsigned int& numSeparateStrips, const bool bRestart, const unsigned int restartVal)
{
    assert(numSeparateStrips == 0);

    NvFaceInfo tLastFace(0, 0, 0);
    NvFaceInfo tPrevStripLastFace(0, 0, 0);
    int nStripCount = allStrips.size();
    assert(nStripCount > 0);
    
    int accountForNegatives = 0;                                                                                                    // we infer the cw/ccw ordering depending on the number of indices this is screwed up 
                                                                                                                                    // by the fact that we insert -1s to denote changing strips this is to account for that

    for (int i = 0; i < nStripCount; i++)
    {
        NvStripInfo *strip = allStrips[i];
        int nStripFaceCount = strip->m_faces.size();
        assert(nStripFaceCount > 0);

        {
            NvFaceInfo tFirstFace(strip->m_faces[0]->m_v0, strip->m_faces[0]->m_v1, strip->m_faces[0]->m_v2);                       // Handle the first face in the strip            
            if (nStripFaceCount > 1)                                                                                                // If there is a second face, reorder vertices such that the unique vertex is first
            {
                int nUnique = NvStripifier::GetUniqueVertexInB(strip->m_faces[1], &tFirstFace);
                if (nUnique == tFirstFace.m_v1)
                    std::swap(tFirstFace.m_v0, tFirstFace.m_v1);
                else if (nUnique == tFirstFace.m_v2)
                    std::swap(tFirstFace.m_v0, tFirstFace.m_v2);
               
                if (nStripFaceCount > 2)                                                                                            // If there is a third face, reorder vertices such that the shared vertex is last
                {
                    if(IsDegenerate(strip->m_faces[1]))
                    {
                        int pivot = strip->m_faces[1]->m_v1;
                        if (tFirstFace.m_v1 == pivot)
                            std::swap(tFirstFace.m_v1, tFirstFace.m_v2);
                    }
                    else
                    {
                        int nShared0, nShared1;
                        GetSharedVertices(strip->m_faces[2], &tFirstFace, &nShared0, &nShared1);
                        if ((nShared0 == tFirstFace.m_v1) && (nShared1 == -1))
                            std::swap(tFirstFace.m_v1, tFirstFace.m_v2);
                    }
                }
            }

            if ((i == 0) || !bStitchStrips || bRestart)
            {
                if (!IsCW(strip->m_faces[0], tFirstFace.m_v0, tFirstFace.m_v1))
                    stripIndices.push_back(tFirstFace.m_v0);
            }
            else
            {
                stripIndices.push_back(tFirstFace.m_v0);                                                                            // Double tap the first in the new strip
                                                                                                                                    // Check CW/CCW ordering
                if (NextIsCW(stripIndices.size() - accountForNegatives) != IsCW(strip->m_faces[0], tFirstFace.m_v0, tFirstFace.m_v1))
                    stripIndices.push_back(tFirstFace.m_v0);
            }

            stripIndices.push_back(tFirstFace.m_v0);
            stripIndices.push_back(tFirstFace.m_v1);
            stripIndices.push_back(tFirstFace.m_v2);
            
            tLastFace = tFirstFace;                                                                                                 // Update last face info
        }

        for (int j = 1; j < nStripFaceCount; j++)
        {
            int nUnique = GetUniqueVertexInB(&tLastFace, strip->m_faces[j]);
            if (nUnique != -1)
            {
                stripIndices.push_back(nUnique);
   
                tLastFace.m_v0 = tLastFace.m_v1;                                                                                    // Update last face info
                tLastFace.m_v1 = tLastFace.m_v2;
                tLastFace.m_v2 = nUnique;
            }
            else
            {
                stripIndices.push_back(strip->m_faces[j]->m_v2);                                                                    // we've hit a degenerate
                tLastFace.m_v0 = strip->m_faces[j]->m_v0;                                                                           // tLastFace.m_v1;
                tLastFace.m_v1 = strip->m_faces[j]->m_v1;                                                                           // tLastFace.m_v2;
                tLastFace.m_v2 = strip->m_faces[j]->m_v2;                                                                           // tLastFace.m_v1;
            }
        }

        
        if (bStitchStrips && !bRestart)                                                                                             // Double tap between strips.
        {
            if (i != nStripCount - 1)
                stripIndices.push_back(tLastFace.m_v2);
        } 
        else if (bRestart) 
            stripIndices.push_back(restartVal);
        else 
        {            
            stripIndices.push_back(-1);                                                                                             // -1 index indicates next strip
            accountForNegatives++;
            numSeparateStrips++;
        }
        
        tLastFace.m_v0 = tLastFace.m_v1;                                                                                            // Update last face info
        tLastFace.m_v1 = tLastFace.m_v2;
        tLastFace.m_v2 = tLastFace.m_v2;
    }
    
    if(bStitchStrips || bRestart)
        numSeparateStrips = 1;
}

//============================================================================================================================================================================================================================
// Stripify() : in_indices are the input indices of the mesh to stripify, in_cacheSize is the target cache size 
//============================================================================================================================================================================================================================
void NvStripifier::Stripify(const WordVec &in_indices, const int in_cacheSize, const int in_minStripLength, const unsigned short maxIndex, NvStripInfoVec &outStrips, NvFaceInfoVec& outFaceList)
{
    meshJump = 0.0f;
    bFirstTimeResetPoint = true;                                                                                                    // used in FindGoodResetPoint()
    int numSamples = 10;                                                                                                            // the number of times to run the experiments

    cacheSize = std::max(1, in_cacheSize - CACHE_INEFFICIENCY);                                                                     // the cache size, clamped to one

    minStripLength = in_minStripLength;                                                                                             // this is the strip size threshold below which we dump the strip into a list

    indices = in_indices;
    
    
    NvFaceInfoVec allFaceInfos;                                                                                                     // build the stripification info
    NvEdgeInfoVec allEdgeInfos;
    BuildStripifyInfo(allFaceInfos, allEdgeInfos, maxIndex);
    NvStripInfoVec allStrips;
    FindAllStrips(allStrips, allFaceInfos, allEdgeInfos, numSamples);                                                               // stripify
    SplitUpStripsAndOptimize(allStrips, outStrips, allEdgeInfos, outFaceList);                                                      // split up the strips into cache friendly pieces, optimize them, then dump these into outStrips
    
    for(int i = 0; i < allStrips.size(); i++)                                                                                       // clean up
        delete allStrips[i];
    
    for (int i = 0; i < allEdgeInfos.size(); i++)
    {
        NvEdgeInfo *info = allEdgeInfos[i];
        while (info != 0)
        {
            NvEdgeInfo *next = (info->m_v0 == i ? info->m_nextV0 : info->m_nextV1);
            info->Unref();
            info = next;
        }
    }   
}

bool NvStripifier::IsDegenerate(const NvFaceInfo* face)
{
    if(face->m_v0 == face->m_v1) return true;
    if(face->m_v0 == face->m_v2) return true;
    if(face->m_v1 == face->m_v2) return true;
    return false;
}

bool NvStripifier::IsDegenerate(const unsigned short v0, const unsigned short v1, const unsigned short v2)
{
    if (v0 == v1) return true;
    if (v0 == v2) return true;
    if (v1 == v2) return true;
    return false;
}

//============================================================================================================================================================================================================================
// SplitUpStripsAndOptimize()
// Splits the input vector of strips (allBigStrips) into smaller, cache friendly pieces, then reorders these pieces to maximize cache hits
// The final strips are output through outStrips
//============================================================================================================================================================================================================================
void NvStripifier::SplitUpStripsAndOptimize(NvStripInfoVec &allStrips, NvStripInfoVec &outStrips, NvEdgeInfoVec& edgeInfos, NvFaceInfoVec& outFaceList)
{
    int threshold = cacheSize;
    NvStripInfoVec tempStrips;
        
    for(int i = 0; i < allStrips.size(); i++)                                                                                       // split up strips into threshold-sized pieces
    {
        NvStripInfo* currentStrip;
        NvStripStartInfo startInfo(NULL, NULL, false);
    
        int actualStripSize = 0;
        for(int j = 0; j < allStrips[i]->m_faces.size(); ++j)
        {
            if (!IsDegenerate(allStrips[i]->m_faces[j]))
                actualStripSize++;
        }
        
        if(actualStripSize /* allStrips[i]->m_faces.size() */ > threshold)
        {
            
            int numTimes    = actualStripSize /* allStrips[i]->m_faces.size() */ / threshold;
            int numLeftover = actualStripSize /* allStrips[i]->m_faces.size() */ % threshold;

            int degenerateCount = 0;
            int j;
            for(j = 0; j < numTimes; j++)
            {
                currentStrip = new NvStripInfo(startInfo, 0, -1);
                int faceCtr = j*threshold + degenerateCount;
                bool bFirstTime = true;
                while(faceCtr < threshold+(j*threshold)+degenerateCount)
                {
                    if(IsDegenerate(allStrips[i]->m_faces[faceCtr]))
                    {
                        degenerateCount++;
                        if ((((faceCtr + 1) != threshold+(j*threshold)+degenerateCount) || ((j == numTimes - 1) && (numLeftover < 4) && (numLeftover > 0))) &&  !bFirstTime)
                            currentStrip->m_faces.push_back(allStrips[i]->m_faces[faceCtr++]);                                      // last time or first time through, no need for a degenerate
                        else
                        {
                            //but, we do need to delete the degenerate, if it's marked fake, to avoid leaking
                            if(allStrips[i]->m_faces[faceCtr]->m_bIsFake)
                            {
                                delete allStrips[i]->m_faces[faceCtr], allStrips[i]->m_faces[faceCtr] = NULL;
                            }
                            ++faceCtr;
                        }
                    }
                    else
                    {
                        currentStrip->m_faces.push_back(allStrips[i]->m_faces[faceCtr++]);
                        bFirstTime = false;
                    }
                }

                if (j == numTimes - 1)                                                                                              // last time through
                {
                    if ((numLeftover < 4) && (numLeftover > 0))                                                                     // way too small
                    {
                        
                        int ctr = 0;                                                                                                // just add to last strip
                        while(ctr < numLeftover)
                        {
                            IsDegenerate( allStrips[i]->m_faces[faceCtr] ) ? ++degenerateCount : ++ctr;
                            currentStrip->m_faces.push_back(allStrips[i]->m_faces[faceCtr++]);
                        }
                        numLeftover = 0;
                    }
                }
                tempStrips.push_back(currentStrip);
            }
            
            int leftOff = j * threshold + degenerateCount;
            
            if(numLeftover != 0)
            {
                currentStrip = new NvStripInfo(startInfo, 0, -1);   
                
                int ctr = 0;
                bool bFirstTime = true;
                while(ctr < numLeftover)
                {
                    if( !IsDegenerate(allStrips[i]->m_faces[leftOff]) )
                    {
                        ctr++;
                        bFirstTime = false;
                        currentStrip->m_faces.push_back(allStrips[i]->m_faces[leftOff++]);
                    }
                    else if(!bFirstTime)
                        currentStrip->m_faces.push_back(allStrips[i]->m_faces[leftOff++]);
                    else
                    {
                        //don't leak
                        if(allStrips[i]->m_faces[leftOff]->m_bIsFake)
                        {
                            delete allStrips[i]->m_faces[leftOff], allStrips[i]->m_faces[leftOff] = NULL;
                        }

                        leftOff++;
                    }
                }
                tempStrips.push_back(currentStrip);
            }
        }
        else
        {
            currentStrip = new NvStripInfo(startInfo, 0, -1);                                                                       // we're not just doing a tempStrips.push_back(allBigStrips[i]) because this way we can delete allBigStrips later to free the memory
            for(int j = 0; j < allStrips[i]->m_faces.size(); j++)
                currentStrip->m_faces.push_back(allStrips[i]->m_faces[j]);
            tempStrips.push_back(currentStrip);
        }
    }

    
    NvStripInfoVec tempStrips2;                                                                                                     // add small strips to face list
    RemoveSmallStrips(tempStrips, tempStrips2, outFaceList);    
    outStrips.clear();
    
    if(tempStrips2.size() != 0)
    {        
        VertexCache* vcache = new VertexCache(cacheSize);                                                                           // Optimize for the vertex cache
        
        float bestNumHits = -1.0f;
        float numHits;
        int bestIndex;
        bool done = false;
        int firstIndex = 0;
        float minCost = 10000.0f;
        
        for(int i = 0; i < tempStrips2.size(); i++)
        {
            int numNeighbors = 0;
            for(int j = 0; j < tempStrips2[i]->m_faces.size(); j++)                                                                 // find strip with least number of neighbors per face
                numNeighbors += NumNeighbors(tempStrips2[i]->m_faces[j], edgeInfos);
            
            float currCost = (float)numNeighbors / (float)tempStrips2[i]->m_faces.size();
            if(currCost < minCost)
            {
                minCost = currCost;
                firstIndex = i;
            }
        }
        
        UpdateCacheStrip(vcache, tempStrips2[firstIndex]);
        outStrips.push_back(tempStrips2[firstIndex]);
        tempStrips2[firstIndex]->visited = true;        
        bool bWantsCW = (tempStrips2[firstIndex]->m_faces.size() % 2) == 0;
        
        while(1)                                                                                                                    // this n^2 algo is what slows down stripification so much .. needs to be improved
        {
            bestNumHits = -1.0f;
            
            
            for(int i = 0; i < tempStrips2.size(); i++)                                                                             // find best strip to add next, given the current cache
            {
                if(tempStrips2[i]->visited) continue;

                numHits = CalcNumHitsStrip(vcache, tempStrips2[i]);
                if(numHits > bestNumHits)
                {
                    bestNumHits = numHits;
                    bestIndex = i;
                }
                else if(numHits >= bestNumHits)
                {                    
                    NvStripInfo *strip = tempStrips2[i];                                                                            // check previous strip to see if this one requires it to switch polarity
                    int nStripFaceCount = strip->m_faces.size();
                    NvFaceInfo tFirstFace(strip->m_faces[0]->m_v0, strip->m_faces[0]->m_v1, strip->m_faces[0]->m_v2);

                    if (nStripFaceCount > 1)                                                                                        // If there is a second face, reorder vertices such that the unique vertex is first
                    {
                        int nUnique = NvStripifier::GetUniqueVertexInB(strip->m_faces[1], &tFirstFace);
                        if (nUnique == tFirstFace.m_v1)
                        {
                            std::swap(tFirstFace.m_v0, tFirstFace.m_v1);
                        }
                        else if (nUnique == tFirstFace.m_v2)
                        {
                            std::swap(tFirstFace.m_v0, tFirstFace.m_v2);
                        }
                        
                        if (nStripFaceCount > 2)                                                                                    // If there is a third face, reorder vertices such that the shared vertex is last
                        {
                            int nShared0, nShared1;
                            GetSharedVertices(strip->m_faces[2], &tFirstFace, &nShared0, &nShared1);
                            if ( (nShared0 == tFirstFace.m_v1) && (nShared1 == -1) )
                            {
                                std::swap(tFirstFace.m_v1, tFirstFace.m_v2);
                            }
                        }
                    }
                        
                    
                    if (bWantsCW == IsCW(strip->m_faces[0], tFirstFace.m_v0, tFirstFace.m_v1))                                      // Check CW/CCW ordering
                        bestIndex = i;                                                                                              // I like this one!
                }
            }
            
            if (bestNumHits == -1.0f) break;
            tempStrips2[bestIndex]->visited = true;
            UpdateCacheStrip(vcache, tempStrips2[bestIndex]);
            outStrips.push_back(tempStrips2[bestIndex]);
            bWantsCW = (tempStrips2[bestIndex]->m_faces.size() % 2 == 0) ? bWantsCW : !bWantsCW;
        }
        delete vcache;  
    }
}

//============================================================================================================================================================================================================================
// UpdateCacheStrip() : updates the input vertex cache with this strip's vertices
//============================================================================================================================================================================================================================
void NvStripifier::UpdateCacheStrip(VertexCache* vcache, NvStripInfo* strip)
{
    for(int i = 0; i < strip->m_faces.size(); ++i)
    {
        if(!vcache->InCache(strip->m_faces[i]->m_v0)) vcache->AddEntry(strip->m_faces[i]->m_v0);
        if(!vcache->InCache(strip->m_faces[i]->m_v1)) vcache->AddEntry(strip->m_faces[i]->m_v1);        
        if(!vcache->InCache(strip->m_faces[i]->m_v2)) vcache->AddEntry(strip->m_faces[i]->m_v2);
    }
}

//============================================================================================================================================================================================================================
// UpdateCacheFace() : updates the input vertex cache with this face's vertices
//============================================================================================================================================================================================================================
void NvStripifier::UpdateCacheFace(VertexCache* vcache, NvFaceInfo* face)
{
    if(!vcache->InCache(face->m_v0)) vcache->AddEntry(face->m_v0);
    if(!vcache->InCache(face->m_v1)) vcache->AddEntry(face->m_v1);
    if(!vcache->InCache(face->m_v2)) vcache->AddEntry(face->m_v2);
}

//============================================================================================================================================================================================================================
// CalcNumHitsStrip() : returns the number of cache hits per face in the strip
//============================================================================================================================================================================================================================
float NvStripifier::CalcNumHitsStrip(VertexCache* vcache, NvStripInfo* strip)
{
    int numHits = 0;
    int numFaces = 0;
    for(int i = 0; i < strip->m_faces.size(); i++)
    {
        if (vcache->InCache(strip->m_faces[i]->m_v0)) ++numHits;
        if (vcache->InCache(strip->m_faces[i]->m_v1)) ++numHits;
        if (vcache->InCache(strip->m_faces[i]->m_v2)) ++numHits;
        numFaces++;
    }
    return ((float)numHits / (float)numFaces);
}

//============================================================================================================================================================================================================================
// CalcNumHitsFace() : returns the number of cache hits in the face
//============================================================================================================================================================================================================================
int NvStripifier::CalcNumHitsFace(VertexCache* vcache, NvFaceInfo* face)
{
    int numHits = 0;
    if (vcache->InCache(face->m_v0)) numHits++;
    if (vcache->InCache(face->m_v1)) numHits++;
    if (vcache->InCache(face->m_v2)) numHits++;
    return numHits;
}

//============================================================================================================================================================================================================================
// NumNeighbors() : returns the number of neighbors that this face has
//============================================================================================================================================================================================================================
int NvStripifier::NumNeighbors(NvFaceInfo* face, NvEdgeInfoVec& edgeInfoVec)
{
    int numNeighbors = 0;
    if (FindOtherFace(edgeInfoVec, face->m_v0, face->m_v1, face) != 0) numNeighbors++;
    if (FindOtherFace(edgeInfoVec, face->m_v1, face->m_v2, face) != 0) numNeighbors++;
    if (FindOtherFace(edgeInfoVec, face->m_v2, face->m_v0, face) != 0) numNeighbors++;
    return numNeighbors;
}

//============================================================================================================================================================================================================================
// AvgStripSize() : finds the average strip size of the input vector of strips
//============================================================================================================================================================================================================================
float NvStripifier::AvgStripSize(const NvStripInfoVec &strips)
{
    int sizeAccum = 0;
    int numStrips = strips.size();
    for (int i = 0; i < numStrips; i++)
    {
        NvStripInfo *strip = strips[i];
        sizeAccum += strip->m_faces.size();
        sizeAccum -= strip->m_numDegenerates;
    }
    return ((float)sizeAccum) / ((float)numStrips);
}

//============================================================================================================================================================================================================================
// FindAllStrips()
// Does the stripification, puts output strips into vector allStrips
// Works by setting runnning a number of experiments in different areas of the mesh, and
//  accepting the one which results in the longest strips.  It then accepts this, and moves
//  on to a different area of the mesh.  We try to jump around the mesh some, to ensure that
//  large open spans of strips get generated.
//============================================================================================================================================================================================================================
void NvStripifier::FindAllStrips(NvStripInfoVec &allStrips, NvFaceInfoVec &allFaceInfos, NvEdgeInfoVec &allEdgeInfos, int numSamples)
{    
    int experimentId = 0;                                                                                                           // the experiments
    int stripId = 0;
    bool done = false;
    int loopCtr = 0;
    
    while (!done)
    {
        loopCtr++;
        
        NvStripInfoVec *experiments = new NvStripInfoVec [numSamples * 6];                                                          // PHASE 1: Set up numSamples * numEdges experiments
        int experimentIndex = 0;
        std::set   <NvFaceInfo*>  resetPoints;
        for (int i = 0; i < numSamples; i++)
        {
            NvFaceInfo *nextFace = FindGoodResetPoint(allFaceInfos, allEdgeInfos);                                                  // Try to find another good reset point. If there are none to be found, we are done
            if (nextFace == 0)
            {
                done = true;
                break;
            }
            else if (resetPoints.find(nextFace) != resetPoints.end())                                                               // If we have already evaluated starting at this face in this slew of experiments, then skip going any further
                continue;
            
            resetPoints.insert(nextFace);                                                                                           // trying it now...
            assert(nextFace->m_stripId < 0);                                                                                        // otherwise, we shall now try experiments for starting on the 01,12, and 20 edges
            
            NvEdgeInfo *edge01 = FindEdgeInfo(allEdgeInfos, nextFace->m_v0, nextFace->m_v1);                                        // build the strip off of this face's 0-1 edge
            NvStripInfo *strip01 = new NvStripInfo(NvStripStartInfo(nextFace, edge01, true), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip01);
            NvEdgeInfo *edge10 = FindEdgeInfo(allEdgeInfos, nextFace->m_v0, nextFace->m_v1);                                        // build the strip off of this face's 1-0 edge
            NvStripInfo *strip10 = new NvStripInfo(NvStripStartInfo(nextFace, edge10, false), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip10);
            NvEdgeInfo *edge12 = FindEdgeInfo(allEdgeInfos, nextFace->m_v1, nextFace->m_v2);                                        // build the strip off of this face's 1-2 edge
            NvStripInfo *strip12 = new NvStripInfo(NvStripStartInfo(nextFace, edge12, true), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip12);
            NvEdgeInfo *edge21 = FindEdgeInfo(allEdgeInfos, nextFace->m_v1, nextFace->m_v2);                                        // build the strip off of this face's 2-1 edge
            NvStripInfo *strip21 = new NvStripInfo(NvStripStartInfo(nextFace, edge21, false), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip21);
            NvEdgeInfo *edge20 = FindEdgeInfo(allEdgeInfos, nextFace->m_v2, nextFace->m_v0);                                        // build the strip off of this face's 2-0 edge
            NvStripInfo *strip20 = new NvStripInfo(NvStripStartInfo(nextFace, edge20, true), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip20);
            NvEdgeInfo *edge02 = FindEdgeInfo(allEdgeInfos, nextFace->m_v2, nextFace->m_v0);                                        // build the strip off of this face's 0-2 edge
            NvStripInfo *strip02 = new NvStripInfo(NvStripStartInfo(nextFace, edge02, false), stripId++, experimentId++);
            experiments[experimentIndex++].push_back(strip02);
        }
                                                                                                                                    // PHASE 2: Iterate through that we setup in the last phase
                                                                                                                                    // and really build each of the strips and strips that follow to see how far we get
        int numExperiments = experimentIndex;
        for (int i = 0; i < numExperiments; i++)
        {
            experiments[i][0]->Build(allEdgeInfos, allFaceInfos);                                                                   // get the strip set, build the first strip of the list
            int experimentId = experiments[i][0]->m_experimentId;
            
            NvStripInfo *stripIter = experiments[i][0];
            NvStripStartInfo startInfo(NULL, NULL, false);
            while (FindTraversal(allFaceInfos, allEdgeInfos, stripIter, startInfo))
            {
                stripIter = new NvStripInfo(startInfo, stripId++, experimentId);                                                    // create the new strip info
                stripIter->Build(allEdgeInfos, allFaceInfos);                                                                       // build the next strip
                experiments[i].push_back(stripIter);                                                                                // add it to the list
            }
        }
                                                                                                                                    // Phase 3: Find the experiment that has the most promise
        int bestIndex = 0;
        double bestValue = 0;
        for (int i = 0; i < numExperiments; i++)
        {
            const float avgStripSizeWeight = 1.0f;
            const float numTrisWeight      = 0.0f;
            const float numStripsWeight    = 0.0f;
            float avgStripSize = AvgStripSize(experiments[i]);
            float numStrips = (float) experiments[i].size();
            float value = avgStripSize * avgStripSizeWeight + (numStrips * numStripsWeight);
                
            if (value > bestValue)
            {
                bestValue = value;
                bestIndex = i;
            }
        }
                                                                                                                                    // Phase 4: commit the best experiment of the bunch
        CommitStrips(allStrips, experiments[bestIndex]);
        
        for (int i = 0; i < numExperiments; i++)                                                                                    // and destroy all of the others
        {
            if (i != bestIndex)
            {
                int numStrips = experiments[i].size();
                for (int j = 0; j < numStrips; j++)
                {
                    NvStripInfo* currStrip = experiments[i][j];
                    for (int k = 0; k < currStrip->m_faces.size(); ++k)                                                             // delete all bogus faces in the experiments
                    {
                        if(currStrip->m_faces[k]->m_bIsFake)
                            delete currStrip->m_faces[k], currStrip->m_faces[k] = 0;
                    }
                    delete currStrip, currStrip = 0, experiments[i][j] = 0;
                }
            }
        }
        
        // delete the array that we used for all experiments
        delete [] experiments;
    }
}

//============================================================================================================================================================================================================================
// CountRemainingTris() : will count the number of triangles left in the strip list starting at iter and finishing up at end
//============================================================================================================================================================================================================================
int NvStripifier::CountRemainingTris(std::list<NvStripInfo*>::iterator iter, std::list<NvStripInfo*>::iterator end)
{
    int count = 0;
    while (iter != end)
    {
        count += (*iter)->m_faces.size();
        iter++;
    }
    return count;
}