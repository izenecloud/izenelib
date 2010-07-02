/**
* @file        AbsTermReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Abstract base class for TermReader
*/
#ifndef ABS_TERM_READER_H
#define ABS_TERM_READER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class TermIterator;

/*
* @brief TermReader is used to read vocabulary of index barrel.
* When open index of a cerntain barrel, TermReader is firstly acquired
* because all other search utilities will utilize the information got from vocabulary
* According to the form of indices, we have InMemoryTermReader, which
* indicates its cooresponding indices lie in memory; DiskTermReader, which
* indicates the cooresponding indices lie on disk; and MultiTermReader, which 
* is used if there exists multiple index barrels
* For the sake of efficiency, we do not need to read vocabulary whenever 
* there's a query request, so we use TermReaderImpl to read the vocabulary
* physically, and all other instances of TermReader will reuse TermReaderImpl
*/
class TermReader
{
public:
    TermReader();

    TermReader(FieldInfo* pFieldInfo);

    virtual ~TermReader(void);
public:
    ///Open the vocabulary file on disk
    virtual void open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    virtual void reopen() = 0;
    ///TermIterator instance must be got through TermReader
    virtual TermIterator* termIterator(const char* field) = 0;
    ///Seek whether the query term exist within the vocabulary or not
    virtual bool seek(Term* pTerm) = 0;
    ///Get the TermDocFreqs instance for query
    virtual TermDocFreqs* termDocFreqs() = 0;
    ///Get the TermPositions instance for query
    virtual TermPositions* termPositions() = 0;
    ///Get DF for a certain term. DF exists on vocabulary, so it could be returned directly
    virtual freq_t docFreq(Term* term) = 0;

    virtual void close() = 0;

    virtual TermReader*	clone() = 0;
    ///Return all the information of a certain term on the vocabulary
    virtual TermInfo* termInfo(Term* term) { return NULL;};

    FieldInfo* getFieldInfo() { return pFieldInfo_;}

    void setFieldInfo(FieldInfo* pFieldInfo) { pFieldInfo_ = pFieldInfo; }

    virtual void setDocFilter(BitVector* pFilter) { pDocFilter_ = pFilter;}

    BitVector* getDocFilter() { return pDocFilter_; }

    int getSkipInterval() { return skipInterval_; }

    void setSkipInterval(int skipInterval) { skipInterval_ = skipInterval; }

    int getMaxSkipLevel() { return maxSkipLevel_; }

    void setMaxSkipLevel(int maxSkipLevel) { maxSkipLevel_ = maxSkipLevel; }

    void setBarrelInfo(BarrelInfo* pBarrelInfo) { pBarrelInfo_ = pBarrelInfo; }
protected:
    FieldInfo* pFieldInfo_;	///reference to field info
    BitVector* pDocFilter_;
    int skipInterval_;
    int maxSkipLevel_;
    BarrelInfo* pBarrelInfo_;

    friend class TermDocFreqs;
    friend class MultiFieldTermReader;
    friend class IndexReader;
    friend class InMemoryTermReader;
};



}

NS_IZENELIB_IR_END

#endif
