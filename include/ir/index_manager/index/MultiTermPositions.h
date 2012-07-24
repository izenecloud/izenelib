/**
* @file        MultiTermPositions.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Iterate the index of position posting (*.pop) in multi barrels
*/
#ifndef MULTITERMPOSITIONS_H
#define MULTITERMPOSITIONS_H

#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <list>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BarrelTermPositionsEntry
{
public:
    BarrelTermPositionsEntry(BarrelInfo* barrelInfo,TermPositions* termPositions)
        :barrelInfo_(barrelInfo)
        ,termPositions_(termPositions)
    {
    }
    ~BarrelTermPositionsEntry()
    {
        delete termPositions_;
    }
protected:
    BarrelTermPositionsEntry() {}
public:
    BarrelInfo* barrelInfo_;
    TermPositions* termPositions_;

    friend class MultiTermPositions;
};

/**
* Iterate the index of position posting (*.pop) in multi barrels
*/

class MultiTermPositions : public TermPositions
{
    class TermPositionQueue:public PriorityQueue<BarrelTermPositionsEntry*>
    {
    public:
        TermPositionQueue(size_t size)
        {
            initialize(size,false);
        }
    protected:
        bool lessThan(BarrelTermPositionsEntry* o1, BarrelTermPositionsEntry* o2)
        {
            return (o1->termPositions_->doc()) < (o2->termPositions_->doc());
        }
    };
public:
    MultiTermPositions();

    ~MultiTermPositions();
public:
    docid_t doc();

    count_t freq();

    bool next();

    docid_t skipTo(docid_t docId);

    freq_t docFreq();

    int64_t getCTF();

    int32_t getMaxTF();

    void  close();

    loc_t nextPosition();

    void add(BarrelInfo* pBarrelInfo,TermPositions* pTermPositions);
private:

    void initQueue();

private:

    std::list<BarrelTermPositionsEntry*> termPositionsList_;

    BarrelTermPositionsEntry* current_;

    TermPositionQueue* pTermPositionQueue_;
};

inline docid_t MultiTermPositions::doc()
{
    return current_->termPositions_->doc();
}

inline count_t MultiTermPositions::freq()
{
    return current_->termPositions_->freq();
}

inline bool MultiTermPositions::next()
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

inline docid_t MultiTermPositions::skipTo(docid_t target)
{			
    if(pTermPositionQueue_ == NULL)
    {
        initQueue();
    }

    TermPositions* pTop = NULL;
    docid_t nFoundId = -1;
    while (pTermPositionQueue_->size() > 0)
    {
        current_ = pTermPositionQueue_->top();
        pTop = current_->termPositions_;

        nFoundId = pTop->skipTo(target);
        if((nFoundId != (docid_t)-1)&&(nFoundId >= target))
        {
            return nFoundId;
        }
        else 
        {
            pTermPositionQueue_->pop();
        }		
    }
	
    return -1;
}

inline freq_t MultiTermPositions::docFreq()
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

inline int64_t MultiTermPositions::getCTF()
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

inline int32_t MultiTermPositions::getMaxTF()
{
    BarrelTermPositionsEntry* pEntry;
    int32_t maxtf = 0;
    list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    while (iter != termPositionsList_.end())
    {
        pEntry = (*iter);
        int32_t maxtf_in_barrel = pEntry->termPositions_->getMaxTF();
        if(maxtf_in_barrel > maxtf) maxtf = maxtf_in_barrel;
        iter++;
    }
    return maxtf;
}

inline void MultiTermPositions::close()
{
    std::list<BarrelTermPositionsEntry*>::iterator iter = termPositionsList_.begin();
    for (;iter != termPositionsList_.end();++iter)
        delete (*iter);
    if (pTermPositionQueue_)
    {
        delete pTermPositionQueue_;
        pTermPositionQueue_ = NULL;
    }
    current_ = NULL;
}

inline loc_t MultiTermPositions::nextPosition()
{
    return current_->termPositions_->nextPosition();
}

}

NS_IZENELIB_IR_END

#endif

