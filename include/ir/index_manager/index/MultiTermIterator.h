/**
* @file        MultiTermIterator.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Multi Term Iterator
*/
#ifndef MULTITERMITERATOR_H
#define MULTITERMITERATOR_H

#include <ir/index_manager/index/AbsTermIterator.h>
#include <ir/index_manager/utility/PriorityQueue.h>

#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* Iterate terms when multi index barrels exist
*/
class MultiTermIterator : public TermIterator
{
    class TermIteratorEntry
    {
    public:
        TermIteratorEntry(TermIterator* iter):termIterator_(iter),term_(NULL) {}
        ~TermIteratorEntry()
        {
            if (termIterator_)
            {
                delete termIterator_;
                termIterator_ = NULL;
            }
            term_ = NULL;
        }

        bool next()
        {
            if (termIterator_ == NULL)
                return false;
            bool ret = termIterator_->next();
            if (ret == false)
                return false;
            term_ = (Term*)termIterator_->term();
            return true;
        }

    public:
        TermIterator* termIterator_;
        Term* term_;
    };

    class TermIteratorQueue : public PriorityQueue<MultiTermIterator::TermIteratorEntry*>
    {
    public:
        TermIteratorQueue(size_t size)
        {
            initialize(size,false);
        }
    protected:
        bool lessThan(MultiTermIterator::TermIteratorEntry* o1, MultiTermIterator::TermIteratorEntry* o2)
        {
            return (o1->term_->compare(o2->term_) < 0);
        }
    };

public:
    MultiTermIterator(void);

    virtual ~MultiTermIterator(void);
public:
    bool next();

    const Term* term();

    /**
     * get term information.
     * @return term information
     * @note as there are multiple barrels, in return value,
     * only @c docFreq_, @c ctf_ and @c lastDocID_ is valid.
     */
    const TermInfo* termInfo();

    PostingReader* termPosting();

    void addIterator(TermIterator* iter);

protected:
    void initQueue();
private:
    std::vector<MultiTermIterator::TermIteratorEntry*>	termIterators_;
    MultiTermIterator::TermIteratorQueue* termIteratorsQueue_;
    Term* pTerm_;
    freq_t docFreq_;
    TermInfo* pTermInfo_;
};


}

NS_IZENELIB_IR_END

#endif
