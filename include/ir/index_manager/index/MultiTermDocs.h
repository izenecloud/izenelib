/**
* @file        MultiTermDocs.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Iterate the index of term document frequency posting (*.dfp) in multi barrels
*/
#ifndef MULTITERMDOCS_H
#define MULTITERMDOCS_H

#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <list>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class BarrelTermDocsEntry
{
public:
    BarrelTermDocsEntry(BarrelInfo* barrelInfo,TermDocFreqs* termDocs)
    {
        barrelInfo_ = barrelInfo;
        termDocs_= termDocs;
    }
    ~BarrelTermDocsEntry()
    {
        delete termDocs_;
    }
protected:
    BarrelTermDocsEntry() {}
public:
    BarrelInfo* barrelInfo_;
    TermDocFreqs* termDocs_;

    friend class MultiTermDocs;
};
/**
* Iterate the index of term document frequency posting (*.dfp) in multi barrels
*/

class MultiTermDocs : public TermDocFreqs
{
    class TermDocsQueue:public PriorityQueue<BarrelTermDocsEntry*>
    {
    public:
        TermDocsQueue(size_t size)
        {
            initialize(size,false);
        }
    protected:
        bool lessThan(BarrelTermDocsEntry* o1, BarrelTermDocsEntry* o2)
        {
            return (o1->termDocs_->doc()) < (o2->termDocs_->doc());
        }
    };
public:
    MultiTermDocs();

    virtual ~MultiTermDocs();
public:
    docid_t doc();

    count_t freq();

    bool next();

    docid_t skipTo(docid_t docId);

    freq_t docFreq();

    int64_t	getCTF();

    int32_t getMaxTF();

    void close();

    void add(BarrelInfo* pBarrelInfo,TermDocFreqs* pTermDocs);

protected:

    void initQueue();

protected:

    std::list<BarrelTermDocsEntry*> barrelTermDocs_;

    BarrelTermDocsEntry* current_;

    int cursor_;

    TermDocsQueue* pTermDocsQueue_;
};

inline docid_t MultiTermDocs::doc()
{
    return current_->termDocs_->doc();
}

inline count_t MultiTermDocs::freq()
{
    return current_->termDocs_->freq();
}

inline bool MultiTermDocs::next()
{
    if (pTermDocsQueue_ == NULL)
    {
        initQueue();
        if (current_)
            return true;
        return false;
    }

    while (pTermDocsQueue_->size() > 0)
    {
        if (current_->termDocs_->next())
            return true;
        else
        {
            pTermDocsQueue_->pop();
            if (pTermDocsQueue_->size() > 0)
            {
                current_ = pTermDocsQueue_->top();
                return true;
            }
        }
    }
    return false;
}

inline docid_t MultiTermDocs::skipTo(docid_t docId)
{
    if(pTermDocsQueue_ == NULL)
    {
        initQueue();
    }

    TermDocFreqs* pTop = NULL;	
    docid_t nFoundId = -1;
    while (pTermDocsQueue_->size() > 0)
    {
        current_ = pTermDocsQueue_->top();
        pTop = current_->termDocs_;
		
        nFoundId = pTop->skipTo(docId);
        if((nFoundId != (docid_t)-1)&&(nFoundId >= docId))
        {
            return nFoundId;
        }
        else 
            pTermDocsQueue_->pop();
    }	
    return -1;
}

inline freq_t MultiTermDocs::docFreq()
{
    BarrelTermDocsEntry* pEntry;
    freq_t df = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        pEntry = (*iter);
        df += pEntry->termDocs_->docFreq();
        iter++;
    }
    return df;
}

inline int64_t MultiTermDocs::getCTF()
{
    BarrelTermDocsEntry* pEntry;
    int64_t ctf = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        pEntry = (*iter);
        ctf += pEntry->termDocs_->getCTF();
        iter++;
    }
    return ctf;
}

inline int32_t MultiTermDocs::getMaxTF()
{
    BarrelTermDocsEntry* pEntry;
    int32_t maxtf = 0;
    list<BarrelTermDocsEntry*>::iterator iter = barrelTermDocs_.begin();
    while (iter != barrelTermDocs_.end())
    {
        pEntry = (*iter);
        int32_t maxtf_in_barrel = pEntry->termDocs_->getMaxTF();
        if(maxtf_in_barrel > maxtf) maxtf = maxtf_in_barrel;
        iter++;
    }
    return maxtf;
}
}

NS_IZENELIB_IR_END

#endif
