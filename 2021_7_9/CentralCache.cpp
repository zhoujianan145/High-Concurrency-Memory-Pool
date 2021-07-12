#include"CentralCache.h"
#include"PageCache.h"



Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{

	Span* it = list.Begin();
	//有就return，没有就去问Page要
	while (it != list.End())
	{
		//当这个Span不为空就开始分配了
		if (it->_list)
		{
			return it;
		}
		it = it->_next;
	}

	//代表对应字节大小的Span都没了，只能去找PageCache
	Span* span = pageCache.NewSpan(SizeClass::NumMovePage(size));

	//拿到并切割好，挂在span中的_list上

	return span;
}

//获取一批对象
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t i = SizeClass::Index(size);
	//获取Span
	Span* span=GetOneSpan(_spanlist[i],size);


	//记录起始位置
	start = span->_list;
	//准备去找终止位置
	void* cur = start;
	void* prev = cur;
	//1.足够那我该取多少取多少，2.不够有多少取多少
	size_t j = 1;
	while (j<=n && cur)
	{
		//走之前把他记录下来
		prev = cur;
		//end向后走
		cur= NextObj(cur);
		j++;
		span->_usecount++;
	}

	span->_list = cur;
	end = prev;
	NextObj(prev) = nullptr;


	return j - 1;


}
//将多出来的对象还给对应的Span,
//但是有个问题，1. 一种桶，链接很多Span，怎么找当初分给你的那个
//              2.假设此时8字节桶,从第一个span拿走4个，从第二个span拿走10个，归还的时候肯定是无序的，怎么确定
void ReleaseListToSpans(void* start, size_t bytes)
{

}