#pragma once
#include"Common.h"

//Ϊ�˷�ֹ�ڴ���Ƭ
class PageCache
{
public:
	Span* NewSpan(size_t k);
	void* SystemAllocPage(size_t k);
private:
	//��Ȼ���ɹ���Span,�������Ǹ���ҳ��ӳ���
	SpanList _spanList[NPAGES];//����ҳ��ӳ�䣬��1��ʼ��0�������ݡ����128ҳ
};

static PageCache pageCache;