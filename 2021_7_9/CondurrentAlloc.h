#include"ThreadCache.h"

void* ConcurrentAlloc(size_t size)
{
	
	if (size > MAXBYTES)
	{
		//PageCache

	}
	else
	{
		
		if (tls_threadcache == nullptr)
		{
			tls_threadcache = new ThreadCache;
		}
	}
	   
	return tls_threadcache->Allocate(size);
}
void ConcurrentDealloc(void* _ptr, size_t size)
{
	assert(tls_threadcache);

	tls_threadcache->Deallocate(_ptr, size);
}