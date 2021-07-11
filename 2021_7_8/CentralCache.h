#pragma once
#include "Common.h"

class CentralCache
{
public:
	// 从中心缓存获取一定数量的对象给thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

	// 从SpanList或者page cache获取一个span
	Span* GetOneSpan(SpanList& list, size_t byte_size);
private:
	SpanList _spanLists[NFREELISTS]; // 按对齐方式映射
};

CentralCache centralCache;