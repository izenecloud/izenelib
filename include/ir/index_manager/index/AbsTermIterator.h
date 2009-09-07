#ifndef ABS_TERM_ITERATOR_H
#define ABS_TERM_ITERATOR_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/TermInfo.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Posting;
/**
* TermIterator is used to iterate terms, if necessary, it could provide the posting relevant to the term iterated.
* It is the base class of InMemoryTermIterator and DiskTermIterator.
* currently SkipList has not been added, it could be an improvement in future.
* After the IndexManager API has been modified, the Indexer does not need such an utility, however, The existence of
* TermIterator should be reasonable, and perhaps will be needed in future, therefore it is reserved, although currently
* it is only used during index merging process. TermIterator only iterator terms in a single barrel, if multi barrels exist,
* MultiTermIterator should be used.
*/
class TermIterator
{
public:
    TermIterator(void);

    virtual ~TermIterator(void);
public:
    /**
     * move to next term
     * @return false if to the end,otherwise true
     */
    virtual bool next() = 0;

    /**
     * get current term ,only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term,internal object
     */
    virtual const Term* term() = 0;

    /**
     * get current term info,only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term's info,internal object
     */
    virtual const TermInfo* termInfo() = 0;

    /**
     * get current term's posting(in-memory or on-disk posting),only valid after calling {@link #next()} or {@link #skipTo()} and returning true.
     * @return term's posting,internal object
     */
    virtual Posting* termPosting() = 0;

    virtual size_t   setBuffer(char* pBuffer,size_t bufSize);

protected:

    char* pBuffer;      ///buffer for iterator

    size_t nBuffSize;   ///size of buffer
};


}

NS_IZENELIB_IR_END

#endif

