#include <ir/index_manager/index/MultiTermIterator.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

MultiTermIterator::MultiTermIterator(void)
        :itersQueue(NULL)
        ,pTerm(NULL)
        ,docFreq_(0)
{
}

MultiTermIterator::~MultiTermIterator(void)
{
    vector<TermIteratorEntry*>::iterator iter = iters.begin();
    while (iter != iters.end())
    {
        delete (*iter);
        iter++;
    }
    iters.clear();
    if (pTerm)
    {
        delete pTerm;
        pTerm = NULL;
    }
    if (itersQueue)
    {
        delete itersQueue;
        itersQueue = NULL;
    }
}

bool MultiTermIterator::next()
{
    if (itersQueue == NULL)
    {
        initQueue();
        if (itersQueue == NULL)
        {
            SF1V5_LOG(level::warn) << "No term iterators." << SF1V5_ENDL;
            return false;
        }
    }

    TermIteratorEntry* top = itersQueue->top();
    if (top == NULL)
    {
        if (pTerm)
        {
            delete pTerm;
            pTerm = NULL;
        }

        return false;
    }

    if (pTerm)
    {
        delete pTerm;
        pTerm = NULL;
    }

    pTerm = top->term->clone();
    docFreq_ = 0;

    while (top != NULL && pTerm->compare(top->term) == 0)
    {
        docFreq_ += top->termIterator->docFreq();
        if (top->next())
        {
            itersQueue->adjustTop();
        }
        else itersQueue->pop();
        top = itersQueue->top();
    }
    return true;
}

bool MultiTermIterator::skipTo(const Term* target)
{
    return false;
}
const Term* MultiTermIterator::term()
{
    return pTerm;
}

freq_t MultiTermIterator::docFreq()
{
    return docFreq_;
}
const TermInfo* MultiTermIterator::termInfo()
{
    return NULL;
}
Posting* MultiTermIterator::termPosting()
{
    return NULL;
}

void MultiTermIterator::addIterator(TermIterator* iter)
{
    iters.push_back(new MultiTermIterator::TermIteratorEntry(iter));
}
void MultiTermIterator::initQueue()
{
    if (itersQueue || iters.size()==0)
        return ;
    itersQueue = new TermIteratorQueue(iters.size());
    vector<TermIteratorEntry*>::iterator iter = iters.begin();
    while (iter != iters.end())
    {
        if ((*iter)->next())
            itersQueue->insert(*iter);
        iter++;
    }
}

