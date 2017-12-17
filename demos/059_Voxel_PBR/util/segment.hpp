#pragma once
#include "raw_format.hpp"

struct RawFormat::Segment
{
    size_t size;					// The segment size
    size_t offset;					// The segment offset

    Segment(size_t size, size_t offset)
        : size(size), offset(offset) 
        { }

    Segment(const Segment& other) = delete;
    Segment() = delete;
};