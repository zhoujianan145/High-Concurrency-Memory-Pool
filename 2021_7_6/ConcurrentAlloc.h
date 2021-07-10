#pragma once

#include "Common.h"
#include "ThreadCache.h"

//void* tcmalloc(size_t size)
void* ConcurrentAlloc(size_t size)
{
	if (tls_threadcache == nullptr)
	{
		tls_threadcache = new ThreadCache;
	}

	cout << tls_threadcache << endl;

	return tls_threadcache->Allocate(size);
}

void ConcurrentFree(void* ptr, size_t size)
{
	assert(tls_threadcache);

	tls_threadcache->Deallocate(ptr, size);
}