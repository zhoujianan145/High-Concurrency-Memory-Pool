#include "CentralCache.h"
#include "PageCache.h"

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ����spanlist��ȥ�һ����ڴ��span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_memory)
		{
			return it;
		}

		it = it->_next;
	}

	// �ߵ����������span��û���ڴ��ˣ�ֻ����pagecache
	return pageCache.NewSpan(SizeClass::NumMovePage(size));
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t i = SizeClass::Index(size);
	Span* span = GetOneSpan(_spanLists[i], size);

	// ...
	start = span->_memory;
	for (size_t i = 0; i < n; ++i)
	{

	}


}

