#include<iostream>
#include<exception>
#include<thread>
#include<vector>
#include<cassert>
#include<ctime>
using std::cout;
using std::endl;

//��Ϊ��һ������
//�������á�
inline void*& NextObj(void* obj)
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
static size_t Index(size_t size)
{
	return ((size + 7) >> 3) - 1;
}
//������������
class FreeList
{
public:
	//ͷ���һ������
	void Push(void* obj)
	{
		NextObj(obj) = _head;
		_head = obj;
	}
	//ͷɾһ������
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