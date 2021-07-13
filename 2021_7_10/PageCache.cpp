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
		//Span* it = _spanList[k].Begin();
		//_spanList[k].Erase(it);
		//return it;
		return _spanList[k].PopFront();
	}

	//没有的话，找一个比他大的页，把大页切小
	for (size_t i = k+1; i < NPAGES; ++i)
	{
		//找到某个大页，切成k和i-k
		//k是你要的，返回。剩下的找到对应的新页重新挂起，
		if (!_spanList[i].Empty())
		{
			////这是呢个大页
			//Span* span = _spanList[i].Begin();
			////取出来准备切
			//_spanList->Erase(span);
			//切完之后两部分，splitSpan是切成的较大页,span就变成了小页
			//1.头切
			//Span* splitSpan = new Span;

			//splitSpan->pageId = span->pageId + k;
			//splitSpan->n = span->n - k;

			//span->n = k;//他的id不需要变，页数变成k

			////找到属于较大页的页数
			//_spanList[splitSpan->n].Insert(_spanList[splitSpan->n].Begin(),splitSpan);
			//return span;
			////这是呢个大页
			//Span* span = _spanList[i].Begin();
			////取出来准备切
			//_spanList->Erase(span);//把它封装一下

			Span* span = _spanList[i].PopFront();
			//2.尾切
			Span* splitSpan = new Span;
  	//		splitSpan->pageId = span->pageId+span->n - k;
			//splitSpan->n = k;

			span->n -= k;
			for (PageId i = 0; i < k; ++i)
			{
				//改变切出来的k页，他们页号与Span映射关系
				_idSpanMap[splitSpan->pageId+i] = splitSpan;
			}
			_spanList[span->n].PushFront(span);//把大页重新插
			return splitSpan;
		}
		
	}
	//但是一个都没有怎么办,典型的就是第一次进来
	//只能去向系统开口了
	//找系统拿一块128页内存
	void* memory = SystemAllocPage(NPAGES - 1);
	Span* bigSpan = new Span;
	bigSpan->pageId = size_t(memory )>> 12;//起始页号就是他的地址除以4*k
	bigSpan->n = NPAGES-1;//起始开始，页数为128

	//建立映射关系
	for (PageId i = 0; i < bigSpan->n; ++i)
	{
		PageId id = bigSpan->pageId + i;//拿到每个页
		_idSpanMap[id] = bigSpan;//每个页都映射这个大Span
		//不理解可以去看那张图
	}
	_spanList[NPAGES - 1].Insert(_spanList[NPAGES - 1].Begin(),bigSpan);

	//在递归调用一次，就从上面 return span那里return了
	return NewSpan(k);

}
Span* PageCache::MapObjectToSpan(void* obj)
{
	PageId id = (ADDRES_LEN)obj >> PAGE_SHIFT;

	/*std::map<PageId,Span*>::iterator it*/
	auto it=_idSpanMap.find(id);
	if (it != _idSpanMap.end())
	{
		return it->second;
	}
	else
	{
		//出问题了，才会找不到
		assert(false);
		return nullptr;
	}

}
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//检查前后空闲Span，进行合并

	//他应该是不断的去找
	while (1)
	{
    //1.向前合并
	PageId preId = span->pageId - 1;
	auto ret = _idSpanMap.find(preId);

	//找到且前一个页是空闲的
	if (ret != _idSpanMap.end()&&ret->second->_usecount==0)
	{
		    //总得先把prevSpan取出来吧，span是从centralCache还回来的
		    Span* preSpan = ret->second;
			_spanList[preSpan->n].Erase(preSpan);
			
		
			span->pageId = preSpan->pageId;
			span->n += preSpan->n;

			//更新映射关系
			for (PageId i = 0; i < preSpan->n; ++i)
			{
				_idSpanMap[preSpan->pageId + i] = span;
			}

			//之前的span都是new出来的
			delete preSpan;
			//PageCache这是按页数映射的,合大了，还得改桶本身的映射，再插进去
	}
	else//没找到，或前面不为0，退出
	{
		break;
	}

	}
   //向后合并
	while (1)
	{
		PageId nextId = span->pageId + span->n;

		auto ret = _idSpanMap.find(nextId);

		if (ret != _idSpanMap.end() && ret->second->_usecount==0)
		{
			
			Span* nextSpan = ret->second;
			//解开
			_spanList[nextSpan->pageId].Erase(nextSpan);
			
			span->n += nextSpan->n;
			for (int i = 0; i < nextSpan->n; ++i)
			{
				_idSpanMap[nextSpan->pageId + i] = span;
			}
			delete nextSpan;
		}
		else
		{
			break;
		}

	}

	//合并出的span插入进去，他下次不管要大要小就会重新切

	//头插进去
	_spanList[span->n].PushFront(span);


}