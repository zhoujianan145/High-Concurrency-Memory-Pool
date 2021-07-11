#pragma once

#include <iostream>
#include <exception>
#include <vector>
#include <time.h>
#include <assert.h>

#include <thread>
#include <algorithm>

using std::cout;
using std::endl;

static const size_t MAX_BYTES = 64 * 1024;
static const size_t NFREELISTS = 184;
static const size_t NPAGES = 129;



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

// 管理对齐和映射等关系
class SizeClass
{
public:
	// 控制在1%-12%左右的内碎片浪费
	// [1,128]					8byte对齐	     freelist[0,16)
	// [129,1024]				16byte对齐		 freelist[16,72)
	// [1025,8*1024]			128byte对齐	     freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)

	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// 计算映射的哪一个自由链表桶
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128){
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024){
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8192){
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 65536){
			return _Index(bytes - 8192, 10) + group_array[2] + group_array[1] + group_array[0];
		}

		assert(false);

		return -1;
	}

	// 一次从中心缓存获取多少个
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		// [2, 512]
		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;

		return num;
	}

	// 计算一次向系统获取几个页
	// 单个对象 8byte
	// ...
	// 单个对象 64KB
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num*size;

		npage >>= 12;
		if (npage == 0)
			npage = 1;

		return npage;
	}
};


class FreeList
{
public:
	void PushRange(void* start, void* end, int n)
	{

	}

	// 头插
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
	}

	// 头删
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

	size_t MaxSize()
	{
		return _max_size;
	}

	void SetMaxSize(size_t n)
	{
		_max_size = n;
	}

private:
	void* _head = nullptr;
	size_t _max_size = 1;
};

////////////////////////////////////////////////////////////
// Span
// 管理一个跨度的大块内存

// 2 ^ 32 / 2 ^ 12
// 2 ^ 64 / 2 ^ 12
typedef size_t PageID;

struct Span
{
	PageID _pageId;   // 页号
	size_t _n;        // 页的数量

	Span* _next = nullptr;
	Span* _prev = nullptr;

	void* _memory = nullptr;
	size_t _usecount = 0;  // 使用计数，==0 说明所有对象都回来了
	size_t _objsize;   // 切出来的单个对象的大小
};

class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	void Insert(Span* cur, Span* newspan)
	{
		Span* prev = cur->_prev;
		// prev newspan cur
		prev->_next = newspan;
		newspan->_prev = prev;

		newspan->_next = cur;
		cur->_prev = newspan;
	}

	void Erase(Span* cur)
	{
		assert(cur != _head);

		Span* prev = cur->_prev;
		Span* next = cur->_next;

		prev->_next = next;
		next->_prev = prev;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

private:
	Span* _head;
};