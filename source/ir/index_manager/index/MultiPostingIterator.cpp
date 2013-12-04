#include <ir/index_manager/index/MultiPostingIterator.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

MultiPostingIterator::MultiPostingIterator(size_t nPosition)
    :nPos_(nPosition)
    ,currEntry_(NULL)
    ,positionsQueue_(NULL)
{
}

MultiPostingIterator::~MultiPostingIterator()
{
    for(std::vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin();
                iter != positions_.end(); ++iter)
        delete (*iter);

    if(positionsQueue_)
        delete positionsQueue_;
}

void MultiPostingIterator::addTermPosition(TermPositions* pPosition, Bitset* pDocFilter)
{
    MultiPostingIterator::TermPositionEntry* pEntry =
                new MultiPostingIterator::TermPositionEntry(pPosition, pDocFilter);
    positions_.push_back(pEntry);
}

bool MultiPostingIterator::skipDocs(TermPositionEntry* pEntry)
{
    while(pEntry->pDocFilter_->test(pEntry->pPositions_->doc()))
        if(!pEntry->next())
            return false;
    return true;
}

void MultiPostingIterator::initQueue()
{
    if (positionsQueue_ || positions_.size()==0)
        return;
    positionsQueue_ = new MultiPostingIterator::TermPositionQueue(nPos_);
    for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin();
                iter != positions_.end(); ++iter)
    {
        MultiPostingIterator::TermPositionEntry* pEntry = *iter;
        if (pEntry->next())
        {
            if(pEntry->pDocFilter_&&(!skipDocs(pEntry)))
                continue;
            positionsQueue_->insert(*iter);
        }
    }
}

bool MultiPostingIterator::next()
{
    if (positionsQueue_ == NULL)
    {
        initQueue();
        if (positionsQueue_ == NULL)
        {
            return false;
        }
        TermPositionEntry* top = positionsQueue_->top();
        if (top == NULL)
        {
            return false;
        }

        currDoc_ = top->pPositions_->doc();
        currEntry_ = top;

        for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin();
                iter != positions_.end(); ++iter)
            if(currDoc_ == (*iter)->pPositions_->doc())
                (*iter)->setCurrent(true);
            else
                (*iter)->setCurrent(false);

        return true;
    }

    MultiPostingIterator::TermPositionEntry* top = positionsQueue_->top();

    while(top != NULL && top->isCurrent())
    {
        top->setCurrent(false);
        if (top->next())
            if(top->pDocFilter_)
		if(skipDocs(top))
                  positionsQueue_->adjustTop();
		else
                  positionsQueue_->pop();
            else
                positionsQueue_->adjustTop();
        else
            positionsQueue_->pop();

        top = positionsQueue_->top();
    }

    if (top == NULL)
    {
        return false;
    }

    currDoc_ = top->pPositions_->doc();
    currEntry_ = top;

    for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin();
                iter != positions_.end(); ++iter)
        if((*iter)->pPositions_->doc() == currDoc_)
            (*iter)->setCurrent(true);

    return true;
}

}

NS_IZENELIB_IR_END
