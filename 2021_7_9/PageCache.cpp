#include"PageCache.h"

//只能向系统开口要
void* PageCache::SystemAllocPage(size_t k)
{
	//Linux叫brk
	//windows叫virtualAlloc
	return SystemAlloc(k);
}
Span* PageCache::NewSpan(size_t k)
{

	//对应页，有剩余的Span就给他
	if (!_spanList[k].Empty())
	{
		Span* it = _spanList[k].Begin();
		_spanList[k].Erase(it);
		return it;
	}

	//没有的话，找一个比他大的页，把大页切小
	for (size_t i = k+1; i < NPAGES; ++i)
	{
		//找到某个大页，切成k和i-k
		//k是你要的，返回。剩下的找到对应的新页重新挂起，
		if (!_spanList[i].Empty())
		{
			//这是呢个大页
			Span* span = _spanList[i].Begin();
			//取出来准备切
			_spanList->Erase(span);
			//切完之后两部分，splitSpan是切成的较大页,span就变成了小页
			Span* splitSpan = new Span;

			splitSpan->pageId = span->pageId + k;
			splitSpan->n = span->n - k;

			span->n = k;//他的id不需要变，页数变成k

			//找到属于较大页的页数
			_spanList[splitSpan->n].Insert(_spanList[splitSpan->n].Begin(),splitSpan);
			return span;
		}
		
	}
	//但是一个都没有怎么办,典型的就是第一次进来
	//只能去向系统开口了
	//找系统拿一块128页内存
	void* memory = SystemAllocPage(NPAGES - 1);
	Span* bigSpan = new Span;
	bigSpan->pageId = size_t(memory )>> 12;//页号就是他的地址除以4*k
	bigSpan->n = NPAGES-1;
	_spanList[NPAGES - 1].Insert(_spanList[NPAGES - 1].Begin(),bigSpan);

	//在递归调用一次，就从上面 return span那里return了
	return NewSpan(k);

}