#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/TermDocFreqs.h>

using namespace izenelib::ir::indexmanager;

MultiTermDocs::MultiTermDocs(void)
{
    current = NULL;
    pTermDocsQueue = NULL;
}

MultiTermDocs::~MultiTermDocs(void)
{
    close();
}

docid_t MultiTermDocs::doc()
{
    return current->termDocs->doc();
}

count_t MultiTermDocs::freq()
{
    return current->termDocs->freq();
}

bool MultiTermDocs::next()
{
    if (pTermDocsQueue == NULL)
    {
        initQueue();
        if (current)
            return true;
        return false;
    }

    while (pTermDocsQueue->size() > 0)
    {
        if (current->termDocs->next())
            return true;
        else
        {
            pTermDocsQueue->pop();
            if (pTermDocsQueue->size() > 0)
            {
                current = pTermDocsQueue->top();
                return true;
            }
        }
    }
    return false;
}

count_t MultiTermDocs::next(docid_t*& docs, count_t*& freqs)
{
    if (pTermDocsQueue == NULL)
    {
        initQueue();
    }

    int c = -1;
    while (pTermDocsQueue->size() > 0)
    {
        current = pTermDocsQueue->top();
        c = current->termDocs->next(docs,freqs);
        if (c> 0)
            return c;
        else
            pTermDocsQueue->pop();
    }
    return c;
}

freq_t MultiTermDocs::docFreq()
{
    BarrelTermDocsEntry* pEntry;
    freq_t df = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs.begin();
    while (iter != barrelTermDocs.end())
    {
        pEntry = (*iter);
        df += pEntry->termDocs->docFreq();
        iter++;
    }
    return df;
}
int64_t MultiTermDocs::getCTF()
{
    BarrelTermDocsEntry* pEntry;
    int64_t ctf = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs.begin();
    while (iter != barrelTermDocs.end())
    {
        pEntry = (*iter);
        ctf += pEntry->termDocs->getCTF();
        iter++;
    }
    return ctf;
}

void  MultiTermDocs::close()
{
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs.begin();
    while (iter != barrelTermDocs.end())
    {
        delete (*iter);
        iter++;
    }
    barrelTermDocs.clear();
    if (pTermDocsQueue)
    {
        delete pTermDocsQueue;
        pTermDocsQueue = NULL;
    }
    current = NULL;
    cursor = -1;
}

void MultiTermDocs::add(BarrelInfo* pBarrelInfo,TermDocFreqs* pTermDocs)
{
    barrelTermDocs.push_back(new BarrelTermDocsEntry(pBarrelInfo,pTermDocs));
    if (current == NULL)
        current = barrelTermDocs.front();
}
void MultiTermDocs::initQueue()
{
    pTermDocsQueue = new TermDocsQueue(barrelTermDocs.size());
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs.begin();
    BarrelTermDocsEntry* pEntry;
    while (iter != barrelTermDocs.end())
    {
        pEntry = *iter;
        if (pEntry->termDocs->next())
            pTermDocsQueue->insert(pEntry);
        iter++;
    }
    if (pTermDocsQueue->size() > 0)
        current = pTermDocsQueue->top();
    else current = NULL;
}

