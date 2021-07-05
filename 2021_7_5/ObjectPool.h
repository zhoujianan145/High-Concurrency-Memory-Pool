#pragma once

#include"Common.h"

//template<size_t size>
//class ObjectPool
//{
//	//ֻ�������ڴ�
//	//��Ӧmalloc free
//};

//�����

namespace zjn{
	template<class T>
	class ObjectPool
	{
	public:
		~ObjectPool()
		{}
		//��һ������,����ֵ����������
		void*& NextObj(void* obj)
		{
			return *((void**)obj);
		}
		T*  New()
		{
			//��freeList��ȥ��freelist
			T* obj = nullptr;
			if (_freeList)
			{
				
				obj = (T*)_freeList;
				//��ͷ�ĸ��ֽھ�����һ���ĵ�ַ������һ�����µ�ͷ
				/*_freeList = (void*)*((int*)_freeList);*/
				_freeList = NextObj(obj);
			}
			else//�����ȥ������ڴ�
			{
				//����ڴ�Ҳû�У�ȥmalloc
				if (leftSize<sizeof(T))
				{
					//������100k
					leftSize = 1024 * 100;
					_memory = (char*)malloc(leftSize);
					//ʧ�����쳣
					if (_memory == nullptr)
					{
						throw std::bad_alloc();
					}
				}
				//�ߵ����������
				//1._memory����,���������и�
				//2._memory������,malloc��������
				//���ռ�+��ʼ��
				obj = (T*)_memory;
				_memory += sizeof(T);
				leftSize -= sizeof(T);

			}
			new(obj)T;//��λnew
			return obj;
		}
		//αɾ����ʵ���Ƿŵ�freelist��
		void Delete(T* obj)
		{
			obj->~T();
			//ȡ��ǰ�ĸ��ֽ�����ָ����һ��ͷ
			/**((int*)obj) = (int)_freeList;*/
			NextObj(obj) = _freeList;

			//��freeListָ���µ�ͷ
			_freeList = obj;
		}
	private:
		//һ����ڴ�
		//char*,+1����һ���ֽڣ������ƶ���
		char* _memory = nullptr;
		//��������
		//��Ҫ�ڵ㣬�洢ָ��
		void* _freeList = nullptr;
		//ʣ������
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
