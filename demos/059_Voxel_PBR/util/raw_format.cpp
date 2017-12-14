#include "raw_format.hpp"
#include "data_segment.hpp"

void * RawFormat::BuildRawData()
{
    if (rawDataPointer)
        free(rawDataPointer);

    auto data = reinterpret_cast<unsigned char *>(malloc(wholeSize));
    auto begin = data;

    while (!format.empty())
    {
        auto segment = static_cast<DataSegment<unsigned char> *>(format.front());       // transfer data
        data += segment->offset;
        if (segment->pointer)
        {
            memcpy(data, segment->pointer, segment->size);
            free(segment->pointer);
        }
        segment->pointer = data;
        data += segment->size;        
        format.pop();                                                                   // pop format stack
    }
    return rawDataPointer = begin;                                                      // return whole pointer
}

void * RawFormat::RawData()
{
    if (rawDataPointer)
        return rawDataPointer;
    Build();                                                                            // build whole data
    return rawDataPointer;                                                              // return built raw pointer
}

RawFormat::RawFormat() : isBuilt(false), rawDataPointer(nullptr), wholeSize(0) { }

void RawFormat::Build()
{
    if (!isBuilt)
    {
        rawDataPointer = BuildRawData();                                                // can't enter this function anymore once whole data pointer is built
        isBuilt = true;
    }
}

RawFormat::~RawFormat()
{
    if (rawDataPointer)
        free(rawDataPointer);
    rawDataPointer = nullptr;
}

void RawFormat::SegmentPush(Segment * segment)
{
    if (isBuilt)
        throw std::logic_error("RawFormat is already built, can't push more data segments");

    format.push(segment);                                                               // build format queue
    wholeSize += segment->size + segment->offset;
}
