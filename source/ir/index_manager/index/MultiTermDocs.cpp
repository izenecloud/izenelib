#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/TermDocFreqs.h>

using namespace izenelib::ir::indexmanager;

MultiTermDocs::MultiTermDocs()
{
    current_ = NULL;
    pTermDocsQueue_ = NULL;
}

MultiTermDocs::~MultiTermDocs()
{
    close();
}

bool MultiTermDocs::next()
{
    if (pTermDocsQueue_ == NULL)
    {
        initQueue();
        if (current_)
            return true;
        return false;
    }

    while (pTermDocsQueue_->size() > 0)
    {
        if (current_->termDocs_->next())
            return true;
        else
        {
            pTermDocsQueue_->pop();
            if (pTermDocsQueue_->size() > 0)
            {
                current_ = pTermDocsQueue_->top();
                return true;
            }
        }
    }
    return false;
}

docid_t MultiTermDocs::skipTo(docid_t docId)
{
    if(pTermDocsQueue_ == NULL)
    {
        initQueue();
    }

    TermDocFreqs* pTop = NULL;	
    docid_t nFoundId = -1;
    while (pTermDocsQueue_->size() > 0)
    {
        current_ = pTermDocsQueue_->top();
        pTop = current_->termDocs_;
		
        nFoundId = pTop->skipTo(docId);
        if((nFoundId != (docid_t)-1)&&(nFoundId >= docId))
        {
            return nFoundId;
        }
        else 
            pTermDocsQueue_->pop();
    }	
    return -1;
}

freq_t MultiTermDocs::docFreq()
{
    BarrelTermDocsEntry* pEntry;
    freq_t df = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        pEntry = (*iter);
        df += pEntry->termDocs_->docFreq();
        iter++;
    }
    return df;
}

int64_t MultiTermDocs::getCTF()
{
    BarrelTermDocsEntry* pEntry;
    int64_t ctf = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        pEntry = (*iter);
        ctf += pEntry->termDocs_->getCTF();
        iter++;
    }
    return ctf;
}

void  MultiTermDocs::close()
{
    std::list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    for(;iter != barrelTermDocs_.end();++iter)
        delete (*iter);
    if (pTermDocsQueue_)
    {
        delete pTermDocsQueue_;
        pTermDocsQueue_ = NULL;
    }
    current_ = NULL;
    cursor_ = -1;
}

void MultiTermDocs::add(BarrelInfo* pBarrelInfo,TermDocFreqs* pTermDocs)
{
    barrelTermDocs_.push_back(new BarrelTermDocsEntry(pBarrelInfo,pTermDocs));
    if (current_ == NULL)
        current_ = barrelTermDocs_.front();
}
void MultiTermDocs::initQueue()
{
    pTermDocsQueue_ = new TermDocsQueue(barrelTermDocs_.size());
    std::list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    BarrelTermDocsEntry* pEntry;
    for (;iter != barrelTermDocs_.end();++iter)
    {
        pEntry = *iter;
        if (pEntry->termDocs_->next())
            pTermDocsQueue_->insert(pEntry);
    }
    if (pTermDocsQueue_->size() > 0)
        current_ = pTermDocsQueue_->top();
    else current_ = NULL;
}

