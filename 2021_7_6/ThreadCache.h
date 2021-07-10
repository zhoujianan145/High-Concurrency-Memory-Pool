#pragma once

#include "Common.h"

class ThreadCache
{
public:
	// ������ͷ��ڴ����
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
private:
	FreeList _freeLists[16];
};

static __declspec(thread) ThreadCache* tls_threadcache = nullptr;

// map<int, ThreadCache> idCache;
// TLS  thread local storage