#include <ir/index_manager/index/MultiTermIterator.h>

#include <cassert>

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

    if (!pTerm_)
        pTerm_ = top->term_->clone();
    else
        pTerm_->copy(*(top->term_));

    if (!pTermInfo_)
        pTermInfo_ = new TermInfo();
    else
        pTermInfo_->reset();

    while (top != NULL && pTerm_->compare(top->term_) == 0)
    {
        const TermInfo* termInfo = top->termIterator_->termInfo();
        assert(termInfo);

        pTermInfo_->docFreq_ += termInfo->docFreq_;
        pTermInfo_->ctf_ += termInfo->ctf_;
        if(pTermInfo_->lastDocID_ == BAD_DOCID || pTermInfo_->lastDocID_ < termInfo->lastDocID_)
            pTermInfo_->lastDocID_ = termInfo->lastDocID_;

        if (top->next())
            termIteratorsQueue_->adjustTop();
        else
            termIteratorsQueue_->pop();

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
    if(!pTerm_)
        return NULL;

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

