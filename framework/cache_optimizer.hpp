#include <cassert>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>

//-----------------------------------------------------------------------------
// This is an implementation of Tom Forsyth's "Linear-Speed Vertex Cache Optimization".
//
// The optimizer reorders triangles (and optionally vertices) to make better
// use of the GPU vertex cache and memory bandwidth, preserving the shape.
// Visual changes to the mesh are considered an error.
//
// The algorithm is described here:
//  http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
//
// Other implementations can be found at
// https://github.com/vivkin/forsyth/blob/master/forsyth.h,
// https://github.com/dv1/vcache_optimizer
// https://github.com/bkaradzic/bgfx/blob/master/3rdparty/forsyth-too/forsythtriangleorderoptimizer.cpp
//-----------------------------------------------------------------------------
template<class T> class FaceOptimizer
{
  
    static const unsigned int kMaxVertexCacheSize = 64;                                                 // The maximum allowed cache size.
    static const unsigned int kMaxPrecomputedVertexValenceScores = 64;                                  // The maximum amount of vertex valence scores.
    float s_vertexCacheScores[kMaxVertexCacheSize+1][kMaxVertexCacheSize];                              // A precalculated 2D array used to store vertex cache scores.
    float s_vertexValenceScores[kMaxPrecomputedVertexValenceScores];                                    // A precalculated array storing vertex valency scores.
    bool s_vertexScoresComputed;                                                                        // Flag indicating whether the vertex score values have been calculated.

    // Precalculate the vertex cache score, based on the given position and cach size.
    // cachePosition   : the position in the cache.
    // vertexCacheSize : the cache size.
    float ComputeVertexCacheScore(int cachePosition, unsigned int vertexCacheSize)
    {
        const float FindVertexScore_CacheDecayPower = 1.5f;
        const float FindVertexScore_LastTriScore = 0.75f;

        float score = 0.0f;
        if (cachePosition < 0)
        {
            // Vertex is not in FIFO cache - no score.
        }
        else
        {
            if (cachePosition < 3)
            {
                // This vertex was used in the last triangle, so it has a fixed score, whichever of the three
                // it's in. Otherwise, you can get very different answers depending on whether you add the triangle 1, 2, 3 or 3, 1, 2 - which is silly.
                score = FindVertexScore_LastTriScore;
            }
            else
            {
                assert (cachePosition < (int)vertexCacheSize);
                // Points for being high in the cache.
                const float scaler = 1.0f / ( vertexCacheSize - 3 );
                score = 1.0f - (cachePosition - 3) * scaler;
                score = powf (score, FindVertexScore_CacheDecayPower);
            }
        }

        return score;
    }

    // Calculate the vertex valence score for the given valency.
    // numActiveFaces : the number of active faces, or valency.
    float ComputeVertexValenceScore(unsigned int numActiveFaces)
    {
        const float FindVertexScore_ValenceBoostScale = 2.0f;
        const float FindVertexScore_ValenceBoostPower = 0.5f;
        float score = 0.f;

        // Bonus points for having a low number of tris still to use the vert, so we get rid of lone verts quickly.
        float valenceBoost = powf (static_cast<float>(numActiveFaces), -FindVertexScore_ValenceBoostPower);
        score += FindVertexScore_ValenceBoostScale * valenceBoost;

        return score;
    }

    // Calculate the vertex cache and vertex valency scores.
    // These score values are stored to the `s_vertexCacheScores` and `s_vertexValenceScores` arrays.
    bool ComputeVertexScores()
    {
        for (unsigned int cacheSize = 0; cacheSize <= kMaxVertexCacheSize; ++cacheSize)
        {
            for (unsigned int cachePos = 0; cachePos < cacheSize; ++cachePos)
            {
                s_vertexCacheScores[cacheSize][cachePos] = ComputeVertexCacheScore(cachePos, cacheSize);
            }
        }

        for (unsigned int valence = 0; valence < kMaxPrecomputedVertexValenceScores; ++valence)
        {
            s_vertexValenceScores[valence] = ComputeVertexValenceScore(valence);
        }
        return true;
    }

    // Get the vertex cache score value for the given cache position and maximum cache size.
    // cachePosition : the cache position for which the score should be retrieved.
    // maxSizeVertexCache : the cache size which should be used to retrieve the score.
    // return : The stored vertex cache score for the given position.
    inline float FindVertexCacheScore(unsigned int cachePosition, unsigned int maxSizeVertexCache)
    {
        return s_vertexCacheScores[maxSizeVertexCache][cachePosition];
    }

    // Get the vertex valence score for the given number of active triangles.
    // numActiveTris : the number of active triangles for which the score should be retrieved.
    // return : The vertex valence score for the number of active triangles.
    inline float FindVertexValenceScore(unsigned int numActiveTris)
    {
        return s_vertexValenceScores[numActiveTris];
    }

    // Get the total score for the given vertex valency, vertex cache position and size.
    // numActiveFaces : the number of active faces for which the score should be retrieved.
    // cachePosition : the cache position for which the score should be retrieved.
    // vertexCacheSize : the cache size which should be used to retrieve the score.
    float FindVertexScore(unsigned int numActiveFaces, unsigned int cachePosition, unsigned int vertexCacheSize)
    {
        assert(s_vertexScoresComputed);

        if (numActiveFaces == 0)                                              // No tri needs this vertex!
            return -1.0f;

        float score = 0.f;
        if (cachePosition < vertexCacheSize)
        {
            score += s_vertexCacheScores[vertexCacheSize][cachePosition];
        }

        if (numActiveFaces < kMaxPrecomputedVertexValenceScores)
        {
            score += s_vertexValenceScores[numActiveFaces];
        }
        else
        {
            score += ComputeVertexValenceScore(numActiveFaces);
        }
        return score;
    }

    // A object that stores all the relevant data for optimising the vertex data.
    struct OptimizeVertexData 
    {
        float score;                                        // The total score (including both valency and cache position and cache size scores).
        unsigned int activeFaceListStart;                   // The starting index for the face list.
        unsigned int activeFaceListSize;                    // The face list size.
        T cachePos0;                                        // The vertex cache position.
        T cachePos1;                                        // The vertex cache position.

        OptimizeVertexData() : score(0.f), activeFaceListStart(0), activeFaceListSize(0), cachePos0(0), cachePos1(0) { }
    };

public:

    FaceOptimizer() 
    {
        s_vertexScoresComputed = ComputeVertexScores();
    }

    // Reorder the faces to make better use of the GPU vertex cache and memory bandwidth.
    //
    // indexList : input index list.
    // indexCount : the number of indices in the list.
    // vertexCount : the largest index value in indexList.
    // newIndexList : a pointer to a preallocated buffer the same size as
    // indexList : to hold the optimized index list.
    // lruCacheSize : the size of the simulated post-transform cache (max:64).

    void OptimizeFaces(const T* indexList, unsigned int indexCount, unsigned int vertexCount, T* newIndexList, T lruCacheSize)
    {
        std::vector<OptimizeVertexData> vertexDataList;
        vertexDataList.resize(vertexCount);

        // compute face count per vertex
        for (unsigned int i = 0; i < indexCount; ++i)
        {
            T index = indexList[i];
            assert(index < vertexCount);
            OptimizeVertexData& vertexData = vertexDataList[index];
            vertexData.activeFaceListSize++;
        }

        std::vector<unsigned int> activeFaceList;

        const T kEvictedCacheIndex = std::numeric_limits<T>::max();

        {
            // allocate face list per vertex
            unsigned int curActiveFaceListPos = 0;
            for (unsigned int i = 0; i < vertexCount; ++i) 
            {
                OptimizeVertexData& vertexData = vertexDataList[i];
                vertexData.cachePos0 = kEvictedCacheIndex;
                vertexData.cachePos1 = kEvictedCacheIndex;
                vertexData.activeFaceListStart = curActiveFaceListPos;
                curActiveFaceListPos += vertexData.activeFaceListSize;
                vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos0, lruCacheSize);
                vertexData.activeFaceListSize = 0;
            }
            activeFaceList.resize(curActiveFaceListPos);
        }

        // fill out face list per vertex
        for (unsigned int i = 0; i < indexCount; i += 3)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                T index = indexList[i + j];
                OptimizeVertexData& vertexData = vertexDataList[index];
                activeFaceList[vertexData.activeFaceListStart + vertexData.activeFaceListSize] = i;
                vertexData.activeFaceListSize++;
            }
        }
        std::vector<unsigned char> processedFaceList;
        processedFaceList.resize(indexCount);

        T vertexCacheBuffer[(kMaxVertexCacheSize + 3) * 2];
        T* cache0 = vertexCacheBuffer;
        T* cache1 = vertexCacheBuffer + (kMaxVertexCacheSize + 3);
        T entriesInCache0 = 0;

        unsigned int bestFace = 0;
        float bestScore = -1.f;

        const float maxValenceScore = FindVertexScore(1, kEvictedCacheIndex, lruCacheSize) * 3.f;

        for (unsigned int i = 0; i < indexCount; i += 3)
        {
            if (bestScore < 0.f) 
            {
                // no verts in the cache are used by any unprocessed faces so
                // search all unprocessed faces for a new starting point
                for (unsigned int j = 0; j < indexCount; j += 3) 
                {
                    if (processedFaceList[j] == 0)
                    {
                        unsigned int face = j;
                        float faceScore = 0.f;
                        for (unsigned int k = 0; k < 3; ++k)
                        {
                            T index = indexList[face + k];
                            OptimizeVertexData& vertexData = vertexDataList[index];
                            assert(vertexData.activeFaceListSize > 0);
                            assert(vertexData.cachePos0 >= lruCacheSize);
                            faceScore += vertexData.score;
                        }

                        if (faceScore > bestScore)
                        {
                            bestScore = faceScore;
                            bestFace = face;

                            assert(bestScore <= maxValenceScore);

                            if (bestScore >= maxValenceScore)
                            {
                                break;
                            }
                        }
                    }
                }

                assert(bestScore >= 0.f);
            }

            processedFaceList[bestFace] = 1;
            T entriesInCache1 = 0;

            // add bestFace to LRU cache and to newIndexList
            for (unsigned int v = 0; v < 3; ++v)
            {
                T index = indexList[bestFace+v];
                newIndexList[i+v] = index;

                OptimizeVertexData& vertexData = vertexDataList[index];

                if (vertexData.cachePos1 >= entriesInCache1)
                {
                    vertexData.cachePos1 = entriesInCache1;
                    cache1[entriesInCache1++] = index;

                    if (vertexData.activeFaceListSize == 1)
                    {
                        --vertexData.activeFaceListSize;
                        continue;
                    }
                }

                assert(vertexData.activeFaceListSize > 0);
                unsigned int* begin = &activeFaceList[vertexData.activeFaceListStart];
                unsigned int* end = &activeFaceList[vertexData.activeFaceListStart + vertexData.activeFaceListSize];
                unsigned int* it = std::find(begin, end, bestFace);
                assert(it != end);
                std::swap(*it, *(end-1));
                --vertexData.activeFaceListSize;
                vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);

            }

            // move the rest of the old verts in the cache down and compute their new scores
            for (unsigned int c0 = 0; c0 < entriesInCache0; ++c0)
            {
                T index = cache0[c0];
                OptimizeVertexData& vertexData = vertexDataList[index];

                if (vertexData.cachePos1 >= entriesInCache1)
                {
                    vertexData.cachePos1 = entriesInCache1;
                    cache1[entriesInCache1++] = index;
                    vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);
                }
            }

            // find the best scoring triangle in the current cache (including up to 3 that were just evicted)
            bestScore = -1.f;
            for (unsigned int c1 = 0; c1 < entriesInCache1; ++c1)
            {
                T index = cache1[c1];
                OptimizeVertexData& vertexData = vertexDataList[index];
                vertexData.cachePos0 = vertexData.cachePos1;
                vertexData.cachePos1 = kEvictedCacheIndex;
                for (unsigned int j=0; j<vertexData.activeFaceListSize; ++j)
                {
                    unsigned int face = activeFaceList[vertexData.activeFaceListStart+j];
                    float faceScore = 0.f;
                    for (unsigned int v = 0; v < 3; v++)
                    {
                        T faceIndex = indexList[face + v];
                        OptimizeVertexData& faceVertexData = vertexDataList[faceIndex];
                        faceScore += faceVertexData.score;
                    }

                    if (faceScore > bestScore)
                    {
                        bestScore = faceScore;
                        bestFace = face;
                    }
                }
            }

            std::swap(cache0, cache1);
            entriesInCache0 = std::min(entriesInCache1, lruCacheSize);
        }
    }
};