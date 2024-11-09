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

	//��ȡ�Ӷ���span��ӳ��
	Span* MapObjToSpan(void* obj);

	//��ȡkҳ��span
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

