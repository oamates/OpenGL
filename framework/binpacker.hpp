#ifndef _binpacker_included_079681724572468292136864860873127746970731463113417
#define _binpacker_included_079681724572468292136864860873127746970731463113417

#include <vector>

//=======================================================================================================================================================================================================================
// generic bin packer algorithm
//=======================================================================================================================================================================================================================

struct bin_packer_t
{
    // rects : array containing dimensions each input rect in sequence, i.e. [w0][h0][w1][h1][w2][h2]... 
    // The IDs for the rects are derived from the order in which they appear in the array.
    
    // packs : After packing, the outer array contains the packs (therefore the number of packs is packs.size()). Each inner array contains a
    // sequence of sets of 4 ints. Each set represents a rectangle in the pack. The elements in the set are 1) the rect ID, 2) the x position
    // of the rect with respect to the pack, 3) the y position of the rect with respect to the pack, and 4) whether the rect was rotated (1) or not (0). 
    // The widths and heights of the rects are not included, as it's assumed they are stored on the caller's side (they were after all the input to the function).
    
    // allowRotation : when true (the default value), the packer is allowed the option of rotating the rects in the process of trying to fit them
    // into the current working area.

    struct rect_t
    {
        int id;
        int x, y;
        int w, h;
        int children[2];
        bool packed;

        rect_t(int size) : x(0), y(0), w(size), h(size), id(-1), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }

        rect_t(int x, int y, int w, int h, int id)
            : x(x), y(y), w(w), h(h), id(id), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }
        
        int area() const 
            { return w * h; }
        
        bool operator < (const rect_t& rect) const
            { return area() < rect.area(); }

    };

    int m_packSize;
    int m_numPacked;
    std::vector<rect_t> m_rects;
    std::vector<rect_t> m_packs;
    std::vector<int> m_roots;

    void pack(const std::vector<int>& rects, std::vector<std::vector<int>>& packs, int packSize);

    void fill(int pack);
    void split(int pack, int rect);
    bool fits(const rect_t& rect1, const rect_t& rect2)
        { return rect1.w <= rect2.w && rect1.h <= rect2.h; }

    void AddPackToArray(int pack, std::vector<int>& array) const;
    
};

#endif // _binpacker_included_079681724572468292136864860873127746970731463113417
