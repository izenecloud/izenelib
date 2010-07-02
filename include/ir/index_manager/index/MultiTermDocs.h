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


}

NS_IZENELIB_IR_END

#endif
