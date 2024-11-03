#include "PageCache.h"

PageCache PageCache::_sInstan;

Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0 && k < NPAGES);

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
			Span* kSpan = new Span;

			kSpan->_pageID = nSpan->_pageID;
			kSpan->_n = k;

			nSpan->_pageID += k;
			nSpan->_n -= k;

			return kSpan;
		}
	}
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageID = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}
