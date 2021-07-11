#include "ObjectPool.h"
#include "ConcurrentAlloc.h"

void func1()
{
	for (size_t i = 0; i < 10; ++i)
	{
		ConcurrentAlloc(17);
	}
}

void func2()
{
	for (size_t i = 0; i < 20; ++i)
	{
		ConcurrentAlloc(5);
	}
}

void TestThreads()
{
	std::thread t1(func1);
	std::thread t2(func2);


	t1.join();
	t2.join();
}

void TestSizeClass()
{
	cout << SizeClass::Index(1035) << endl;
	cout << SizeClass::Index(1025) << endl;
	cout << SizeClass::Index(1024) << endl;
}

int main()
{
	//TestObjectPool();
	//TestThreads();
	TestSizeClass();

	return 0;
}