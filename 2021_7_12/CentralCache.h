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
	////���num�󣬴����Ļ����л�ȡһ���Ķ���
	////����Ͳ��������Ա���������
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t bytes);
	////��ҳ�����л�ȡ����
	Span* GetOneSpan(SpanList& list, size_t size);

	//��������Ķ��󻹸���Ӧ��Span,
	//�����и����⣬1. һ��Ͱ�����Ӻܶ�Span����ô�ҵ����ָ�����Ǹ�
	//              2.�����ʱ8�ֽ�Ͱ,�ӵ�һ��span����4�����ӵڶ���span����10�����黹��ʱ��϶�������ģ���ôȷ��
	void ReleaseListToSpans(void* start, size_t bytes);
private:

	//��thread_cacheһ���������ֽ�ӳ�䣬����Ҳ��184��Ͱ
	SpanList _spanlist[FREELISTS];
private:
	CentralCache()
	{}
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator=(const CentralCache&) = delete;
	static CentralCache _sInst;


};
