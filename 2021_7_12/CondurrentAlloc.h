#include"ThreadCache.h"
#include"PageCache.h"

static void* ConcurrentAlloc(size_t size)
{
	try
	{
		//如果比64k大
		if (size > MAXBYTES)
		{
			//PageCache
			//算出多少页
			size_t npage = SizeClass::RoundUp(size) >> PAGE_SHIFT;
			//去向PageCache要
			Span* span = PageCache::GetInstance()->NewSpan(npage);
			//同时把他要申请对象的大小存起来
			span->_objsize = size;
			//根据pageid,乘以4k算出地址
			void* ptr = (void*)(span->pageId << PAGE_SHIFT);
			//返回
			//那比128*4k还大呢，没事newSpan有分流操作大于128页直接向系统申请
			return ptr;

		}
		else
		{

			if (tls_threadcache == nullptr)
			{
				tls_threadcache = new ThreadCache;
			}
			return tls_threadcache->Allocate(size);
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
	return nullptr;
}
static void ConcurrentFree(void* _ptr)
{
	try
	{

	//不用第二个参数的时候,free起码给个地址吧,根据地址映射出Span
	Span* span = PageCache::GetInstance()->MapObjectToSpan(_ptr);
	//找到申请时存的对象大小
	size_t size = span->_objsize;
	//大于64k都没开线程
	/*assert(tls_threadcache);*/
	if (size > MAXBYTES)
	{	
		//还给PageCache
		//大于64k还，和中心缓存还是一样的，只不过前者整体再用，后者等所有小对象回来和整体一样再还
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		
	}
	else
	{
		assert(tls_threadcache);
		tls_threadcache->Deallocate(_ptr, size);
	}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}
	
}