#pragma once
#include"CondurrentAlloc.h"
//#include<map>
//
//std::map<int, ThreadCache> idmap;
//void func1()
//{
//	for (int i = 0; i < 10; ++i)
//	{
//		ConcurrentAlloc(15);
//	}
//}
//void func2()
//{
//	for (int i = 0; i < 10; ++i)
//	{
//		ConcurrentAlloc(5);
//	}
//}
//void TestThread()
//{
//	std::thread t1(func1);
//	std::thread t2(func2);
//
//	t1.join();
//	t2.join();
//	
//}
//void testCalsizeClass()
//{
//	//1024在71个桶
//	cout<<SizeClass::Index(1024)<<endl;
//	//1025在72个桶
//	cout << SizeClass::Index(1025) << endl;
//
//}
void TestConcurrentAlloc()
{
	void* ptr1 = ConcurrentAlloc(8);
	void* ptr2 = ConcurrentAlloc(8);
}
int main()
{

	TestConcurrentAlloc();
	return 0;
}