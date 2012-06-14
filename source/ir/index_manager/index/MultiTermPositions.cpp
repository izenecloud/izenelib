#include <ir/index_manager/index/MultiTermPositions.h>


using namespace izenelib::ir::indexmanager;

MultiTermPositions::MultiTermPositions()
{
    current_ = NULL;
    pTermPositionQueue_ = NULL;
}

MultiTermPositions::~MultiTermPositions()
{
    close();
}

void MultiTermPositions::add(BarrelInfo* pBarrelInfo,TermPositions* pTermPositions)
{
    termPositionsList_.push_back(new BarrelTermPositionsEntry(pBarrelInfo,pTermPositions));
    if (current_ == NULL)
        current_ = termPositionsList_.front();
}

void MultiTermPositions::initQueue()
{
    pTermPositionQueue_ = new TermPositionQueue(termPositionsList_.size());
    std::list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    BarrelTermPositionsEntry* pEntry;
    for(;iter != termPositionsList_.end();++iter)
    {
        pEntry = *iter;
        if (pEntry->termPositions_->next())
            pTermPositionQueue_->insert(pEntry);
    }
    if (pTermPositionQueue_->size() > 0)
        current_ = pTermPositionQueue_->top();
    else current_ = NULL;
}


