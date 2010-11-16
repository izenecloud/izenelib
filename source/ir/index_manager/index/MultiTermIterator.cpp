#include <ir/index_manager/index/MultiTermIterator.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager {

MultiTermIterator::MultiTermIterator(void)
        :termIteratorsQueue_(NULL)
        ,pTerm_(NULL)
        ,docFreq_(0)
        ,pTermInfo_(NULL)
{
}

MultiTermIterator::~MultiTermIterator(void)
{
    std::vector<TermIteratorEntry*>::iterator iter = termIterators_.begin();
    while (iter != termIterators_.end())
    {
        delete (*iter);
        iter++;
    }
    termIterators_.clear();
    if (pTerm_)
    {
        delete pTerm_;
        pTerm_ = NULL;
    }
    if (termIteratorsQueue_)
    {
        delete termIteratorsQueue_;
        termIteratorsQueue_ = NULL;
    }
    if (pTermInfo_)
    {
        delete pTermInfo_;
        pTermInfo_ = NULL;
    }
}

bool MultiTermIterator::next()
{
    if (termIteratorsQueue_ == NULL)
    {
        initQueue();
        if (termIteratorsQueue_ == NULL)
        {
            return false;
        }
    }

    TermIteratorEntry* top = termIteratorsQueue_->top();
    if (top == NULL)
    {
        if (pTerm_)
        {
            delete pTerm_;
            pTerm_ = NULL;
        }

        return false;
    }

    if(pTermInfo_)
        pTermInfo_->reset();

    if (!pTerm_)
        pTerm_ = top->term_->clone();
    else
        pTerm_->copy(*(top->term_));

    std::vector<TermIteratorEntry*>::iterator iter = termIterators_.begin();
    for(;iter != termIterators_.end(); ++iter)
        (*iter)->setCurrent(false);

    while (top != NULL && pTerm_->compare(top->term_) == 0)
    {
        top->setCurrent(true);
        if (top->next())
        {
            termIteratorsQueue_->adjustTop();
        }
        else termIteratorsQueue_->pop();
        top = termIteratorsQueue_->top();
    }

    return true;
}

const Term* MultiTermIterator::term()
{
    return pTerm_;
}

const TermInfo* MultiTermIterator::termInfo()
{
    if(!pTermInfo_)
        pTermInfo_ = new TermInfo();
    if(pTermInfo_->docFreq() == 0)
    {
        fileoffset_t nOffset = -1;
        count_t nDF = 0;
        MultiTermIterator::TermIteratorEntry* pEntry;
        for(std::vector<MultiTermIterator::TermIteratorEntry*>::iterator iter = termIterators_.begin();
            iter != termIterators_.end(); ++iter)
        {
            pEntry = (*iter);
            if(pEntry->isCurrent())
            {
                const TermInfo* termInfo = pEntry->termIterator_->termInfo();
                if(nOffset == -1)
                    nOffset = const_cast<TermInfo*>(termInfo)->docPointer();
                nDF += termInfo->docFreq();
                pTermInfo_->set(*termInfo);
            }					
        }
    }
    return pTermInfo_;
}

PostingReader* MultiTermIterator::termPosting()
{
    ///TODO  not used at current
    return NULL;
}

void MultiTermIterator::addIterator(TermIterator* iter)
{
    termIterators_.push_back(new MultiTermIterator::TermIteratorEntry(iter));
}
void MultiTermIterator::initQueue()
{
    if (termIteratorsQueue_ || termIterators_.size()==0)
        return ;
    termIteratorsQueue_ = new TermIteratorQueue(termIterators_.size());
    for(std::vector<TermIteratorEntry*>::iterator iter = termIterators_.begin(); iter != termIterators_.end(); ++iter)
        if ((*iter)->next())
            termIteratorsQueue_->insert(*iter);
}


}

NS_IZENELIB_IR_END

