#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr)
		{
			return it;//找到不为空的span
		}
		else
		{
			it = it->_next;
		}
	}
	//先把central cache的桶锁解掉，这样不会阻塞其它线程释放内存对象
	list._mtx.unlock();

	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	span->_isUse = true;
	span->_objSize = byte_size;
	PageCache::GetInstance()->_pageMtx.unlock();

	//对span进行切分，此时的span是独享的，不需要加锁保护

	char* start = (char*)(span->_pageID << PAGE_SHIFT);
	size_t bytes = (span->_n << PAGE_SHIFT);

	char* end = (char*)start + bytes;

	//把大块的自由链表穿起来
	span->_freeList = start;
	start += byte_size;
	void* tail = span->_freeList;

	while (start < end)
	{
		NextObj(tail) = start;
		tail = start;
		start += byte_size;
	}

	NextObj(tail) = nullptr;

	//切好以后要把span挂起来
	list._mtx.lock();
	list.PushFront(span);

	return span;
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index],size);
	assert(span);
	assert(span->_freeList);

	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	for (i = 0; i < batchNum && NextObj(end) != nullptr;)
	{
		end = NextObj(end);
		++i;
		++actualNum;
	}
	span->_freeList = NextObj(end);
	span->_useCount++;
	NextObj(end) = nullptr;
	_spanLists[index]._mtx.unlock();

	return actualNum;
}

void CentralCache::ReleaseListToSpans(void* start, size_t byte_size)
{
	size_t index = SizeClass::Index(byte_size);
	_spanLists[index]._mtx.lock();

	while (start)
	{
		void* next = NextObj(start);

		Span* span = PageCache::GetInstance()->MapObjToSpan(start);
		NextObj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;
		if (span->_useCount == 0)
		{
			_spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;
			span->_next = nullptr;
			span->_isUse = false;
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);     
		}
		start = next;
	}

	_spanLists[index]._mtx.unlock();
}
