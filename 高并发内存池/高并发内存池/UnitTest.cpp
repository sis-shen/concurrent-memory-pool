#include "Common.h"
#include "ConcurrentAlloc.h"
#include <thread>
using namespace std;

void Alloc1()
{
	for (int i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}

void Alloc2()
{
	for (int i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}

void TLSTest()
{
	thread t1(Alloc1);
	thread t2(Alloc2);

	t1.join();
	t2.join();
}

int main()
{
	TLSTest();
	return 0;
}