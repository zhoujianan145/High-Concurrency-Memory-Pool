#include"CentralCache.h"
#include"PageCache.h"



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
	Span* span = pageCache.NewSpan(SizeClass::NumMovePage(size));

	//�õ����и�ã�����span�е�_list��

	return span;
}

//��ȡһ������
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t i = SizeClass::Index(size);
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


	return j - 1;


}
//��������Ķ��󻹸���Ӧ��Span,
//�����и����⣬1. һ��Ͱ�����Ӻܶ�Span����ô�ҵ����ָ�����Ǹ�
//              2.�����ʱ8�ֽ�Ͱ,�ӵ�һ��span����4�����ӵڶ���span����10�����黹��ʱ��϶�������ģ���ôȷ��
void ReleaseListToSpans(void* start, size_t bytes)
{

}