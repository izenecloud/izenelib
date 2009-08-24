#include <ir/index_manager/index/MultiTermPositions.h>


using namespace izenelib::ir::indexmanager;

MultiTermPositions::MultiTermPositions(void)
{
    current = NULL;
    pTermPositionQueue = NULL;
}

MultiTermPositions::~MultiTermPositions(void)
{
    close();
}

docid_t MultiTermPositions::doc()
{
    return current->termPositions->doc();
}

count_t MultiTermPositions::freq()
{
    return current->termPositions->freq();
}

bool MultiTermPositions::next()
{
    if (pTermPositionQueue == NULL)
    {
        initQueue();
        if (current)
            return true;
        return false;
    }

    while (pTermPositionQueue->size() > 0)
    {
        if (current->termPositions->next())
        {
            return true;
        }
        else
        {
            pTermPositionQueue->pop();
            if (pTermPositionQueue->size() > 0)
            {
                current = pTermPositionQueue->top();
                return true;
            }
        }
    }
    return false;
}

count_t MultiTermPositions::next(docid_t*& docs, count_t*& freqs)
{
    if (pTermPositionQueue == NULL)
    {
        initQueue();
    }

    int c = -1;
    while (pTermPositionQueue->size() > 0)
    {
        current = pTermPositionQueue->top();
        c = current->termPositions->next(docs,freqs);
        if (c> 0)
            return c;
        else
            pTermPositionQueue->pop();
    }
    return c;
}

freq_t MultiTermPositions::docFreq()
{
    BarrelTermPositionsEntry* pEntry;
    freq_t df = 0;
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList.begin();
    while (iter != termPositionsList.end())
    {
        pEntry = (*iter);
        df += pEntry->termPositions->docFreq();
        iter++;
    }
    return df;
}

freq_t MultiTermPositions::docLength()
{
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList.begin();
    return (*iter)->termPositions->docLength();
}

int64_t MultiTermPositions::getCTF()
{
    BarrelTermPositionsEntry* pEntry;
    int64_t ctf = 0;
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList.begin();
    while (iter != termPositionsList.end())
    {
        pEntry = (*iter);
        ctf += pEntry->termPositions->getCTF();
        iter++;
    }
    return ctf;
}

void MultiTermPositions::close()
{
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList.begin();
    while (iter != termPositionsList.end())
    {
        delete (*iter);
        iter++;
    }
    termPositionsList.clear();
    if (pTermPositionQueue)
    {
        delete pTermPositionQueue;
        pTermPositionQueue = NULL;
    }
    current = NULL;
}

loc_t MultiTermPositions::nextPosition()
{
    return current->termPositions->nextPosition();
}

int32_t MultiTermPositions::nextPositions(loc_t*& positions)
{
    return current->termPositions->nextPositions(positions);
}
void MultiTermPositions::add(BarrelInfo* pBarrelInfo,TermPositions* pTermPositions)
{
    termPositionsList.push_back(new BarrelTermPositionsEntry(pBarrelInfo,pTermPositions));
    if (current == NULL)
        current = termPositionsList.front();
}
void MultiTermPositions::initQueue()
{
    pTermPositionQueue = new TermPositionQueue(termPositionsList.size());
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList.begin();
    BarrelTermPositionsEntry* pEntry;
    while (iter != termPositionsList.end())
    {
        pEntry = *iter;
        if (pEntry->termPositions->next())
            pTermPositionQueue->insert(pEntry);
        iter++;
    }
    if (pTermPositionQueue->size() > 0)
        current = pTermPositionQueue->top();
    else current = NULL;
}


