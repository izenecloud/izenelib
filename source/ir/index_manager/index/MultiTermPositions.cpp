#include <ir/index_manager/index/MultiTermPositions.h>


using namespace izenelib::ir::indexmanager;

MultiTermPositions::MultiTermPositions(void)
{
    current_ = NULL;
    pTermPositionQueue_ = NULL;
}

MultiTermPositions::~MultiTermPositions(void)
{
    close();
}

docid_t MultiTermPositions::doc()
{
    return current_->termPositions_->doc();
}

count_t MultiTermPositions::freq()
{
    return current_->termPositions_->freq();
}

bool MultiTermPositions::next()
{
    if (pTermPositionQueue_ == NULL)
    {
        initQueue();
        if (current_)
            return true;
        return false;
    }

    while (pTermPositionQueue_->size() > 0)
    {
        if (current_->termPositions_->next())
        {
            return true;
        }
        else
        {
            pTermPositionQueue_->pop();
            if (pTermPositionQueue_->size() > 0)
            {
                current_ = pTermPositionQueue_->top();
                return true;
            }
        }
    }
    return false;
}

docid_t MultiTermPositions::skipTo(docid_t target)
{			
    if(pTermPositionQueue_ == NULL)
    {
        initQueue();
    }

    TermPositions* pTop = NULL;
    docid_t nBaseId;
    docid_t t;
    docid_t nFoundId = -1;
    while (pTermPositionQueue_->size() > 0)
    {
        current_ = pTermPositionQueue_->top();
        nBaseId = current_->barrelInfo_->getBaseDocID();
        t = target - nBaseId;
        pTop = current_->termPositions_;

        nFoundId = pTop->skipTo(t);
        if(nFoundId >= t)
        {
            nFoundId += nBaseId;
            return nFoundId;
        }
        else 
        {
            pTermPositionQueue_->pop();
        }		
    }
    return -1;
}

freq_t MultiTermPositions::docFreq()
{
    BarrelTermPositionsEntry* pEntry;
    freq_t df = 0;
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    while (iter != termPositionsList_.end())
    {
        pEntry = (*iter);
        df += pEntry->termPositions_->docFreq();
        iter++;
    }
    return df;
}

int64_t MultiTermPositions::getCTF()
{
    BarrelTermPositionsEntry* pEntry;
    int64_t ctf = 0;
    std::list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    while (iter != termPositionsList_.end())
    {
        pEntry = (*iter);
        ctf += pEntry->termPositions_->getCTF();
        iter++;
    }
    return ctf;
}

void MultiTermPositions::close()
{
    std::list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    while (iter != termPositionsList_.end())
    {
        delete (*iter);
        iter++;
    }
    termPositionsList_.clear();
    if (pTermPositionQueue_)
    {
        delete pTermPositionQueue_;
        pTermPositionQueue_ = NULL;
    }
    current_ = NULL;
}

loc_t MultiTermPositions::nextPosition()
{
    return current_->termPositions_->nextPosition();
}

int32_t MultiTermPositions::nextPositions(loc_t*& positions)
{
    return current_->termPositions_->nextPositions(positions);
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
    while (iter != termPositionsList_.end())
    {
        pEntry = *iter;
        if (pEntry->termPositions_->next())
            pTermPositionQueue_->insert(pEntry);
        iter++;
    }
    if (pTermPositionQueue_->size() > 0)
        current_ = pTermPositionQueue_->top();
    else current_ = NULL;
}


