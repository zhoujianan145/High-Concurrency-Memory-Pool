#include"CentralCache.h"
#include"PageCache.h"


CentralCache CentralCache::_sInst;
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{

	Span* it = list.Begin();
	//�о�return��û�о�ȥ��PageҪ
	while (it != list.End())
	{
		//�����Span��Ϊ�վͿ�ʼ������
		if (it->_list)
		{
			return it;
		}
		it = it->_next;
	}

	//�����Ӧ�ֽڴ�С��Span��û�ˣ�ֻ��ȥ��PageCache
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));

	//�õ����и�ã�����span�е�_list��

	//������ʼҳ�������ʼ��ַ
	char* start = (char*)(span->pageId << PAGE_SHIFT);
	//������ʼ��ַ�����ֹ��ַ
	char* end = start + (span->n << PAGE_SHIFT);
	
	while (start < end)
	{
		//ͷ�壬���ü�¼βָ��
		char* next = start + size;
		NextObj(start) = span->_list;
		span->_list = start;

		//��������
		start = next;
		//����ʣ�µ����һ�����������ˣ���ô���һ���Ͷ���
		//�������������ǰ�8��������ӳ�䣬һҳ����4k,����һ���������е�
	}
	//�к��ˣ��ǾͲ��ȥ
	list.PushFront(span);
	//ͬʱ��������С��free��ʱ���ô�
	span->_objsize = size;
	return span;
}

//��ȡһ������
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	//��thread_cache��ӳ�����һ����ֻ����������Span������һ��span�����list������һ��������
	size_t i = SizeClass::Index(size);
	
	
	/*_spanlist[i].Lock();*/
	//RAII˼��
	//�����˴ֱ���lock��ʹ��lock_guard������������,��ʼ��һ������������������������Զ�����
	//ȡͬһ��Ͱ�ż���
	std::lock_guard<std::mutex> lock(_spanlist[i]._mtx);

	//��ȡSpan
	Span* span=GetOneSpan(_spanlist[i],size);
	//��¼��ʼλ��
	start = span->_list;
	//׼��ȥ����ֹλ��
	void* cur = start;
	void* prev = cur;
	//1.�㹻���Ҹ�ȡ����ȡ���٣�2.�����ж���ȡ����
	size_t j = 1;
	while (j<=n && cur)
	{
		//��֮ǰ������¼����
		prev = cur;
		//end�����
		cur= NextObj(cur);
		j++;
		span->_usecount++;
	}

	span->_list = cur;
	end = prev;
	NextObj(prev) = nullptr;

	//_spanlist[i].UnLock();

	return j - 1;


}
//��������Ķ��󻹸���Ӧ��Span,
//�����и����⣬1. һ��Ͱ�����Ӻܶ�Span����ô�ҵ����ָ�����Ǹ�
//              2.�����ʱ8�ֽ�Ͱ,thread_cache�ӵ�һ��span����4�����ӵڶ���span����10�����黹��ʱ��϶�������ģ���ôȷ��
void CentralCache::ReleaseListToSpans(void* start, size_t bytes)
{
	size_t i = SizeClass::Index(bytes);
	//�ͷŵ�ȻҲҪ��
	std::lock_guard<std::mutex> lock(_spanlist[i]._mtx);
	while (start)
	{
		void* next = NextObj(start);
		//��thread�����Ķ���,�����start,�ҳ����ĸ�Span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);

		//�Ѷ�����뵽span��_list��
	    //ͷ��
		NextObj(start) = span->_list;
		span->_list = start;
		span->_usecount--;

		if (span->_usecount == 0)
		{
			//��ȡ����
			_spanlist[i].Erase(span);
			//ȡ��ȥ�ˣ�û��
			span->_list = nullptr;
			////�����ȥPageCache
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		}
		start = next;

	}

}