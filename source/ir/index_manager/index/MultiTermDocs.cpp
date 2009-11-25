#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/TermDocFreqs.h>

using namespace izenelib::ir::indexmanager;

MultiTermDocs::MultiTermDocs(void)
{
    current_ = NULL;
    pTermDocsQueue_ = NULL;
}

MultiTermDocs::~MultiTermDocs(void)
{
    close();
}

docid_t MultiTermDocs::doc()
{
    return current_->termDocs_->doc();
}

count_t MultiTermDocs::freq()
{
    return current_->termDocs_->freq();
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

count_t MultiTermDocs::next(docid_t*& docs, count_t*& freqs)
{
    if (pTermDocsQueue_ == NULL)
    {
        initQueue();
    }

    int c = -1;
    while (pTermDocsQueue_->size() > 0)
    {
        current_ = pTermDocsQueue_->top();
        c = current_->termDocs_->next(docs,freqs);
        if (c> 0)
            return c;
        else
            pTermDocsQueue_->pop();
    }
    return c;
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
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        delete (*iter);
        iter++;
    }
    barrelTermDocs_.clear();
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
    while (iter != barrelTermDocs_.end())
    {
        pEntry = *iter;
        if (pEntry->termDocs_->next())
            pTermDocsQueue_->insert(pEntry);
        iter++;
    }
    if (pTermDocsQueue_->size() > 0)
        current_ = pTermDocsQueue_->top();
    else current_ = NULL;
}

