#include"CentralCache.h"
#include"PageCache.h"


CentralCache CentralCache::_sInst;
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
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));

	//拿到并切割好，挂在span中的_list上

	//根据起始页号算出起始地址
	char* start = (char*)(span->pageId << PAGE_SHIFT);
	//根据起始地址算出终止地址
	char* end = start + (span->n << PAGE_SHIFT);
	
	while (start < end)
	{
		//头插，不用记录尾指针
		char* next = start + size;
		NextObj(start) = span->_list;
		span->_list = start;

		//迭代起来
		start = next;
		//假如剩下的最后一个不够整切了，那么最后一个就丢掉
		//但在我们这里是按8的整数倍映射，一页又是4k,所以一定是能整切的
	}
	//切好了，那就插进去
	list.PushFront(span);
	//同时保存对象大小，free的时候不用传
	span->_objsize = size;
	return span;
}

//获取一批对象
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	//和thread_cache的映射规则一样，只不过这里是Span连接在一起，span里面的list连接着一个个对象
	size_t i = SizeClass::Index(size);
	
	
	/*_spanlist[i].Lock();*/
	//RAII思想
	//舍弃了粗暴的lock，使用lock_guard，参数传入锁,初始化一个对象加锁，出作用域析构自动解锁
	//取同一个桶才加锁
	std::lock_guard<std::mutex> lock(_spanlist[i]._mtx);

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

	//_spanlist[i].UnLock();

	return j - 1;


}
//将多出来的对象还给对应的Span,
//但是有个问题，1. 一种桶，链接很多Span，怎么找当初分给你的那个
//              2.假设此时8字节桶,thread_cache从第一个span拿走4个，从第二个span拿走10个，归还的时候肯定是无序的，怎么确定
void CentralCache::ReleaseListToSpans(void* start, size_t bytes)
{
	size_t i = SizeClass::Index(bytes);
	//释放当然也要锁
	std::lock_guard<std::mutex> lock(_spanlist[i]._mtx);
	while (start)
	{
		void* next = NextObj(start);
		//从thread回来的对象,用你的start,找出是哪个Span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);

		//把对象插入到span的_list中
	    //头插
		NextObj(start) = span->_list;
		span->_list = start;
		span->_usecount--;

		if (span->_usecount == 0)
		{
			//先取出来
			_spanlist[i].Erase(span);
			//取出去了，没了
			span->_list = nullptr;
			////都归回去PageCache
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		}
		start = next;

	}

}