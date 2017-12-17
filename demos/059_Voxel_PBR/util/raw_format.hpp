#pragma once
#include <queue>

struct RawFormat
{
    template<typename T, size_t _Count> struct DataSegment;

	struct Segment;
	bool isBuilt;
	void* rawDataPointer;
	size_t wholeSize;
	std::queue<Segment *> format;

	RawFormat();
	~RawFormat();

	void * RawData();								// Returns the raw data pointer = pointer holding all the stacked data. Will call Build if the raw format hasn't been built already
	size_t Size() const								// Size of this data segment
		{ return wholeSize; };			
	void Build();									// Calls BuildRawData. This function will only work if isBuilt is false.

	void SegmentPush(Segment * segment);
	void* BuildRawData();
};
