#include "ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	size_t i = Index(size);
	if (!_freeLists[i].Empty())
	{
		//return _freeLists[i].Pop();
	}
	else
	{
		// ...
	}

	return nullptr;
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	//size_t i = Index(size);
	//_freeLists[i].Push(ptr);

	// List Too Long central cache »• Õ∑≈
}