#include"ThreadCache.h"
#include"CentralCache.h"

//假如当前没有，去向中心缓存申请
void* ThreadCache::FetchFromCentralCache(size_t i, size_t size)
{
	//慢启动策略，开始最少有一个对象
	//后面经常来，一次性给你多一点，尽量别和中心缓存交互
	size_t batch_num = min(SizeClass::NumMoveSize(size), _freelist[i].MaxSize());

	//多个线程缓存去中心缓存申请，这个中心缓存需要是全局唯一的.

	//做输出型参数,获取内存的地址
	void* start = nullptr;
	void* end = nullptr;
	
	//真实返回值,假如最后你要10个对象，万一Span只剩有6个。返回6个影响吗，不影响，我只是一个为了不让你和中心缓存交互提高效率的策略
	//实际上我真正只要1个对应字节大小的对象,其他多余的只是为了下一次来不与中心缓存交互
	size_t actual_num=centralCache.FetchRangeObj(start, end, batch_num, size);

	assert(actual_num > 0);

	//我们的策略，返回一个对象，剩下挂在对应的自由链表上，下次再来直接，在Alloac中pop就好了
	if (actual_num > 1)
	{
		/*_freelist[i]，写一个函数吧一个个太麻烦了*/
		//返回一个对象，他们的首地址即这个对象的头4个字节，减去这个对象，把剩余的挂起来，
		_freelist[i].PushRange(NextObj(start), end, actual_num - 1); 
	}

	if (_freelist[i].MaxSize() == batch_num)
	{
		_freelist[i].SetMaxSize(_freelist[i].MaxSize() + 1);
	}

	return start;//
}
//申请内存
void* ThreadCache::Allocate(size_t size)
{
	size_t i = SizeClass::Index(size);
	//对应的桶非空,取一个对象弹出去
	if (!_freelist[i].Empty())
	{
		return _freelist[i].Pop();
	}
	else//没空间，去向central cache要
	{
		//两种情况1.中心缓存也没有2.中心缓存有(其他线程空闲的较多还给中心缓存,从pagecache切分而来)
		return FetchFromCentralCache(i, size);
	}
}

//某个桶过长，回收至中心缓存
void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	size_t batchNum = list.MaxSize();
	void* start;
	void* end;

	list.PopRange(start, end, batchNum);
	centralCache.ReleaseListToSpans(start, size);

}

//释放内存
void ThreadCache::Deallocate(void* ptr,size_t size)
{
	//算出对应的桶,直接把不用的这个地址(对象)放到桶里
	size_t i = SizeClass::Index(size);
	_freelist[i].Push(ptr);

	//但是放进去不管了吗，太多了怎么办,当前桶过长把他们归还给中心缓存
	if (_freelist[i].Size() > _freelist[i].MaxSize())
	{
		ListTooLong(_freelist[i],size);
		
	}
}