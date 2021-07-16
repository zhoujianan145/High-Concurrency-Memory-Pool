#pragma once

#include<iostream>
#include<exception>
#include<mutex>
#include<thread>
#include<vector>
#include<unordered_map>
#include<cassert>
#include<ctime>
#include<algorithm>
#ifdef _WIN32
#include<windows.h>
#else
 
#endif
using std::cout;
using std::endl;

//���64k
static const size_t MAXBYTES = 64 * 1024;
//������֮���������Ͱ��184��
static const size_t FREELISTS = 184;
static const size_t NPAGES = 129;
static const size_t PAGE_SHIFT = 12;
#ifdef _WIN32
typedef int ADDRES_LEN;
#else
typedef int ADDRES_LEN;
#endif

#ifdef _WIN32
typedef size_t PageId;
#else
typedef unsigned longlong PageId;
#endif

//��Ϊ��һ������
//�������á�
inline  void*& NextObj(void* obj)
{
	return *((void**)obj);
}
//size_t Index(size_t size)
//{
//	//8��������
//	if (size % 8 == 0)
//	{
//		return size / 8 - 1;
//	}
//	else
//	{
//		return size / 8;
//	}
//}

//���������µ�Ͱ
//Ͱ�Ƿ�����ӳ�䣬���Լ������Ǹ�Ͱ����ʹ�ù̶����㷨
class SizeClass
{
public:
	//��������,central_cache�зָ�thread_cacheʱ��ͬ����Ҫ���룬����5�ֽڰ�8�ֽ���
	static inline size_t _RoundUp(size_t bytes, size_t align)
	{
		    //��7�ֽ�ʱ,7+8-1=14   14&~(7)
		    //0000 1110 
//0000 0111-> 1111 1000
		    //�ͻ��ǰ��λ������λȫ����0���ʹﵽ�����ǵ�Ŀ��0000 1000=8
		return (((bytes)+align - 1) & ~(align - 1));
	}
	// �����С���㣬�˷Ѵ����1%-12%����
	static inline size_t RoundUp(size_t bytes)
	{
		
		if (bytes <= 128){
			return _RoundUp(bytes, 8);
		}
		else if (bytes <= 1024){
			return _RoundUp(bytes, 16);
		}
		else if (bytes <= 8*1024){
			return _RoundUp(bytes, 128);
		}
		else if (bytes <= 64*1024){
			return _RoundUp(bytes, 1024);
		}
		else//����64k����4k����һҳ���롣����˵64k��64+4=668k����
		{
			return _RoundUp(bytes, 1 << PAGE_SHIFT);
		}
		return - 1;
	}



	//��������thread_Cache�и���freeList��ӳ�䣬central_cache�и���Spanlist��ӳ��
	static size_t _Index(size_t bytes,size_t align_shift)
	{    
		//[1,128]����,8�ֽڶ��룬�������1�ֽ�,��С8�ֽ�
		        //((1    +  ��2^3��-1)/8 )   - 1
		        //���������Ͱ�����λ��
		//[129,1024]����,16�ֽڶ��룬�������129�ֽڣ�129�ȼ�ȥǰ���128������С144�ֽ�,����ʱӦ�ü�ȥǰ128���ֽڰ���16�ֽڵĶ��뷽ʽ����
		        //(1   +   (2^4)-1)/16)     -1
		        //���������Ͱ�����λ��
		return ((bytes + (1<<align_shift)-1)>>align_shift) - 1;
	}
	    // ������ƽ��1%-12%���ҵ�����Ƭ�˷�
		// [1,128] 8byte���� freelist[0,15]
		// [129,1024] 16byte���� freelist[16,71]
		// [1025,8*1024] 128byte���� freelist[72,127]
		// [8*1024+1,64*1024] 1024byte���� freelist[128,183]
	
	//����������Ǹ�Ͱ
	static size_t Index(size_t bytes)
	{
		assert(bytes <= MAXBYTES);

		//ÿ������Ͱ������
		static int group[4] = { 16, 56, 56, 56 };

		//8�ֽڶ���
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}//16�ֽڶ���
		else if (bytes <= 1024)
		{
			        //���ڷ��ص��Ǳ�����Ͱ�����λ�ã����Լ����ϸ����������Ͱ����������ľ���λ��
			       
			return _Index(bytes - 128, 4)+group[0];
		}//128�ֽڶ���
		else if (bytes <= (8 * 1024))
		{
			return _Index(bytes - 1024, 7) + group[0] + group[1];
		}//1024�ֽڶ���
		else if (bytes <= (64 * 1024))
		{
			return _Index(bytes - (8 * 1024), 10) + group[0] + group[1] + group[2];
		}
		assert(false);
		return -1;
	}
	//һ�δ����Ļ����ȡ����
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		//������ʵ����
		int num = MAXBYTES / size;

	//��������ֽ�С���Ǿ͸����һ��,num����Ϊ512
	//��������ֽڶ࣬�Ǿ͸�����һ�㣬num����Ϊ2
		if (num < 2)
			num = 2;
		if (num>512)
			num = 512;

		return num;
	}
	//������ϵͳ������ٸ�ҳ
	//�������� 8�ֽڣ�16�ֽ�
	//��Ҳ�п�����64k������SpanӦ���ö��ٸ�ҳ��
	static size_t NumMovePage(size_t size)
	{
		//���ݵ���������ֽڣ���������ٶ���
		size_t num = NumMoveSize(size);
		//�������ж���������ֽ�
		size_t nPage = num*size;

		//2^12��4k�������Ҫ����ҳ
		nPage >>= 12;
		if (nPage == 0)
			nPage = 1;

		return nPage;
	}
};
//������������
class FreeList
{
public:
	//ͷ���һ������
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
		_size++;
	}
	//ͷɾһ������
	void* Pop()
	{
		void* obj = _head;
		_head = NextObj(obj);
		_size--;
		return obj;
		
	}
	//��������Ĺ������������Ӧ��Ͱ�¡�
	void PushRange(void* start,void* end,int n)
	{
		//ͷ��
		//����һ��������ģ�β���ӵ�ԭ����ͷ
		//ͷ���start
		NextObj(end) = _head;
		_head = start;
		_size += n;
	}
	//��Ͱ�϶����ժ��ȥ
	void PopRange(void*& start, void*& end, int n)
	{
		start = _head;
		for (int i = 0; i < n;++i)
		{
			end = _head;
			_head = NextObj(_head);
		}
		NextObj(end) = nullptr;
		_size -= n;
	}
	bool  Empty()
	{
		if (_head == nullptr)
			return true;
		else
			return false;
	}
	size_t Size()
	{
		return _size;
	}
	size_t MaxSize()
	{
		return max_size;
	}
	void SetMaxSize(size_t n)
	{
		max_size = n;
	}
private:
	void* _head = nullptr;
	size_t _size = 0;
	size_t max_size=1;
};

////central��page��Ҫ��
//Span��������һ������ڴ�,

struct Span
{
	PageId pageId=0;//һ��Span����ʼҳ��
	size_t n=0;//�����ҳ�ſ�ʼҳ������,���Ǽ�ҳ

	Span* _next=nullptr;//������һ��Span
	Span* _prev=nullptr;//������һ��Span
	void* _list=nullptr;//����ڴ�����С֮��ģ�������������
	size_t _usecount=0;//ʹ�ü���
	size_t _objsize=0;//���������С 

};
//ʵ�ֳɴ�ͷ˫��ѭ��
class SpanList
{
private:
	Span* _head ;

public:
	std::mutex _mtx;
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;

	}
	Span* Begin()
	{
		return _head->_next;
	}
	Span* End()
	{
		//��ʼ����_head->prev���ⶼ��Ƴɵ������ĸ�ʽ�˾��ÿ������
		return _head;
	}
	Span* PopFront()
	{
		Span* span = Begin();
		Erase(span);

		//������ӹ�ϵ
		return span;
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);;
	}
	void Insert(Span* curSpan, Span* newSpan)
	{ 
		Span* prev = curSpan->_prev;

		prev->_next = newSpan;
		newSpan->_prev = prev;

		newSpan->_next = curSpan;
		curSpan->_prev = newSpan;
	}
	void Erase(Span* curSpan)
	{
		Span* next = curSpan->_next;
		Span* prev = curSpan->_prev;

		prev->_next = next;
		next->_prev = prev;
	}
	bool Empty()
	{
		return _head->_next == _head;
	}
	//void Lock()
	//{
	//	_mtx.lock();
	//}
	//void UnLock()
	//{
	//	_mtx.unlock();
	//}
};
inline static void* SystemAlloc(size_t Kpage)
{
	//Linux��brk
	//windows��virtualAlloc
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, Kpage*(1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}
inline static void SystemFree(void* ptr)
{
	//Linux��brk
	//windows��virtualAlloc
#ifdef _WIN32
	VirtualFree(ptr,0,MEM_RELEASE);
#else
#endif
}