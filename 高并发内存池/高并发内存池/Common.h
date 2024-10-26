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