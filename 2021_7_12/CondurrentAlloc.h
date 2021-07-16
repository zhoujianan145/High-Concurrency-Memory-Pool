#include"ThreadCache.h"
#include"PageCache.h"

static void* ConcurrentAlloc(size_t size)
{
	try
	{
		//�����64k��
		if (size > MAXBYTES)
		{
			//PageCache
			//�������ҳ
			size_t npage = SizeClass::RoundUp(size) >> PAGE_SHIFT;
			//ȥ��PageCacheҪ
			Span* span = PageCache::GetInstance()->NewSpan(npage);
			//ͬʱ����Ҫ�������Ĵ�С������
			span->_objsize = size;
			//����pageid,����4k�����ַ
			void* ptr = (void*)(span->pageId << PAGE_SHIFT);
			//����
			//�Ǳ�128*4k�����أ�û��newSpan�з�����������128ҳֱ����ϵͳ����
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

	//���õڶ���������ʱ��,free���������ַ��,���ݵ�ַӳ���Span
	Span* span = PageCache::GetInstance()->MapObjectToSpan(_ptr);
	//�ҵ�����ʱ��Ķ����С
	size_t size = span->_objsize;
	//����64k��û���߳�
	/*assert(tls_threadcache);*/
	if (size > MAXBYTES)
	{	
		//����PageCache
		//����64k���������Ļ��滹��һ���ģ�ֻ����ǰ���������ã����ߵ�����С�������������һ���ٻ�
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