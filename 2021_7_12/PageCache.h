#pragma once
#include"Common.h"

//Ϊ�˷�ֹ�ڴ���Ƭ
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}
public:
	//PageCacheȡ���ڴ�
	Span* NewSpan(size_t k);
	//��ϵͳ����Kҳ�ڴ棬����ҳ��ӳ�����������
	void* SystemAllocPage(size_t k);
	//�����Span��ӳ��
	Span* MapObjectToSpan(void* obj);
	//�ͷſ���Span��PageCache���ϲ�����Span
	void ReleaseSpanToPageCache(Span* span);
private:
	PageCache()
	{}
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;
	static PageCache _sInst;
private:
	//��Ȼ���ɹ���Span,�������Ǹ���ҳ��ӳ���
	SpanList _spanList[NPAGES];    //����ҳ��ӳ�䣬��1��ʼ��0�������ݡ����128ҳ
	//
	std::unordered_map<PageId, Span*> _idSpanMap;

	std::recursive_mutex _mtx;

};

