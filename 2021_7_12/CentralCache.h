#pragma once

#include"Common.h"

class CentralCache
{

public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}
public:
	////算出num后，从中心缓存中获取一定的对象
	////输出型参数，所以必须是引用
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t bytes);
	////从页缓存中获取对象
	Span* GetOneSpan(SpanList& list, size_t size);

	//将多出来的对象还给对应的Span,
	//但是有个问题，1. 一种桶，链接很多Span，怎么找当初分给你的那个
	//              2.假设此时8字节桶,从第一个span拿走4个，从第二个span拿走10个，归还的时候肯定是无序的，怎么确定
	void ReleaseListToSpans(void* start, size_t bytes);
private:

	//和thread_cache一样，按照字节映射，所以也有184个桶
	SpanList _spanlist[FREELISTS];
private:
	CentralCache()
	{}
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator=(const CentralCache&) = delete;
	static CentralCache _sInst;


};
