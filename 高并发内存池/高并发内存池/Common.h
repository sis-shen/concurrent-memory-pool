#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <cassert>
#include <thread>
#include <algorithm>
#include <mutex>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#else
	#include <sys/mman.h>
#endif


using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;//设置分配的最大的字节
static const size_t NFREE_LISTS = 208;//设置最大哈希桶的数量
static const size_t NPAGES = 128;//多少页
static const size_t PAGE_SHIFT = 13;//一页是2的多少次方

#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#endif 


//直接去堆上按页申请空间
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	void* ptr = mmap(0, kpage << PAGE_SHIFT, PORT_READ | PORT_WRITE, MAP_ANONYMOUS, -1, 0);
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}

inline static void SystemFree(void* ptr,size_t size)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);;
#else
	//Linux
	unmmap(ptr, size);
#endif
}


//管理切分好的小对象的自由链表

static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

class FreeList
{
public:
	void Push(void* obj)
	{
		assert(obj);
		//头插
		NextObj(obj) = _freeList;//给obj前一个指针的内存块赋值上地址
		_freeList = obj;
		_size++;
	}

	void PopRange(void* &start, void* &end, size_t n)
	{
		assert(n >= _size);
		start = _freeList;
		end = start;
		for (size_t i = 0; i < n - 1; ++i)
		{
			end = NextObj(end);
		}
		_freeList = NextObj(end);
		NextObj(end) = nullptr;
		_size -= n;
	}

	void PushRange(void* start, void* end,size_t n)
	{
		NextObj(end) = _freeList;
		_freeList = start;
		_size += n;
	}

	void* Pop()
	{
		assert(_freeList);
		//头删
		void* obj = _freeList;
		_freeList = NextObj(obj);
		--_size;
		return obj;
	}

	bool Empty()
	{
		return _freeList == nullptr;
	}

	size_t& MaxSize()
	{
		return _maxSize;
	}

	size_t& Size()
	{
		return _size;
	}
private:
	void* _freeList = nullptr;
	size_t _maxSize = 1;
	size_t _size = 0;
};

class SizeClass
{
public:

	static size_t _RoundUp(size_t size, size_t AlignNum)
	{
		//前提：AlignNum是16的倍数
		return (size + AlignNum - 1) & ~(AlignNum - 1);
	}

	static size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			return _RoundUp(size, 1 << PAGE_SHIFT);
		}
		return 0;
	}
	static size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		//每个区间有多少条链表
		static int group_arry[4] = { 16,56,56,56 };
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024)
		{
			return _Index(bytes - 128, 4) + group_arry[0];
		}
		else if (bytes <= 8 * 1024)
		{
			return _Index(bytes - 1024, 7) + group_arry[0] + group_arry[1];
		}
		else if (bytes <= 64 * 1024)
		{
			return _Index(bytes - 8 * 1024, 10) + group_arry[0] + group_arry[1] + group_arry[2];
		}
		else if (bytes <= 256 * 1024)
		{
			return _Index(bytes - 64 * 1024, 13) + group_arry[0] + group_arry[1] + group_arry[2] + group_arry[3];
		}
		else
		{
			assert(false);
			return 0;
		}
	}

	//thread cache一次从中心缓存获取多少个
	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);

		size_t num = MAX_BYTES / size;

		if (num < 2)
			num = 2;
		else if (num > 512)
			num = 512;

		return num;
	}

	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;

		if (npage == 0)//至少拿一页
			npage = 1;

		return npage;
	}

};

struct Span
{
public:
	PAGE_ID _pageID = 0;//大块内存起始页的页号
	size_t _n = 0;//页的数量

	Span* _next = nullptr;
	Span* _prev = nullptr;

	size_t _useCount = 0;//切好的小块内存的引用计数
	void* _freeList = nullptr;//切好的空闲小块内存的自由链表
	bool _isUse = false;
	size_t _objSize = 0;
};

//带头双向循环链表
class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin() 
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

	Span* PopFront()
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}

	void PushFront(Span* newSpan)
	{
		Insert(_head->_next,newSpan);
	}

	void Insert(Span* pos, Span* newSpan)
	{
		assert(pos);
		assert(newSpan);

		Span* prev = pos->_prev;;
		prev->_next = newSpan;
		newSpan->_prev = prev;
		newSpan->_next = pos;
		pos->_prev = newSpan;
	}

	void Erase(Span* pos)
	{
		assert(pos);
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}

private:
	Span* _head;
public:
	std::mutex _mtx;
};