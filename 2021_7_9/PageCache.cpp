#include"PageCache.h"

//ֻ����ϵͳ����Ҫ
void* PageCache::SystemAllocPage(size_t k)
{
	//Linux��brk
	//windows��virtualAlloc
	return SystemAlloc(k);
}
Span* PageCache::NewSpan(size_t k)
{

	//��Ӧҳ����ʣ���Span�͸���
	if (!_spanList[k].Empty())
	{
		Span* it = _spanList[k].Begin();
		_spanList[k].Erase(it);
		return it;
	}

	//û�еĻ�����һ���������ҳ���Ѵ�ҳ��С
	for (size_t i = k+1; i < NPAGES; ++i)
	{
		//�ҵ�ĳ����ҳ���г�k��i-k
		//k����Ҫ�ģ����ء�ʣ�µ��ҵ���Ӧ����ҳ���¹���
		if (!_spanList[i].Empty())
		{
			//�����ظ���ҳ
			Span* span = _spanList[i].Begin();
			//ȡ����׼����
			_spanList->Erase(span);
			//����֮�������֣�splitSpan���гɵĽϴ�ҳ,span�ͱ����Сҳ
			Span* splitSpan = new Span;

			splitSpan->pageId = span->pageId + k;
			splitSpan->n = span->n - k;

			span->n = k;//����id����Ҫ�䣬ҳ�����k

			//�ҵ����ڽϴ�ҳ��ҳ��
			_spanList[splitSpan->n].Insert(_spanList[splitSpan->n].Begin(),splitSpan);
			return span;
		}
		
	}
	//����һ����û����ô��,���͵ľ��ǵ�һ�ν���
	//ֻ��ȥ��ϵͳ������
	//��ϵͳ��һ��128ҳ�ڴ�
	void* memory = SystemAllocPage(NPAGES - 1);
	Span* bigSpan = new Span;
	bigSpan->pageId = size_t(memory )>> 12;//ҳ�ž������ĵ�ַ����4*k
	bigSpan->n = NPAGES-1;
	_spanList[NPAGES - 1].Insert(_spanList[NPAGES - 1].Begin(),bigSpan);

	//�ڵݹ����һ�Σ��ʹ����� return span����return��
	return NewSpan(k);

}