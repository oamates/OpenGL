#include <cassert>
#include <algorithm>

#include "binpacker.hpp"

void bin_packer_t::pack(const std::vector<int>& rects, std::vector<std::vector<int>>& packs, int packSize)
{
    m_numPacked = 0;
    m_rects.clear();
    m_packs.clear();
    m_roots.clear();

    m_packSize = packSize;
    
    for (size_t i = 0; i < rects.size(); i += 2)                                                // Add rects to member array, and check to make sure none is too big
    {
        if (rects[i] > m_packSize || rects[i + 1] > m_packSize)
            assert(!"All rect dimensions must be <= the pack size");
        m_rects.push_back(rect_t(0, 0, rects[i], rects[i + 1], i >> 1));
    }
    
    std::sort(m_rects.rbegin(), m_rects.rend());                                                // Sort from greatest to least area
    
    while (m_numPacked < (int)m_rects.size())                                                   // Pack
    {
        int i = m_packs.size();
        m_packs.push_back(rect_t(m_packSize));
        m_roots.push_back(i);
        fill(i);
    }
    
    packs.resize(m_roots.size());                                                               // Write out
    for (size_t i = 0; i < m_roots.size(); ++i)
    {
        packs[i].clear();
        AddPackToArray(m_roots[i], packs[i]);
    }
    
    for (size_t i = 0; i < m_rects.size(); ++i)                                                 // Check and make sure all rects were packed
        if (!m_rects[i].packed) assert(!"Not all rects were packed");
}

void bin_packer_t::fill(int pack)
{
    for (size_t j = 0; j < m_rects.size(); ++j)                                                 // For each rect
    {
        if (!m_rects[j].packed && fits(m_rects[j], m_packs[pack]))                              // If it's not already packed and fits in the current working area
        {
            ++m_numPacked;                                                                      // Store in lower-left of working area, split, and recurse
            split(pack, j);
            fill(m_packs[pack].children[0]);
            fill(m_packs[pack].children[1]);
            return;
        }
    }
}

void bin_packer_t::split(int pack, int rect)
{
    int i = pack;
    int j = rect;

    // Split the working area either horizontally or vertically with respect to the rect we're storing, such that we get the largest possible child area.

    rect_t left = m_packs[i];
    rect_t right = m_packs[i];
    rect_t bottom = m_packs[i];
    rect_t top = m_packs[i];

    left.y += m_rects[j].h;
    left.w = m_rects[j].w;
    left.h -= m_rects[j].h;
    right.x += m_rects[j].w;
    right.w -= m_rects[j].w;

    bottom.x += m_rects[j].w;
    bottom.h = m_rects[j].h;
    bottom.w -= m_rects[j].w;
    top.y += m_rects[j].h;
    top.h -= m_rects[j].h;

    int max_lr_area = std::max(left.area(), right.area());
    int max_tb_area = std::max(bottom.area(), top.area());

    if (max_lr_area > max_tb_area)
    {
        if (left.area() > right.area())
        {
            m_packs.push_back(left);
            m_packs.push_back(right);
        }
        else
        {
            m_packs.push_back(right);
            m_packs.push_back(left);
        }
    }
    else
    {
        if (bottom.area() > top.area())
        {
            m_packs.push_back(bottom);
            m_packs.push_back(top);
        }
        else
        {
            m_packs.push_back(top);
            m_packs.push_back(bottom);
        }
    }

    // This pack area now represents the rect we've just stored, so save the relevant info to it, and assign children.
    m_packs[i].w = m_rects[j].w;
    m_packs[i].h = m_rects[j].h;
    m_packs[i].id = m_rects[j].id;
    m_packs[i].children[0] = m_packs.size() - 2;
    m_packs[i].children[1] = m_packs.size() - 1;

    // Done with the rect
    m_rects[j].packed = true;
}

void bin_packer_t::AddPackToArray(int idx, std::vector<int>& array) const
{
    if (m_packs[idx].id != -1)
    {
        array.push_back(m_packs[idx].id);
        array.push_back(m_packs[idx].x);
        array.push_back(m_packs[idx].y);
        if (m_packs[idx].children[0] != -1) AddPackToArray(m_packs[idx].children[0], array);
        if (m_packs[idx].children[1] != -1) AddPackToArray(m_packs[idx].children[1], array);
    }
}