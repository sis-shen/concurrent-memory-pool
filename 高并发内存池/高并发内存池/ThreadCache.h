#pragma once
#include "Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);

	void Deallocate(void* ptr, size_t size);

private:

};
