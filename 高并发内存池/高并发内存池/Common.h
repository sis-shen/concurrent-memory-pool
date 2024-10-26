#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <cassert>

using std::cout;
using std::endl;

//管理切分好的小对象的自由链表

void*& NextObj(void* obj)
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
private:
	void* _freeList;
};