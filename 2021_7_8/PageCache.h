#pragma once
#include "Common.h"

class PageCache
{
public:
	// 向系统申请k页内存挂到自由链表
	void* SystemAllocPage(size_t k);

	Span* NewSpan(size_t k);

private:
	SpanList _spanList[NPAGES];	// 按页数映射
};

PageCache pageCache;
