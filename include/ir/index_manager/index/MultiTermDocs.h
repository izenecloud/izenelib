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
    BarrelTermDocsEntry(BarrelInfo* barrelInfo_,TermDocFreqs* termDocs_)
    {
        barrelInfo = new BarrelInfo(*barrelInfo_);
        termDocs = termDocs_;
    }
    ~BarrelTermDocsEntry()
    {
        delete barrelInfo;
        delete termDocs;
    }
protected:
    BarrelTermDocsEntry() {}
public:
    BarrelInfo* barrelInfo;
    TermDocFreqs* termDocs;

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
            return (o1->termDocs->doc()) < (o2->termDocs->doc());
        }
    };
public:
    MultiTermDocs(void);

    virtual ~MultiTermDocs(void);
public:
    docid_t doc();

    count_t freq();

    bool next();

    count_t next(docid_t*& docs, count_t*& freqs);

    freq_t docFreq();

    freq_t docLength();

    int64_t	getCTF();

    void close();

    void add(BarrelInfo* pBarrelInfo,TermDocFreqs* pTermDocs);

protected:

    void initQueue();

protected:

    list<BarrelTermDocsEntry*> barrelTermDocs;

    BarrelTermDocsEntry* current;

    int cursor;

    TermDocsQueue* pTermDocsQueue;
};


}

NS_IZENELIB_IR_END

#endif
