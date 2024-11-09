#pragma once
#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"
#include "ObjectPool.h"

static void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES)
	{
		size_t alignSize = SizeClass::RoundUp(size);
		size_t kpage = alignSize >> PAGE_SHIFT;
		PageCache::GetInstance()->_pageMtx.lock();
		Span* span = PageCache::GetInstance()->NewSpan(kpage);
		span->_isUse = true;
		span->_objSize = alignSize;
		PageCache::GetInstance()->_pageMtx.unlock();
		void* ptr = (void*)(span->_pageID << PAGE_SHIFT);
		return ptr;
	}

	if (pTLSThreadCache == nullptr)
	{
		static ObjectPool<ThreadCache>tcPool;
		pTLSThreadCache = tcPool.New();
	}

	return pTLSThreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr)
{
	Span* span = PageCache::GetInstance()->MapObjToSpan(ptr);
	size_t size = span->_objSize;
	assert(pTLSThreadCache);
	if (size > MAX_BYTES)
	{
		Span* span = PageCache::GetInstance()->MapObjToSpan(ptr);
		PageCache::GetInstance()->_pageMtx.lock();
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		PageCache::GetInstance()->_pageMtx.unlock();
	}
	else
	{
		pTLSThreadCache->Deallocate(ptr,size);
	}
}

