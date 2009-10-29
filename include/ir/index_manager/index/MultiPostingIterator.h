#ifndef MULTI_POSTINGITERATOR_H
#define MULTI_POSTINGITERATOR_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/index/TermPositions.h>

#include <vector>

NS_IZENELIB_IR_BEGIN
namespace indexmanager{

///used for posting merger only.
///when merging two postings that does not satisfy the doc id sequence order, such as
///   1 2 5 10...100  and 3 4 7 8
///notice: among postings, there will not exist duplicated docs
class MultiPostingIterator
{
    class TermPositionEntry
    {
    public:
    TermPositionEntry(TermPositions* pPositions, BitVector* pDocFiler = NULL)
        :pPositions_(pPositions)
        ,pDocFilter_(pDocFiler)
    {}

    ~TermPositionEntry()
    {
        if(pPositions_)
            delete pPositions_;
        pDocFilter_ = NULL;
    }

    bool next()
    {
        if(pPositions_ == NULL)
            return false;
        return pPositions_->next();
    }

    void setCurrent(bool current){current_ = current;}

    bool isCurrent(){return current_;}

    public:
        TermPositions* pPositions_;
        BitVector* pDocFilter_;
        bool current_;
    };

    class TermPositionQueue : public PriorityQueue<MultiPostingIterator::TermPositionEntry*>
    {
    public:
        TermPositionQueue(size_t size)
        {
            initialize(size,false);
        }
    protected:
        bool lessThan(MultiPostingIterator::TermPositionEntry* o1, MultiPostingIterator::TermPositionEntry* o2)
        {
            return (o1->pPositions_->doc() < o2->pPositions_->doc());
        }
    };

public:
    MultiPostingIterator(size_t nPositions);

    ~MultiPostingIterator();
public:
    void addTermPosition(TermPositions* pPosition, BitVector* pDocFilter = NULL);
	
    bool next();

    docid_t doc() { return currDoc_; }

    loc_t nextPosition() { return currEntry_->pPositions_->nextPosition(); }

private:
    void initQueue();

    bool next_();
private:
    size_t nPos_;
	
    docid_t currDoc_;

    MultiPostingIterator::TermPositionEntry* currEntry_;

    MultiPostingIterator::TermPositionQueue* positionsQueue_;

    std::vector<MultiPostingIterator::TermPositionEntry*> positions_;
};

}

NS_IZENELIB_IR_END

#endif
