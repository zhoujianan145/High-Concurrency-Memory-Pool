#pragma once
#include "Common.h"

class CentralCache
{
public:
	// �����Ļ����ȡһ�������Ķ����thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

	// ��SpanList����page cache��ȡһ��span
	Span* GetOneSpan(SpanList& list, size_t byte_size);
private:
	SpanList _spanLists[NFREELISTS]; // �����뷽ʽӳ��
};

CentralCache centralCache;