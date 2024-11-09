#pragma once

#include "Common.h"
#include "ObjectPool.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInstan;
	}

	//获取从对象到span的映射
	Span* MapObjToSpan(void* obj);

	//获取k页的span
	Span* NewSpan(size_t k);

	void ReleaseSpanToPageCache(Span* span);

private:
	SpanList _spanLists[NPAGES];
	ObjectPool<Span> _spanPool;

	static PageCache _sInstan;

private:
	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache operator=(const PageCache&) = delete;
public:
	std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	std::mutex _pageMtx;
};

