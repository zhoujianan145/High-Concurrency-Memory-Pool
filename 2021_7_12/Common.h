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

//最大64k
static const size_t MAXBYTES = 64 * 1024;
//分区间之后，算出来的桶有184个
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

//意为下一个对象
//返回引用。
inline  void*& NextObj(void* obj)
{
	return *((void**)obj);
}
//size_t Index(size_t size)
//{
//	//8的整数倍
//	if (size % 8 == 0)
//	{
//		return size / 8 - 1;
//	}
//	else
//	{
//		return size / 8;
//	}
//}

//自由链表下的桶
//桶是分区间映射，所以计算在那个桶不能使用固定的算法
class SizeClass
{
public:
	//辅助计算,central_cache切分给thread_cache时，同样需要补齐，比如5字节按8字节切
	static inline size_t _RoundUp(size_t bytes, size_t align)
	{
		    //切7字节时,7+8-1=14   14&~(7)
		    //0000 1110 
//0000 0111-> 1111 1000
		    //就会把前四位，后三位全部置0，就达到了我们的目的0000 1000=8
		return (((bytes)+align - 1) & ~(align - 1));
	}
	// 对齐大小计算，浪费大概在1%-12%左右
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
		else//大于64k，按4k，即一页对齐。比如说64k按64+4=668k对齐
		{
			return _RoundUp(bytes, 1 << PAGE_SHIFT);
		}
		return - 1;
	}



	//辅助计算thread_Cache中各个freeList的映射，central_cache中各个Spanlist的映射
	static size_t _Index(size_t bytes,size_t align_shift)
	{    
		//[1,128]区间,8字节对齐，例如分配1字节,最小8字节
		        //((1    +  （2^3）-1)/8 )   - 1
		        //算出本区间桶的相对位置
		//[129,1024]区间,16字节对齐，例如分配129字节（129先减去前面的128），最小144字节,但此时应该减去前128个字节按照16字节的对齐方式计算
		        //(1   +   (2^4)-1)/16)     -1
		        //算出本区间桶的相对位置
		return ((bytes + (1<<align_shift)-1)>>align_shift) - 1;
	}
	    // 控制在平均1%-12%左右的内碎片浪费
		// [1,128] 8byte对齐 freelist[0,15]
		// [129,1024] 16byte对齐 freelist[16,71]
		// [1025,8*1024] 128byte对齐 freelist[72,127]
		// [8*1024+1,64*1024] 1024byte对齐 freelist[128,183]
	
	//算出他们在那个桶
	static size_t Index(size_t bytes)
	{
		assert(bytes <= MAXBYTES);

		//每个区间桶的数量
		static int group[4] = { 16, 56, 56, 56 };

		//8字节对齐
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}//16字节对齐
		else if (bytes <= 1024)
		{
			        //由于返回的是本区间桶的相对位置，所以加上上个区间的所有桶，就是整体的绝对位置
			       
			return _Index(bytes - 128, 4)+group[0];
		}//128字节对齐
		else if (bytes <= (8 * 1024))
		{
			return _Index(bytes - 1024, 7) + group[0] + group[1];
		}//1024字节对齐
		else if (bytes <= (64 * 1024))
		{
			return _Index(bytes - (8 * 1024), 10) + group[0] + group[1] + group[2];
		}
		assert(false);
		return -1;
	}
	//一次从中心缓存获取多少
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		//这样就实现了
		int num = MAXBYTES / size;

	//当申请的字节小，那就给你多一点,num设置为512
	//当申请的字节多，那就给你少一点，num设置为2
		if (num < 2)
			num = 2;
		if (num>512)
			num = 512;

		return num;
	}
	//计算向系统申请多少个页
	//单个对象 8字节，16字节
	//但也有可能是64k，所以Span应该拿多少个页呢
	static size_t NumMovePage(size_t size)
	{
		//根据单个对象的字节，算出给多少对象
		size_t num = NumMoveSize(size);
		//根据所有对象算出总字节
		size_t nPage = num*size;

		//2^12即4k，算出需要多少页
		nPage >>= 12;
		if (nPage == 0)
			nPage = 1;

		return nPage;
	}
};
//自由链表类型
class FreeList
{
public:
	//头插进一个对象
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
		_size++;
	}
	//头删一个对象
	void* Pop()
	{
		void* obj = _head;
		_head = NextObj(obj);
		_size--;
		return obj;
		
	}
	//将多申请的挂在自由链表对应的桶下。
	void PushRange(void* start,void* end,int n)
	{
		//头插
		//将那一串多申请的，尾连接到原来的头
		//头变成start
		NextObj(end) = _head;
		_head = start;
		_size += n;
	}
	//将桶上多余的摘出去
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

////central和page都要用
//Span用来管理一个大块内存,

struct Span
{
	PageId pageId=0;//一个Span的起始页号
	size_t n=0;//从这个页号开始页的数量,他是几页

	Span* _next=nullptr;//链接下一个Span
	Span* _prev=nullptr;//链接上一个Span
	void* _list=nullptr;//大块内存是切小之后的，并且连接起来
	size_t _usecount=0;//使用计数
	size_t _objsize=0;//单个对象大小 

};
//实现成带头双向循环
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
		//开始想着_head->prev，这都设计成迭代器的格式了就用开区间把
		return _head;
	}
	Span* PopFront()
	{
		Span* span = Begin();
		Erase(span);

		//解除连接关系
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
	//Linux叫brk
	//windows叫virtualAlloc
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
	//Linux叫brk
	//windows叫virtualAlloc
#ifdef _WIN32
	VirtualFree(ptr,0,MEM_RELEASE);
#else
#endif
}