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

    void  close();

    loc_t nextPosition();

    int32_t nextPositions(loc_t*& positions);

    void add(BarrelInfo* pBarrelInfo,TermPositions* pTermPositions);
private:

    void initQueue();

private:

    std::list<BarrelTermPositionsEntry*> termPositionsList_;

    BarrelTermPositionsEntry* current_;

    TermPositionQueue* pTermPositionQueue_;
};

}

NS_IZENELIB_IR_END

#endif

