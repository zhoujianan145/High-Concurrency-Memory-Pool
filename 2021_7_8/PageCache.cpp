#include "PageCache.h"

// ��ϵͳ����kҳ�ڴ�
void* PageCache::SystemAllocPage(size_t k)
{

}

Span* PageCache::NewSpan(size_t k)
{
	if (!_spanList[k].Empty())
	{
		Span* it = _spanList[k].Begin();
		_spanList[k].Erase(it);
		return it;
	}

	for (size_t i = k+1; i < NPAGES; ++i)
	{
		// ��ҳ����С,�г�kҳ��span����
		// �г�i-kҳ�һ���������
		if (!_spanList[i].Empty())
		{
			Span* span = _spanList[i].Begin();
			_spanList->Erase(span);

			Span* splitSpan = new Span;
			splitSpan->_pageId = span->_pageId + k;
			splitSpan->_n = span->_n - k;

			span->_n = k;

			_spanList[splitSpan->_n].Insert(_spanList[splitSpan->_n].Begin(), splitSpan);

			return span;
		}
	}

	Span* bigSpan = new Span;
	void* memory = SystemAllocPage(NPAGES - 1);
	bigSpan->_pageId = (size_t)memory >> 12;
	bigSpan->_n = NPAGES - 1;

	_spanList[NPAGES - 1].Insert(_spanList[NPAGES - 1].Begin(), bigSpan);

	return NewSpan(k);
}