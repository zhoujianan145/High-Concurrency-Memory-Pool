#pragma once
#include"Common.h"

//为了防止内存碎片
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}
public:
	//PageCache取出内存
	Span* NewSpan(size_t k);
	//向系统申请K页内存，挂在页数映射的自由链表
	void* SystemAllocPage(size_t k);
	//对象从Span的映射
	Span* MapObjectToSpan(void* obj);
	//释放空闲Span到PageCache，合并相邻Span
	void ReleaseSpanToPageCache(Span* span);
private:
	PageCache()
	{}
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;
	static PageCache _sInst;
private:
	//虽然依旧挂着Span,但是它是根据页数映射的
	SpanList _spanList[NPAGES];    //按照页数映射，从1开始，0不放数据。最多128页
	//
	std::unordered_map<PageId, Span*> _idSpanMap;

	std::recursive_mutex _mtx;

};

