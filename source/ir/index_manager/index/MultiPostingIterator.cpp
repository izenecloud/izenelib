#include <ir/index_manager/index/MultiPostingIterator.h>

using namespace izenelib::ir::indexmanager;
using namespace std;

MultiPostingIterator::MultiPostingIterator(size_t nPosition)
    :nPos_(nPosition)
    ,currEntry_(NULL)
    ,positionsQueue_(NULL)
{
}

MultiPostingIterator::~MultiPostingIterator()
{
    for(std::vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
        delete (*iter);

    if(positionsQueue_)
        delete positionsQueue_;
}

void MultiPostingIterator::addTermPosition(TermPositions* pPosition, BitVector* pDocFilter)
{
    MultiPostingIterator::TermPositionEntry* pEntry = new MultiPostingIterator::TermPositionEntry(pPosition, pDocFilter);
    positions_.push_back(pEntry);
}

void MultiPostingIterator::initQueue()
{
    if (positionsQueue_ || positions_.size()==0)
        return;
    positionsQueue_ = new MultiPostingIterator::TermPositionQueue(nPos_);
    for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        if ((*iter)->next())
            positionsQueue_->insert(*iter);
    }
}

bool MultiPostingIterator::next()
{
    bool ret = next_();
    if(ret)
    {
        while((currEntry_->pDocFilter_)&&(currEntry_->pDocFilter_->test(currDoc_)))
        {
            ///skip positions
            loc_t pos = currEntry_->pPositions_->nextPosition();
            while (pos != BAD_POSITION)
            {
                pos = currEntry_->pPositions_->nextPosition();
            }
            if(!next_())
                break;
        }
    }
    return ret;
}

bool MultiPostingIterator::next_()
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
        for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
            (*iter)->setCurrent(false);

        for(size_t i = 0; i < positionsQueue_->size(); ++i)
        {
            MultiPostingIterator::TermPositionEntry* pEntry = positionsQueue_->getAt(i);
            if(currDoc_ == pEntry->pPositions_->doc())
                pEntry->setCurrent(true);
        }
        return true;
    }

    MultiPostingIterator::TermPositionEntry* top = positionsQueue_->top();

    while(top != NULL && top->isCurrent())
    {
        top->setCurrent(false);
        if (top->next())
            positionsQueue_->adjustTop();
        else positionsQueue_->pop();
        top = positionsQueue_->top();
    }

    if (top == NULL)
    {
        return false;
    }

    currDoc_ = top->pPositions_->doc();
    currEntry_ = top;

    for(vector<MultiPostingIterator::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
        if((*iter)->pPositions_->doc() == currDoc_)
            (*iter)->setCurrent(true);
    return true;
}


