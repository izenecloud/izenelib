/**
* @file        MultiTermIterator.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Multi Term Iterator
*/
#ifndef MULTITERMITERATOR_H
#define MULTITERMITERATOR_H

#include <ir/index_manager/index/TermIterator.h>
#include <ir/index_manager/utility/Logger.h>
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
        TermIteratorEntry(TermIterator* iter):termIterator(iter),term(NULL) {}
        ~TermIteratorEntry()
        {
            if (termIterator)
            {
                delete termIterator;
                termIterator = NULL;
            }
            term = NULL;
        }

        bool next()
        {
            if (termIterator == NULL)
                return false;
            bool ret = termIterator->next();
            if (ret == false)
                return false;
            term = (Term*)termIterator->term();
            return true;
        }
    public:
        TermIterator* termIterator;
        Term* term;
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
            return (o1->term->compare(o2->term) < 0);
        }
    };
public:
    MultiTermIterator(void);
    virtual ~MultiTermIterator(void);
public:
    /**
     * move to next term
     * @return false if to the end,otherwise true
     */
    bool next();

    /**
     * move to the first term which equal to or bigger than target
     * @param target the target term
     * @return true if exist,otherwise false
     */
    bool skipTo(const Term* target);

    /**
     * get current term ,only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term,internal object
     */
    const Term* term();

    /**
     * get current term info,only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term's info,internal object
     */
    const TermInfo* termInfo();

    /**
     * get current term's posting(in-memory or on-disk posting),only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term's posting,internal object
     */
    Posting* termPosting();

    /**
     * get doc freq of current term,only valid after calling {@link #next()} or {@link #skipTo()} and returning true
     */
    freq_t docFreq();

    /**
     * add term iterator
     * @param iter term iterator,internal object
     */
    void addIterator(TermIterator* iter);
protected:
    void initQueue();
private:
    std::vector<MultiTermIterator::TermIteratorEntry*>	iters;
    MultiTermIterator::TermIteratorQueue* itersQueue;
    Term* pTerm;
    freq_t docFreq_;
};


}

NS_IZENELIB_IR_END

#endif
