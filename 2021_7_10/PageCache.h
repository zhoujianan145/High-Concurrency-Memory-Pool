#pragma once
#include"Common.h"

//Ϊ�˷�ֹ�ڴ���Ƭ
class PageCache
{
public:
	Span* NewSpan(size_t k);
	void* SystemAllocPage(size_t k);
	Span* MapObjectToSpan(void* obj);
	void ReleaseSpanToPageCache(Span* span);
private:
	//��Ȼ���ɹ���Span,�������Ǹ���ҳ��ӳ���
	SpanList _spanList[NPAGES];    //����ҳ��ӳ�䣬��1��ʼ��0�������ݡ����128ҳ
	//
	std::map<PageId, Span*> _idSpanMap;
};

static PageCache pageCache;