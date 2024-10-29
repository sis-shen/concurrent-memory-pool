#pragma once

#include <iostream>
#include <vector>
#include <time.h>
#include <cassert>

using std::cout;
using std::endl;

//�����зֺõ�С�������������

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
		//ͷ��
		NextObj(obj) = _freeList;//��objǰһ��ָ����ڴ�鸳ֵ�ϵ�ַ
		_freeList = obj;
	}

	void* Pop()
	{
		assert(_freeList);
		//ͷɾ
		void* obj = _freeList;
		_freeList = NextObj(obj);
		return obj;
	}
private:
	void* _freeList;
};

class SizeClass
{
public:
	static size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{

		}
		else if (size <= 1024)
		{

		}
		else if (size <= 8 * 1024)
		{

		}
		else if (size <= 64 * 1024)
		{

		}
		else if (size <= 256 * 1024)
		{

		}
		else
		{
			assert(false);
		}

	}
};