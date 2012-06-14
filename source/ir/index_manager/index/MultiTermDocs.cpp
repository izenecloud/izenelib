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

