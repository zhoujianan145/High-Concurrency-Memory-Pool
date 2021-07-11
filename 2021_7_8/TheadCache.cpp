#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t i, size_t size)
{
	// 获取一批对象，数量使用慢启动方式
	size_t batchNum = std::min(SizeClass::NumMoveSize(size), _freeLists[i].MaxSize());

	// 去中心缓存获取batch_num个对象
	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = centralCache.FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);

	// >1，返回一个，剩下挂到自由链表
	if (actualNum > 1)
	{
		_freeLists[i].PushRange(NextObj(start), end, actualNum - 1);
	}

	if (_freeLists[i].MaxSize() == batchNum)
	{
		_freeLists[i].SetMaxSize(_freeLists[i].MaxSize() + 1);
	}

	return start;
}

void* ThreadCache::Allocate(size_t size)
{
	size_t i = SizeClass::Index(size);
	if (!_freeLists[i].Empty())
	{
		return _freeLists[i].Pop();
	}
	else
	{
		return FetchFromCentralCache(i, size);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	size_t i = SizeClass::Index(size);
	_freeLists[i].Push(ptr);

	// List Too Long central cache 去释放
}