#include"ThreadCache.h"
#include"CentralCache.h"

//���統ǰû�У�ȥ�����Ļ�������
void* ThreadCache::FetchFromCentralCache(size_t i, size_t size)
{
	//���������ԣ���ʼ������һ������
	//���澭������һ���Ը����һ�㣬����������Ļ��潻��
	size_t batch_num = min(SizeClass::NumMoveSize(size), _freelist[i].MaxSize());

	//����̻߳���ȥ���Ļ������룬������Ļ�����Ҫ��ȫ��Ψһ��.

	//������Ͳ���,��ȡ�ڴ�ĵ�ַ
	void* start = nullptr;
	void* end = nullptr;
	
	//��ʵ����ֵ,���������Ҫ10��������һSpanֻʣ��6��������6��Ӱ���𣬲�Ӱ�죬��ֻ��һ��Ϊ�˲���������Ļ��潻�����Ч�ʵĲ���
	//ʵ����������ֻҪ1����Ӧ�ֽڴ�С�Ķ���,���������ֻ��Ϊ����һ�����������Ļ��潻��
	size_t actual_num=centralCache.FetchRangeObj(start, end, batch_num, size);

	assert(actual_num > 0);

	//���ǵĲ��ԣ�����һ������ʣ�¹��ڶ�Ӧ�����������ϣ��´�����ֱ�ӣ���Alloac��pop�ͺ���
	if (actual_num > 1)
	{
		/*_freelist[i]��дһ��������һ����̫�鷳��*/
		//����һ���������ǵ��׵�ַ����������ͷ4���ֽڣ���ȥ������󣬰�ʣ��Ĺ�������
		_freelist[i].PushRange(NextObj(start), end, actual_num - 1); 
	}

	if (_freelist[i].MaxSize() == batch_num)
	{
		_freelist[i].SetMaxSize(_freelist[i].MaxSize() + 1);
	}

	return start;//
}
//�����ڴ�
void* ThreadCache::Allocate(size_t size)
{
	size_t i = SizeClass::Index(size);
	//��Ӧ��Ͱ�ǿ�,ȡһ�����󵯳�ȥ
	if (!_freelist[i].Empty())
	{
		return _freelist[i].Pop();
	}
	else//û�ռ䣬ȥ��central cacheҪ
	{
		//�������1.���Ļ���Ҳû��2.���Ļ�����(�����߳̿��еĽ϶໹�����Ļ���,��pagecache�зֶ���)
		return FetchFromCentralCache(i, size);
	}
}

//ĳ��Ͱ���������������Ļ���
void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	size_t batchNum = list.MaxSize();
	void* start;
	void* end;

	list.PopRange(start, end, batchNum);
	centralCache.ReleaseListToSpans(start, size);

}

//�ͷ��ڴ�
void ThreadCache::Deallocate(void* ptr,size_t size)
{
	//�����Ӧ��Ͱ,ֱ�ӰѲ��õ������ַ(����)�ŵ�Ͱ��
	size_t i = SizeClass::Index(size);
	_freelist[i].Push(ptr);

	//���ǷŽ�ȥ��������̫������ô��,��ǰͰ���������ǹ黹�����Ļ���
	if (_freelist[i].Size() > _freelist[i].MaxSize())
	{
		ListTooLong(_freelist[i],size);
		
	}
}