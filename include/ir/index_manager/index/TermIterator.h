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
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/AbsTermIterator.h>

#include <3rdparty/am/stx/btree_map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class Posting;
class InputDescriptor;
class DiskTermReader;
typedef stx::btree_map<termid_t, TermInfo > ORDERED_TERM_TABLE;

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
    ORDERED_TERM_TABLE::iterator currTermIter;
    ORDERED_TERM_TABLE::iterator termIterEnd;	
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

