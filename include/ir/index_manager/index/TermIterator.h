/**
* @file        TermIterator.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Term Iterator
*/
#ifndef TERMITERATOR_H
#define TERMITERATOR_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/FieldIndexer.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Posting;
class TermInfo;
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
     * move to the first term which equal to or bigger than target
     * @param target the target term
     * @return true if exist,otherwise false
     */
    virtual bool skipTo(const Term* target) = 0;

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

    /**
     * get doc freq of current term,only valid after calling {@link #next()} or {@link #skipTo()} and returning true
     */
    virtual freq_t docFreq() = 0;
public:
    /**
     * set buffer for iterator
     * @param pBuffer buffer, only keep the pointer, caller response for destroying.
     * @param bufSize size of buffer
     * @return actual size used
     */
    virtual size_t   setBuffer(char* pBuffer,size_t bufSize);
protected:
    char* pBuffer;      ///buffer for iterator
    size_t nBuffSize;   ///size of buffer
};



class InputDescriptor;
///DiskTermIterator
class DiskTermReader;
/**
* Iterate terms from index barrel files(*.voc)
*/
class DiskTermIterator : public TermIterator
{
public:
    DiskTermIterator(DiskTermReader* pTermReader);

    virtual ~DiskTermIterator(void);
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
     * @return term's doc freq
     */
    freq_t docFreq();

    /**
     * set buffer for iterator
     * @param pBuffer buffer, only keep the pointer, caller response for destroying.
     * @param bufSize size of buffer
     * @return actual size used
     */
    size_t   setBuffer(char* pBuffer,size_t bufSize);
private:
    DiskTermReader* pTermReader;      ///parent term reader
    Term* pCurTerm;         ///current term in this iterator
    TermInfo* pCurTermInfo;      ///current term info in this iterator
    Posting* pCurTermPosting;   ///current term's posting in this iterator
    InputDescriptor* pInputDescriptor;
    int32_t nCurPos;         ///current position in this iterator
};

class InMemoryTermReader;
/**
* Iterator terms from memory
*/
class InMemoryTermIterator : public TermIterator
{
public:
    InMemoryTermIterator(InMemoryTermReader* pTermReader);
    virtual ~InMemoryTermIterator();
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
     * @return term's doc freq
     */
    freq_t docFreq();

    /**
     * set buffer for iterator
     * @param pBuffer buffer, only keep the pointer, caller response for destroying.
     * @param bufSize size of buffer
     * @return actual size used
     */
    size_t   setBuffer(char* pBuffer,size_t bufSize);
protected:
    InMemoryTermReader* pTermReader;
    Term* pCurTerm;         ///current term in this iterator
    TermInfo* pCurTermInfo;      ///current term info in this iterator
    InMemoryPosting* pCurTermPosting;   ///current term's posting in this iterator
    InMemoryPostingMap::iterator postingIterator;
    InMemoryPostingMap::iterator postingIteratorEnd;
};

}

NS_IZENELIB_IR_END

#endif

