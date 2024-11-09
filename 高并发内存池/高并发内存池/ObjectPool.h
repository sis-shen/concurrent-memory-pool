#pragma once

#include <iostream>
#include "Common.h"

//�����ڴ��
template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		if (_freeList != nullptr)
		{
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;

		}
		else
		{
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 128 * 1024;
				_memory = (char*)SystemAlloc(_remainBytes >> PAGE_SHIFT);
				if (_memory == nullptr)
				{
					throw std::bad_alloc();
				}
			}

			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_remainBytes -= objSize;
		}
		new(obj)T;

		return obj;
	}

	void Delete(T* obj)
	{
		//��ʽ������������ 
		obj->~T();
		//����ͷ��
		*(void**)obj = _freeList;
		_freeList = obj;
	}

private:
	char* _memory; //ָ�����ڴ�
	size_t _remainBytes = 0;//����ڴ��ʣ���ֽ���
	void* _freeList = nullptr;
};