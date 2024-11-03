#pragma once

#include "Common.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInstan;
	}

	Span* NewSpan(size_t num);

private:
	SpanList _spanLists[NPAGES];

	static PageCache _sInstan;

private:
	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache operator=(const PageCache&) = delete;
public:
	std::mutex _pageMtx;
};

