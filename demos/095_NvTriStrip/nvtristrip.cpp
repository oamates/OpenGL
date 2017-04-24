#include <cstring>

#include "nvtristripobjects.hpp"
#include "nvtristrip.hpp"

static unsigned int cacheSize    = CACHESIZE_GEFORCE1_2;
static bool bStitchStrips         = true;
static unsigned int minStripSize = 0;
static bool bListsOnly            = false;
static unsigned int restartVal   = 0;
static bool bRestart              = false;

void EnableRestart(const unsigned int _restartVal)
{
    bRestart = true;
    restartVal = _restartVal;
}

void DisableRestart()
    { bRestart = false; }

void SetListsOnly(const bool _bListsOnly)
    { bListsOnly = _bListsOnly; }

void SetCacheSize(const unsigned int _cacheSize)
    { cacheSize = _cacheSize; }

void SetStitchStrips(const bool _bStitchStrips)
    { bStitchStrips = _bStitchStrips; }

void SetMinStripSize(const unsigned int _minStripSize)
    { minStripSize = _minStripSize; }



void Cleanup(NvStripInfoVec& tempStrips, NvFaceInfoVec& tempFaces)
{
    for(int i = 0; i < tempStrips.size(); i++)                                                                                      // delete strips
    {
        for(int j = 0; j < tempStrips[i]->m_faces.size(); j++)
        {
            delete tempStrips[i]->m_faces[j];
            tempStrips[i]->m_faces[j] = 0;
        }
        tempStrips[i]->m_faces.resize(0);
        delete tempStrips[i];
        tempStrips[i] = 0;
    }
    
    for(int i = 0; i < tempFaces.size(); i++)                                                                                       // delete faces
    {
        delete tempFaces[i];
        tempFaces[i] = 0;
    }
}

bool SameTriangle(unsigned short firstTri0, unsigned short firstTri1, unsigned short firstTri2, unsigned short secondTri0, unsigned short secondTri1, unsigned short secondTri2)
{
    if (firstTri0 == secondTri0)
        { if ((firstTri1 == secondTri1) && (firstTri2 == secondTri2)) return true; }
    else if (firstTri0 == secondTri1)
        { if ((firstTri1 == secondTri2) && (firstTri2 == secondTri0)) return true; }
    else if (firstTri0 == secondTri2)
        { if ((firstTri1 == secondTri0) && (firstTri2 == secondTri1)) return true; }
    
    return false;
}

bool TestTriangle(const unsigned short v0, const unsigned short v1, const unsigned short v2, const std::vector<NvFaceInfo>* in_bins, const int NUMBINS)
{    
    bool isLegit = false;                                                                                                           // hash this triangle
    int ctr = v0 % NUMBINS;
    for (int k = 0; k < in_bins[ctr].size(); ++k)
    {        
        if (SameTriangle(in_bins[ctr][k].m_v0, in_bins[ctr][k].m_v1, in_bins[ctr][k].m_v2, v0, v1, v2))                             // check triangles in this bin
        {
            isLegit = true;
            break;
        }
    }
    if (!isLegit)
    {
        ctr = v1 % NUMBINS;
        for (int k = 0; k < in_bins[ctr].size(); ++k)
        {            
            if (SameTriangle(in_bins[ctr][k].m_v0, in_bins[ctr][k].m_v1, in_bins[ctr][k].m_v2, v0, v1, v2))                         // check triangles in this bin
            {
                isLegit = true;
                break;
            }
        }
        
        if (!isLegit)
        {
            ctr = v2 % NUMBINS;
            for (int k = 0; k < in_bins[ctr].size(); ++k)
            {
                if (SameTriangle(in_bins[ctr][k].m_v0, in_bins[ctr][k].m_v1, in_bins[ctr][k].m_v2, v0, v1, v2))                     // check triangles in this bin
                {
                    isLegit = true;
                    break;
                }
            }            
        }
    }
    return isLegit;
}
    
bool GenerateStrips(const unsigned short* in_indices, const unsigned int in_numIndices, PrimitiveGroup** primGroups, unsigned short* numGroups, bool validateEnabled)
{
    WordVec tempIndices;                                                                                                            // put data in format that the stripifier likes
    tempIndices.resize(in_numIndices);
    unsigned short maxIndex = 0;
    unsigned short minIndex = 0xFFFF;
    for(int i = 0; i < in_numIndices; i++)
    {
        tempIndices[i] = in_indices[i];
        if (in_indices[i] > maxIndex) maxIndex = in_indices[i];
        if (in_indices[i] < minIndex) minIndex = in_indices[i];
    }
    NvStripInfoVec tempStrips;
    NvFaceInfoVec tempFaces;
    NvStripifier stripifier;
    
    stripifier.Stripify(tempIndices, cacheSize, minStripSize, maxIndex, tempStrips, tempFaces);                                     // do actual stripification
    
    IntVec stripIndices;
    unsigned int numSeparateStrips = 0;

    if(bListsOnly)
    {        
        *numGroups = 1;                                                                                                             // if we're outputting only lists, we're done
        (*primGroups) = new PrimitiveGroup[*numGroups];
        PrimitiveGroup* primGroupArray = *primGroups;
        
        unsigned int numIndices = 0;                                                                                                // count the total number of indices
        for(int i = 0; i < tempStrips.size(); i++)
        {
            numIndices += tempStrips[i]->m_faces.size() * 3;
        }
        
        numIndices += tempFaces.size() * 3;                                                                                         // add in the list

        primGroupArray[0].type       = PT_LIST;
        primGroupArray[0].numIndices = numIndices;
        primGroupArray[0].indices    = new unsigned short[numIndices];
        
        unsigned int indexCtr = 0;                                                                                                  // do strips
        for(int i = 0; i < tempStrips.size(); i++)
        {
            for(int j = 0; j < tempStrips[i]->m_faces.size(); j++)
            {
                if(!NvStripifier::IsDegenerate(tempStrips[i]->m_faces[j]))                                                          // degenerates are of no use with lists
                {
                    primGroupArray[0].indices[indexCtr++] = tempStrips[i]->m_faces[j]->m_v0;
                    primGroupArray[0].indices[indexCtr++] = tempStrips[i]->m_faces[j]->m_v1;
                    primGroupArray[0].indices[indexCtr++] = tempStrips[i]->m_faces[j]->m_v2;
                }
                else
                    primGroupArray[0].numIndices -= 3;                                                                              // we've removed a tri, reduce the number of indices
            }
        }
        
        for(int i = 0; i < tempFaces.size(); i++)                                                                                   // do lists
        {           
            primGroupArray[0].indices[indexCtr++] = tempFaces[i]->m_v0;
            primGroupArray[0].indices[indexCtr++] = tempFaces[i]->m_v1;
            primGroupArray[0].indices[indexCtr++] = tempFaces[i]->m_v2;
        }
    }
    else
    {
        stripifier.CreateStrips(tempStrips, stripIndices, bStitchStrips, numSeparateStrips, bRestart, restartVal);
        assert( (bStitchStrips && (numSeparateStrips == 1)) || !bStitchStrips);                                                     // if we're stitching strips together, we better get back only one strip from CreateStrips()
                                                                                                                                    // convert to output format
        
        *numGroups = numSeparateStrips;                                                                                             // for the strips
        if(tempFaces.size() != 0)
            (*numGroups)++;                                                                                                         // we've got a list as well, increment
        (*primGroups) = new PrimitiveGroup[*numGroups];
        
        PrimitiveGroup* primGroupArray = *primGroups;
        
        int startingLoc = 0;                                                                                                        // first, the strips
        for(int stripCtr = 0; stripCtr < numSeparateStrips; stripCtr++)
        {
            int stripLength = 0;

            if(!bStitchStrips)
            {
                int i;                                                                                                              // if we've got multiple strips, we need to figure out the correct length
                for(i = startingLoc; i < stripIndices.size(); i++)
                {
                    if(stripIndices[i] == -1)
                        break;
                }
                stripLength = i - startingLoc;
            }
            else
                stripLength = stripIndices.size();
            
            primGroupArray[stripCtr].type       = PT_STRIP;
            primGroupArray[stripCtr].indices    = new unsigned short[stripLength];
            primGroupArray[stripCtr].numIndices = stripLength;
            
            int indexCtr = 0;
            for(int i = startingLoc; i < stripLength + startingLoc; i++)
                primGroupArray[stripCtr].indices[indexCtr++] = stripIndices[i];
            
            startingLoc += stripLength + 1;                                                                                         // we add 1 to account for the -1 separating strips this doesn't break the stitched case since we'll exit the loop
        }
                
        if(tempFaces.size() != 0)                                                                                                   // next, the list
        {
            int faceGroupLoc = (*numGroups) - 1;                                                                                    // the face group is the last one
            primGroupArray[faceGroupLoc].type       = PT_LIST;
            primGroupArray[faceGroupLoc].indices    = new unsigned short[tempFaces.size() * 3];
            primGroupArray[faceGroupLoc].numIndices = tempFaces.size() * 3;
            int indexCtr = 0;
            for(int i = 0; i < tempFaces.size(); i++)
            {
                primGroupArray[faceGroupLoc].indices[indexCtr++] = tempFaces[i]->m_v0;
                primGroupArray[faceGroupLoc].indices[indexCtr++] = tempFaces[i]->m_v1;
                primGroupArray[faceGroupLoc].indices[indexCtr++] = tempFaces[i]->m_v2;
            }
        }
    }
    
    if (validateEnabled)                                                                                                            // validate generated data against input
    {
        const int NUMBINS = 100;
        std::vector<NvFaceInfo> in_bins[NUMBINS];
        
        for (int i = 0; i < in_numIndices; i += 3)                                                                                  // hash input indices on first index
        {
            NvFaceInfo faceInfo(in_indices[i], in_indices[i + 1], in_indices[i + 2]);
            in_bins[in_indices[i] % NUMBINS].push_back(faceInfo);
        }
        
        for (int i = 0; i < *numGroups; ++i)
        {
            switch ((*primGroups)[i].type)
            {
                case PT_LIST:
                {
                    for (int j = 0; j < (*primGroups)[i].numIndices; j += 3)
                    {
                        unsigned short v0 = (*primGroups)[i].indices[j];
                        unsigned short v1 = (*primGroups)[i].indices[j + 1];
                        unsigned short v2 = (*primGroups)[i].indices[j + 2];
                                                
                        if (NvStripifier::IsDegenerate(v0, v1, v2)) continue;                                                       // ignore degenerates

                        if (!TestTriangle(v0, v1, v2, in_bins, NUMBINS))
                        {
                            Cleanup(tempStrips, tempFaces);
                            return false;
                        }
                    }
                    break;
                }

                case PT_STRIP:
                {
                    int brokenCtr = 0;
                    bool flip = false;
                    for (int j = 2; j < (*primGroups)[i].numIndices; ++j)
                    {
                        unsigned short v0 = (*primGroups)[i].indices[j - 2];
                        unsigned short v1 = (*primGroups)[i].indices[j - 1];
                        unsigned short v2 = (*primGroups)[i].indices[j];
                        
                        if (flip) std::swap(v1, v2);                                                                                // swap v1 and v2
                        
                        if (NvStripifier::IsDegenerate(v0, v1, v2))                                                                 // ignore degenerates
                        {
                            flip = !flip;
                            continue;
                        }

                        if (!TestTriangle(v0, v1, v2, in_bins, NUMBINS))
                        {
                            Cleanup(tempStrips, tempFaces);
                            return false;
                        }

                        flip = !flip;
                    }
                    break;
                }

                case PT_FAN:
                default:
                    break;
            }
        }
    }

    Cleanup(tempStrips, tempFaces);
    return true;
}

void RemapIndices(const PrimitiveGroup* in_primGroups, const unsigned short numGroups, const unsigned short numVerts, PrimitiveGroup** remappedGroups)
{
    (*remappedGroups) = new PrimitiveGroup[numGroups];

    int *indexCache;                                                                                                                // caches oldIndex --> newIndex conversion
    indexCache = new int[numVerts];
    memset(indexCache, -1, sizeof(int) * numVerts);
        
    unsigned int indexCtr = 0;                                                                                                      // loop over primitive groups
    for(int i = 0; i < numGroups; i++)
    {
        unsigned int numIndices = in_primGroups[i].numIndices;
        
        (*remappedGroups)[i].type       = in_primGroups[i].type;                                                                    // init remapped group
        (*remappedGroups)[i].numIndices = numIndices;
        (*remappedGroups)[i].indices    = new unsigned short[numIndices];

        for(int j = 0; j < numIndices; j++)
        {
            int cachedIndex = indexCache[in_primGroups[i].indices[j]];
            if(cachedIndex == -1) //we haven't seen this index before
            {
                (*remappedGroups)[i].indices[j] = indexCtr;                                                                         // point to "last" vertex in VB
                indexCache[in_primGroups[i].indices[j]] = indexCtr++;                                                               // add to index cache, increment
            }
            else
                (*remappedGroups)[i].indices[j] = cachedIndex;                                                                      // we've seen this index before
        }
    }

    delete[] indexCache;
}