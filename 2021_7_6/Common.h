#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <time.h>
#include <assert.h>

#include <thread>

using std::cout;
using std::endl;

inline void*& NextObj(void* obj)
{
	return *((void**)obj);
}

//size_t Index(size_t size)
//{
//	if (size % 8 == 0)
//	{
//		return size / 8 - 1;
//	}
//	else
//	{
//		return size / 8;
//	}
//}

// 8 + 7 = 15
// 7 + 7
// ...
// 1 + 7 = 8
static size_t Index(size_t size)
{
	return ((size + (2^3 - 1)) >> 3) - 1;
}

class FreeList
{
public:
	// Í·²å
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
	}

	// Í·É¾
	void* Pop()
	{
		void* obj = _head;
		_head = NextObj(_head);

		return obj;
	}

	bool Empty()
	{
		return _head == nullptr;
	}

private:
	void* _head = nullptr;
};