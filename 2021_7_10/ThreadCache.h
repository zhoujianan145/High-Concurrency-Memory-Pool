#pragma once

#include"Common.h"
class ThreadCache
{
public:
	//申请内存
	void* Allocate(size_t size);
	//假如当前没有，去向中心缓存申请
	void* FetchFromCentralCache(size_t index, size_t size);

	//释放内存
	void Deallocate(void* ptr,size_t size);
	//假如太长，归还给中心缓存
	void ListTooLong(FreeList& list, size_t size);
private:
	//多个哈希映射的自由链表组成，每个自由链表有自己的头结点
	//自由链表数组
	FreeList _freelist[FREELISTS];
};

//TLS Thread Local Storage
//线程本地缓存
static _declspec(thread) ThreadCache* tls_threadcache = nullptr;
