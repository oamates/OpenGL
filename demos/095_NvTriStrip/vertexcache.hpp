#ifndef VERTEX_CACHE_H
#define VERTEX_CACHE_H

struct VertexCache
{
    int *entries;
    int numEntries;
    
    VertexCache(int size = 16)
    {
        numEntries = size;
        entries = new int[numEntries];
        for(int i = 0; i < numEntries; i++) entries[i] = -1;
    }
        
    ~VertexCache() 
        { delete[] entries; }
    
    bool InCache(int entry)
    {
        for(int i = 0; i < numEntries; i++)
            if (entries[i] == entry) 
                return true;

        return false;
    }
    
    int AddEntry(int entry)
    {
        int removed = entries[numEntries - 1];
        
        for(int i = numEntries - 2; i >= 0; i--)
            entries[i + 1] = entries[i];
        
        entries[0] = entry;
        
        return removed;
    }

    void Clear()
    {
        for(int i = 0; i < numEntries; i++)
            entries[i] = -1;
    }
    
    void Copy(VertexCache* inVcache) 
    {
        for(int i = 0; i < numEntries; i++)
            inVcache->Set(i, entries[i]);
    }

    int At (int index) 
        { return entries[index]; }

    void Set(int index, int value) 
        { entries[index] = value; }

};

#endif
