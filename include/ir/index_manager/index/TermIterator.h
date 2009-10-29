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
typedef stx::btree_map<termid_t, TermInfo > TERM_TABLE;

/**
* Iterate terms from index barrel files(*.voc)
*/

class DiskTermIterator : public TermIterator
{
public:
    DiskTermIterator(DiskTermReader* termReader);

    ~DiskTermIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    Posting* termPosting();

    size_t setBuffer(char* pBuffer,size_t bufSize);

protected:
    DiskTermReader* pTermReader_;      ///parent term reader
    Term* pCurTerm_;         ///current term in this iterator
    TermInfo* pCurTermInfo_;      ///current term info in this iterator
    Posting* pCurTermPosting_;   ///current term's posting in this iterator
    InputDescriptor* pInputDescriptor_;
    TERM_TABLE::iterator currTermIter_;
    TERM_TABLE::iterator termIterEnd_;
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

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    Posting* termPosting();

    size_t   setBuffer(char* pBuffer,size_t bufSize);
protected:
    InMemoryTermReader* pTermReader_;
    Term* pCurTerm_;         ///current term in this iterator
    TermInfo* pCurTermInfo_;      ///current term info in this iterator
    InMemoryPosting* pCurTermPosting_;   ///current term's posting in this iterator
    InMemoryPostingMap::iterator postingIterator_;
    InMemoryPostingMap::iterator postingIteratorEnd_;
};

}

NS_IZENELIB_IR_END

#endif

