#pragma once

#include"Common.h"
class ThreadCache
{
public:
	//�����ڴ�
	void* Allocate(size_t size);
	//���統ǰû�У�ȥ�����Ļ�������
	void* FetchFromCentralCache(size_t index, size_t size);

	//�ͷ��ڴ�
	void Deallocate(void* ptr,size_t size);
	//����̫�����黹�����Ļ���
	void ListTooLong(FreeList& list, size_t size);
private:
	//�����ϣӳ�������������ɣ�ÿ�������������Լ���ͷ���
	//������������
	FreeList _freelist[FREELISTS];
};

//TLS Thread Local Storage
//�̱߳��ػ���
static _declspec(thread) ThreadCache* tls_threadcache = nullptr;
