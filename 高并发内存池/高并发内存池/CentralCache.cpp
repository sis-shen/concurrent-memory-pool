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
			return it;//�ҵ���Ϊ�յ�span
		}
		else
		{
			it = it->_next;
		}
	}
	//�Ȱ�central cache��Ͱ������������������������߳��ͷ��ڴ����
	list._mtx.unlock();

	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	span->_isUse = true;
	span->_objSize = byte_size;
	PageCache::GetInstance()->_pageMtx.unlock();

	//��span�����з֣���ʱ��span�Ƕ���ģ�����Ҫ��������

	char* start = (char*)(span->_pageID << PAGE_SHIFT);
	size_t bytes = (span->_n << PAGE_SHIFT);

	char* end = (char*)start + bytes;

	//�Ѵ���������������
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

	//�к��Ժ�Ҫ��span������
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
