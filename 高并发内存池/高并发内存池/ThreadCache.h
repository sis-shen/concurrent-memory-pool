#pragma once
#include "Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);

	void Deallocate(void* ptr, size_t size);

	void* FetchFromCentralCache(size_t index,size_t aliginsize);

	void ListTooLong(FreeList& list, size_t size);
private:
	FreeList _freeLists[NFREE_LISTS];
};

static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;