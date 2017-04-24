#ifndef NV_TRISTRIP_OBJECTS_H
#define NV_TRISTRIP_OBJECTS_H

#include <cassert>
#include <vector>
#include <list>
#include "vertexcache.hpp"

//============================================================================================================================================================================================================================
// Types defined for stripification
//============================================================================================================================================================================================================================

struct NvFaceInfo 
{    
    NvFaceInfo(int v0, int v1, int v2, bool bIsFake = false)
        : m_v0(v0), m_v1(v1), m_v2(v2), m_stripId(-1), m_testStripId(-1), m_experimentId(-1), m_bIsFake(bIsFake) { }
    
    int m_v0, m_v1, m_v2;                                                                                                         // vertex indices
    int m_stripId;                                                                                                                // real strip Id
    int m_testStripId;                                                                                                            // strip Id in an experiment
    int m_experimentId;                                                                                                           // in what experiment was it given an experiment Id?
    bool m_bIsFake;                                                                                                                // if true, will be deleted when the strip it's in is deleted
};

//============================================================================================================================================================================================================================
// nice and dumb edge class that points knows its indices, the two faces, and the next edge using the lesser of the indices
//============================================================================================================================================================================================================================
struct NvEdgeInfo
{
    NvEdgeInfo (int v0, int v1) 
        : m_v0(v0), m_v1(v1), m_face0(0), m_face1(0), m_nextV0(0), m_nextV1(0), m_refCount(2) {}
    
    // we will appear in 2 lists. this is a good way to make sure we delete it the second time we hit it in the edge infos
    void Unref () 
        { if (--m_refCount == 0) delete this; }
        
    int m_v0, m_v1;
    NvFaceInfo *m_face0, *m_face1;
    NvEdgeInfo *m_nextV0, *m_nextV1;
    unsigned int m_refCount;
};


//============================================================================================================================================================================================================================
// This class is a quick summary of parameters used to begin a triangle strip. 
// Some operations may want to create lists of such items, so they were pulled out into a class
//============================================================================================================================================================================================================================

struct NvStripStartInfo
{
    NvFaceInfo    *m_startFace;
    NvEdgeInfo    *m_startEdge;
    bool           m_toV1;      

    NvStripStartInfo(NvFaceInfo *startFace, NvEdgeInfo *startEdge, bool toV1) : m_startFace(startFace), m_startEdge(startEdge), m_toV1(toV1) {}
};

typedef std::vector<NvFaceInfo*> NvFaceInfoVec;
typedef std::list<NvFaceInfo*> NvFaceInfoList;
typedef std::list<NvFaceInfoVec*> NvStripList;
typedef std::vector<NvEdgeInfo*> NvEdgeInfoVec;

typedef std::vector<short int> WordVec;
typedef std::vector<int> IntVec;

//============================================================================================================================================================================================================================
// This is a summary of a strip that has been built
//============================================================================================================================================================================================================================
struct NvStripInfo 
{
    NvStripStartInfo m_startInfo;
    NvFaceInfoVec m_faces;
    int m_stripId;
    int m_experimentId;      
    bool visited;
    int m_numDegenerates;
    
    NvStripInfo(const NvStripStartInfo &startInfo, int stripId, int experimentId = -1) : m_startInfo(startInfo)                     // A little information about the creation of the triangle strips
    {
        m_stripId      = stripId;
        m_experimentId = experimentId;
        visited = false;
        m_numDegenerates = 0;
    }

    inline bool IsExperiment () const                                                                                               // This is an experiment if the experiment id is >= 0
        { return m_experimentId >= 0; }
      
    inline bool IsInStrip (const NvFaceInfo *faceInfo) const 
    {
        if(!faceInfo) return false;          
        return (m_experimentId >= 0 ? faceInfo->m_testStripId == m_stripId : faceInfo->m_stripId == m_stripId);
    }
      
    bool SharesEdge(const NvFaceInfo* faceInfo, NvEdgeInfoVec &edgeInfos);
    void Combine(const NvFaceInfoVec &forward, const NvFaceInfoVec &backward);                                                      // take the given forward and backward strips and combine them together
    bool Unique(NvFaceInfoVec& faceVec, NvFaceInfo* face);                                                                          // returns true if the face is "unique", i.e. has a vertex which doesn't exist in the faceVec
    bool IsMarked    (NvFaceInfo *faceInfo);                                                                                        // mark the triangle as taken by this strip
    void MarkTriangle(NvFaceInfo *faceInfo);
          
    void Build(NvEdgeInfoVec &edgeInfos, NvFaceInfoVec &faceInfos);                                                                 // build the strip      
};

typedef std::vector<NvStripInfo*> NvStripInfoVec;


//============================================================================================================================================================================================================================
// The actual stripifier
//============================================================================================================================================================================================================================
struct NvStripifier 
{   
    WordVec indices;
    int cacheSize;
    int minStripLength;
    float meshJump;
    bool bFirstTimeResetPoint;

    NvStripifier() {};
    ~NvStripifier() {};
    
    //========================================================================================================================================================================================================================
    //the target vertex cache size, the structure to place the strips in, and the input indices
    //========================================================================================================================================================================================================================
    void Stripify(const WordVec& in_indices, const int in_cacheSize, const int in_minStripLength, const unsigned short maxIndex, NvStripInfoVec& allStrips, NvFaceInfoVec& allFaces);
    void CreateStrips(const NvStripInfoVec& allStrips, IntVec& stripIndices, const bool bStitchStrips, unsigned int& numSeparateStrips, const bool bRestart, const unsigned int restartVal);
    
    static int GetUniqueVertexInB(NvFaceInfo *faceA, NvFaceInfo *faceB);
    static void GetSharedVertices(NvFaceInfo *faceA, NvFaceInfo *faceB, int* vertex0, int* vertex1);

    static bool IsDegenerate(const NvFaceInfo* face);
    static bool IsDegenerate(const unsigned short v0, const unsigned short v1, const unsigned short v2);
        
    //========================================================================================================================================================================================================================
    // Big mess of functions called during stripification
    //========================================================================================================================================================================================================================

    bool FaceContainsIndex(const NvFaceInfo& face, const unsigned int index);

    bool IsCW(NvFaceInfo *faceInfo, int v0, int v1);
    bool NextIsCW(const int numIndices);
    
    static int GetNextIndex(const WordVec &indices, NvFaceInfo *face);
    static NvEdgeInfo *FindEdgeInfo(NvEdgeInfoVec &edgeInfos, int v0, int v1);
    static NvFaceInfo *FindOtherFace(NvEdgeInfoVec &edgeInfos, int v0, int v1, NvFaceInfo *faceInfo);
    NvFaceInfo *FindGoodResetPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);
    
    void FindAllStrips(NvStripInfoVec &allStrips, NvFaceInfoVec &allFaceInfos, NvEdgeInfoVec &allEdgeInfos, int numSamples);
    void SplitUpStripsAndOptimize(NvStripInfoVec &allStrips, NvStripInfoVec &outStrips, NvEdgeInfoVec& edgeInfos, NvFaceInfoVec& outFaceList);
    void RemoveSmallStrips(NvStripInfoVec& allStrips, NvStripInfoVec& allBigStrips, NvFaceInfoVec& faceList);
    
    bool FindTraversal(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, NvStripInfo *strip, NvStripStartInfo &startInfo);
    int  CountRemainingTris(std::list<NvStripInfo*>::iterator iter, std::list<NvStripInfo*>::iterator  end);
    
    void CommitStrips(NvStripInfoVec &allStrips, const NvStripInfoVec &strips);
    
    float AvgStripSize(const NvStripInfoVec &strips);
    int FindStartPoint(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos);
    
    void UpdateCacheStrip(VertexCache* vcache, NvStripInfo* strip);
    void UpdateCacheFace(VertexCache* vcache, NvFaceInfo* face);
    float CalcNumHitsStrip(VertexCache* vcache, NvStripInfo* strip);
    int CalcNumHitsFace(VertexCache* vcache, NvFaceInfo* face);
    int NumNeighbors(NvFaceInfo* face, NvEdgeInfoVec& edgeInfoVec);
    
    void BuildStripifyInfo(NvFaceInfoVec &faceInfos, NvEdgeInfoVec &edgeInfos, const unsigned short maxIndex);
    bool AlreadyExists(NvFaceInfo* faceInfo, NvFaceInfoVec& faceInfos);
};

#endif