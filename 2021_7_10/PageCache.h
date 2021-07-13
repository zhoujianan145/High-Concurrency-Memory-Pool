#pragma once
#include"Common.h"

//为了防止内存碎片
class PageCache
{
public:
	Span* NewSpan(size_t k);
	void* SystemAllocPage(size_t k);
	Span* MapObjectToSpan(void* obj);
	void ReleaseSpanToPageCache(Span* span);
private:
	//虽然依旧挂着Span,但是它是根据页数映射的
	SpanList _spanList[NPAGES];    //按照页数映射，从1开始，0不放数据。最多128页
	//
	std::map<PageId, Span*> _idSpanMap;
};

static PageCache pageCache;