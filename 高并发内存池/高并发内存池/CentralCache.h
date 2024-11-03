#pragma once

#include "Common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}
	
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

private:
	SpanList _spanLists[NFREE_LISTS];
private:
	CentralCache() {}

	CentralCache(const CentralCache&) = delete;

	CentralCache operator=(const CentralCache&) = delete;

	static CentralCache _sInst;
};
