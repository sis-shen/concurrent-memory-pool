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
	PageCache::GetInstance()->_pageMtx.unlock();

	//��span�����з֣���ʱ��span�Ƕ���ģ�����Ҫ��������

	char* start = (char*)(span->_pageID << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;

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
	NextObj(end) = nullptr;
	_spanLists[index]._mtx.unlock();

	return actualNum;
}