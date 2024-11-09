#include "PageCache.h"

PageCache PageCache::_sInstan;

Span* PageCache::MapObjToSpan(void* obj)
{
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);
	auto ret = _idSpanMap.find(id);
	if (ret != _idSpanMap.end())
	{
		return ret->second;
	}

	assert(false);
	return nullptr;
}

Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0 && k < NPAGES);

	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		Span* span = _spanPool.New();
		span->_pageID = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		//建立id和span的映射
		_idSpanMap[span->_pageID] = span;
		return span;
	}

	if (!_spanLists[k].Empty())
	{
		return _spanLists[k].PopFront();
	}

	//检查一下后面的桶有没有大的span,如果有就切分
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nSpan = _spanLists[i].PopFront();
			//Span* kSpan = new Span;
			Span* kSpan = _spanPool.New();

			kSpan->_pageID = nSpan->_pageID;
			kSpan->_n = k;

			nSpan->_pageID += k;
			nSpan->_n -= k;

			_spanLists[nSpan->_n].PushFront(nSpan);

			//建立id和span的映射
			for (PAGE_ID i = 0; i < kSpan->_n; ++i)
			{
				_idSpanMap[kSpan->_pageID + i] = kSpan;
			}

			return kSpan;
		}
	}
	Span* bigSpan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageID = (PAGE_ID)ptr >> PAGE_SHIFT;
	_idSpanMap[bigSpan->_pageID] = bigSpan;
	bigSpan->_n = NPAGES - 1;
	_spanLists[bigSpan->_n].PushFront(bigSpan);

	//建立id和span的映射
	for (PAGE_ID i = 0; i < bigSpan->_n; ++i)
	{
		_idSpanMap[bigSpan->_pageID + i] = bigSpan;
	}

	return NewSpan(k);
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//_spanLists[span->_n].Erase(span);
	_idSpanMap.erase(span->_pageID);
	if (span->_n > NPAGES - 1)
	{
		void* ptr = (void*)(span->_n << PAGE_SHIFT);
		SystemFree(ptr,span->_objSize);
		_spanPool.Delete(span);
		return;
	}
	//向前合并
	while (true)
	{
		PAGE_ID prevID = span->_pageID - 1;
		auto ret = _idSpanMap.find(prevID);
		//前面的页号没有，不合并了
		if (ret == _idSpanMap.end())
		{
			break;
		}
		//如果在使用，不合并了
		Span* prevSpan = ret->second;
		if (prevSpan->_isUse == true)
		{
			break;
		}
		//不能合并出过大的span
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}

		span->_pageID = prevSpan->_pageID;
		span->_n+= prevSpan->_n;

		_spanLists[prevSpan->_n].Erase(prevSpan);
		_spanPool.Delete(prevSpan);
	}

	//向后合并
	while (true)
	{
		PAGE_ID nextID = span->_pageID + 1;
		auto ret = _idSpanMap.find(nextID);
		if (ret == _idSpanMap.end())
		{
			break;
		}

		Span* nextSpan = ret->second;
		if (nextSpan->_isUse == true)
		{
			break;
		}
		if (nextSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}

		span->_n += nextSpan->_n;
		_spanLists[nextSpan->_n].Erase(nextSpan);
		_idSpanMap.erase(nextSpan->_pageID);
		_spanPool.Delete(nextSpan);
	}

	_spanLists[span->_n].PushFront(span);
	span->_isUse = false;
	_idSpanMap[span->_pageID] = span;
	_idSpanMap[span->_pageID + span->_n] = span;
}
