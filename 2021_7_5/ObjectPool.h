#pragma once

#include"Common.h"

//template<size_t size>
//class ObjectPool
//{
//	//只管申请内存
//	//对应malloc free
//};

//对象池

namespace zjn{
	template<class T>
	class ObjectPool
	{
	public:
		~ObjectPool()
		{}
		//下一个对象,返回值可以是引用
		void*& NextObj(void* obj)
		{
			return *((void**)obj);
		}
		T*  New()
		{
			//有freeList就去找freelist
			T* obj = nullptr;
			if (_freeList)
			{
				
				obj = (T*)_freeList;
				//他头四个字节就是下一个的地址，让下一个做新的头
				/*_freeList = (void*)*((int*)_freeList);*/
				_freeList = NextObj(obj);
			}
			else//否则就去看大块内存
			{
				//大块内存也没有，去malloc
				if (leftSize<sizeof(T))
				{
					//先申请100k
					leftSize = 1024 * 100;
					_memory = (char*)malloc(leftSize);
					//失败抛异常
					if (_memory == nullptr)
					{
						throw std::bad_alloc();
					}
				}
				//走到这两种情况
				//1._memory存在,从他上面切割
				//2._memory不存在,malloc开出来了
				//开空间+初始化
				obj = (T*)_memory;
				_memory += sizeof(T);
				leftSize -= sizeof(T);

			}
			new(obj)T;//定位new
			return obj;
		}
		//伪删除，实际是放到freelist中
		void Delete(T* obj)
		{
			obj->~T();
			//取出前四个字节让他指向上一个头
			/**((int*)obj) = (int)_freeList;*/
			NextObj(obj) = _freeList;

			//让freeList指向新的头
			_freeList = obj;
		}
	private:
		//一大块内存
		//char*,+1就是一个字节，容易移动。
		char* _memory = nullptr;
		//自由链表
		//不要节点，存储指针
		void* _freeList = nullptr;
		//剩余容量
		size_t leftSize=0;
	};
	struct TreeNode
	{
		int val ;
		TreeNode* left;
		TreeNode* right;
		TreeNode()
			:val(0)
			,left(nullptr)
			,right(nullptr)
			{}
	};
	void TestObjectPool()
	{
		ObjectPool<TreeNode> tnPool;

		for (size_t i = 0; i < 1000; ++i)
		{
			TreeNode* node = tnPool.New();
			cout << node << endl;
		}
		
	}
	void TestObjectPool1()
	{
		ObjectPool<TreeNode> tnPool;
		TreeNode* node1 = tnPool.New();
		TreeNode* node2 = tnPool.New();
		TreeNode* node3 = tnPool.New();
		cout << "node1:" << node1 << endl;
		cout << "node2:" << node2 << endl;
		cout << "node3:" << node3 << endl;
		tnPool.Delete(node2);
		TreeNode* node4 = tnPool.New();
		cout << "node4:" << node4 << endl;
	}
	void TestTime()
	{
		size_t begin1=clock();
		std::vector<TreeNode*> v1;
		for (int i = 0; i < 100000; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < 100000; ++i)
		{
			delete v1[i];
		}
		v1.clear();
		for (int i = 0; i < 100000; ++i)
		{
			v1.push_back(new TreeNode);
		}
		size_t end1 =clock();
		cout << "new and delete:   "<<end1 - begin1 << endl;

		ObjectPool<TreeNode> tnPool;
		size_t begin2 = clock();
		std::vector<TreeNode*> v2;
		for (int i = 0; i < 100000; ++i)
		{
			v2.push_back(tnPool.New());
		}
		for (int i = 0; i < 100000; ++i)
		{
			tnPool.Delete(v2[i]);
		}
		v2.clear();
		for (int i = 0; i < 100000; ++i)
		{
			v2.push_back(new TreeNode);
		}
		size_t end2 = clock();
		cout <<"New() and Delete():   "<<end2 - begin2 << endl;
	}
};
