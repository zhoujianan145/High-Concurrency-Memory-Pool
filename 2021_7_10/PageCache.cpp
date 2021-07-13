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
		//Span* it = _spanList[k].Begin();
		//_spanList[k].Erase(it);
		//return it;
		return _spanList[k].PopFront();
	}

	//û�еĻ�����һ���������ҳ���Ѵ�ҳ��С
	for (size_t i = k+1; i < NPAGES; ++i)
	{
		//�ҵ�ĳ����ҳ���г�k��i-k
		//k����Ҫ�ģ����ء�ʣ�µ��ҵ���Ӧ����ҳ���¹���
		if (!_spanList[i].Empty())
		{
			////�����ظ���ҳ
			//Span* span = _spanList[i].Begin();
			////ȡ����׼����
			//_spanList->Erase(span);
			//����֮�������֣�splitSpan���гɵĽϴ�ҳ,span�ͱ����Сҳ
			//1.ͷ��
			//Span* splitSpan = new Span;

			//splitSpan->pageId = span->pageId + k;
			//splitSpan->n = span->n - k;

			//span->n = k;//����id����Ҫ�䣬ҳ�����k

			////�ҵ����ڽϴ�ҳ��ҳ��
			//_spanList[splitSpan->n].Insert(_spanList[splitSpan->n].Begin(),splitSpan);
			//return span;
			////�����ظ���ҳ
			//Span* span = _spanList[i].Begin();
			////ȡ����׼����
			//_spanList->Erase(span);//������װһ��

			Span* span = _spanList[i].PopFront();
			//2.β��
			Span* splitSpan = new Span;
  	//		splitSpan->pageId = span->pageId+span->n - k;
			//splitSpan->n = k;

			span->n -= k;
			for (PageId i = 0; i < k; ++i)
			{
				//�ı��г�����kҳ������ҳ����Spanӳ���ϵ
				_idSpanMap[splitSpan->pageId+i] = splitSpan;
			}
			_spanList[span->n].PushFront(span);//�Ѵ�ҳ���²�
			return splitSpan;
		}
		
	}
	//����һ����û����ô��,���͵ľ��ǵ�һ�ν���
	//ֻ��ȥ��ϵͳ������
	//��ϵͳ��һ��128ҳ�ڴ�
	void* memory = SystemAllocPage(NPAGES - 1);
	Span* bigSpan = new Span;
	bigSpan->pageId = size_t(memory )>> 12;//��ʼҳ�ž������ĵ�ַ����4*k
	bigSpan->n = NPAGES-1;//��ʼ��ʼ��ҳ��Ϊ128

	//����ӳ���ϵ
	for (PageId i = 0; i < bigSpan->n; ++i)
	{
		PageId id = bigSpan->pageId + i;//�õ�ÿ��ҳ
		_idSpanMap[id] = bigSpan;//ÿ��ҳ��ӳ�������Span
		//��������ȥ������ͼ
	}
	_spanList[NPAGES - 1].Insert(_spanList[NPAGES - 1].Begin(),bigSpan);

	//�ڵݹ����һ�Σ��ʹ����� return span����return��
	return NewSpan(k);

}
Span* PageCache::MapObjectToSpan(void* obj)
{
	PageId id = (ADDRES_LEN)obj >> PAGE_SHIFT;

	/*std::map<PageId,Span*>::iterator it*/
	auto it=_idSpanMap.find(id);
	if (it != _idSpanMap.end())
	{
		return it->second;
	}
	else
	{
		//�������ˣ��Ż��Ҳ���
		assert(false);
		return nullptr;
	}

}
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//���ǰ�����Span�����кϲ�

	//��Ӧ���ǲ��ϵ�ȥ��
	while (1)
	{
    //1.��ǰ�ϲ�
	PageId preId = span->pageId - 1;
	auto ret = _idSpanMap.find(preId);

	//�ҵ���ǰһ��ҳ�ǿ��е�
	if (ret != _idSpanMap.end()&&ret->second->_usecount==0)
	{
		    //�ܵ��Ȱ�prevSpanȡ�����ɣ�span�Ǵ�centralCache��������
		    Span* preSpan = ret->second;
			_spanList[preSpan->n].Erase(preSpan);
			
		
			span->pageId = preSpan->pageId;
			span->n += preSpan->n;

			//����ӳ���ϵ
			for (PageId i = 0; i < preSpan->n; ++i)
			{
				_idSpanMap[preSpan->pageId + i] = span;
			}

			//֮ǰ��span����new������
			delete preSpan;
			//PageCache���ǰ�ҳ��ӳ���,�ϴ��ˣ����ø�Ͱ�����ӳ�䣬�ٲ��ȥ
	}
	else//û�ҵ�����ǰ�治Ϊ0���˳�
	{
		break;
	}

	}
   //���ϲ�
	while (1)
	{
		PageId nextId = span->pageId + span->n;

		auto ret = _idSpanMap.find(nextId);

		if (ret != _idSpanMap.end() && ret->second->_usecount==0)
		{
			
			Span* nextSpan = ret->second;
			//�⿪
			_spanList[nextSpan->pageId].Erase(nextSpan);
			
			span->n += nextSpan->n;
			for (int i = 0; i < nextSpan->n; ++i)
			{
				_idSpanMap[nextSpan->pageId + i] = span;
			}
			delete nextSpan;
		}
		else
		{
			break;
		}

	}

	//�ϲ�����span�����ȥ�����´β���Ҫ��ҪС�ͻ�������

	//ͷ���ȥ
	_spanList[span->n].PushFront(span);


}