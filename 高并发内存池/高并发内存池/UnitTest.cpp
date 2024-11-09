//#include "Common.h"
//#include "ConcurrentAlloc.h"
//#include <thread>
//using namespace std;
//
////void Alloc1()
////{
////	void* ptr;
////	for (int i = 0; i < 5; ++i)
////	{
////		ptr = ConcurrentAlloc(6);
////	}
////
////	ConcurrentFree(ptr);
////}
//
////void Alloc2()
////{
////	void* ptr;
////	for (int i = 0; i < 5; ++i)
////	{
////		ptr = ConcurrentAlloc(7);
////	}
////	ConcurrentFree(ptr);
////
////}
//
////void TLSTest()
////{
////	thread t1(Alloc1);
////	thread t2(Alloc2);
////
////	t1.join();
////	t2.join();
////}
//
//void TestConcurrentAlloc()
//{
//	void* ptr5 = ConcurrentAlloc(12);
//
//	ConcurrentFree(ptr5);
//	void* ptr1 = ConcurrentAlloc(8);
//	void* ptr2 = ConcurrentAlloc(2);
//	void* ptr3 = ConcurrentAlloc(4);
//	void* ptr4 = ConcurrentAlloc(1);
//	ConcurrentFree(ptr1);
//	ConcurrentFree(ptr2);
//	ConcurrentFree(ptr3);
//	ConcurrentFree(ptr4);
//
//
//}
//
//int main()
//{
//	//TLSTest();
//	TestConcurrentAlloc();
//	return 0;
//}