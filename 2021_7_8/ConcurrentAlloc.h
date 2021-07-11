#pragma once

#include "Common.h"
#include "ThreadCache.h"

//void* tcmalloc(size_t size)
void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES)
	{
		// PageCache
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

void ConcurrentFree(void* ptr, size_t size)
{
	assert(tls_threadcache);

	if (size > MAX_BYTES)
	{
		// PageCache
	}
	else
	{
		tls_threadcache->Deallocate(ptr, size);
	}
}