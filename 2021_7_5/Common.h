#include<iostream>
#include<exception>
#include<thread>
#include<vector>
#include<cassert>
#include<ctime>
using std::cout;
using std::endl;

//意为下一个对象
//返回引用。
inline void*& NextObj(void* obj)
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
static size_t Index(size_t size)
{
	return ((size + 7) >> 3) - 1;
}
//自由链表类型
class FreeList
{
public:
	//头插进一个对象
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
	}
	//头删一个对象
	void* Pop()
	{
		void* obj = _head;
		_head = NextObj(obj);
		return obj;
	}
	bool  Empty()
	{
		if (_head == nullptr)
			return true;
		else
			return false;
	}
private:
	void* _head = nullptr;
};