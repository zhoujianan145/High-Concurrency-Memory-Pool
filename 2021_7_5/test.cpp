#pragma once
#include"CondurrentAlloc.h"
//#include<map>
//
//std::map<int, ThreadCache> idmap;
void func1()
{
	for (int i = 0; i < 10; ++i)
	{
		ConcurrentAlloc(15);
	}
}
void func2()
{
	for (int i = 0; i < 10; ++i)
	{
		ConcurrentAlloc(5);
	}
}
void TestThread()
{
	std::thread t1(func1);
	std::thread t2(func2);

	t1.join();
	t2.join();
	
}

int main()
{
	/*zjn::TestObjectPool1();*/
	//extern void* Index(size_t size);
	/*Index(8);*/

	TestThread();
	return 0;
}