#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <cassert>
#include <thread>


using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;//设置分配的最大的字节
static const size_t NFREE_LISTS = 208;//设置最大哈希桶的数量

#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#endif 


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
	}

	void* Pop()
	{
		assert(_freeList);
		//头删
		void* obj = _freeList;
		_freeList = NextObj(obj);
		return obj;
	}

	bool Empty()
	{
		return _freeList == nullptr;
	}
private:
	void* _freeList = nullptr;
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
			assert(false);
		}

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
};

struct Span
{
	PAGE_ID _pageID;//大块内存起始页的页号
	size_t _n;//页的数量

	Span* _next;
	Span* _prev;

	size_t _useCountl;//切好的小块内存的引用计数
	void* _freeList;//切好的空闲小块内存的自由链表
};