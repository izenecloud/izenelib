/**
* @file        TermReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Multi Term Reader
*/

#ifndef TERMREADER_H
#define TERMREADER_H

#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/TermIterator.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <string>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{
typedef rde::hash_map<termid_t, TermInfo > TERM_TABLE;

/*
struct TERM_TABLE
{
    termid_t tid;
    TermInfo ti;
};
*/
/**
* Internal class of DiskTermReader
* We use this class because there would exist concurrent read, without this class,
* we have to repeat constructing the vocabulary in memory when read the index concurrently
*/

class TermReaderImpl
{
public:
    TermReaderImpl(FieldInfo* pFieldInfo_);

    ~TermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    bool seek(Term* pTerm);

    void close() ;

    void updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset);

    TermInfo* termInfo(Term* term);

public:
    FieldInfo* pFieldInfo;

    TERM_TABLE* pTermTable;

    InputDescriptor* pInputDescriptor;

    int32_t nTermCount;

    int64_t nVocLength;
};


class OrderPreservingTermReaderImpl
{
public:
    OrderPreservingTermReaderImpl(FieldInfo* pFieldInfo_);

    ~OrderPreservingTermReaderImpl();
public:

    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    bool seek(Term* pTerm);

    void close() ;

public:
    FieldInfo* pFieldInfo;

    ORDERED_TERM_TABLE* pTermTable;

    InputDescriptor* pInputDescriptor;

    int32_t nTermCount;

    int64_t nVocLength;
};

class DiskTermReader:public TermReader
{
public:
    enum DiskTermReaderMode { UNORDERED,  ORDERPRESERVING }; 

    DiskTermReader(DiskTermReaderMode mode = UNORDERED);

    DiskTermReader(TermReaderImpl* pTermReaderImpl);

    virtual ~DiskTermReader(void);
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    TermIterator* termIterator(const char* field);

    bool seek(Term* pTerm);

    TERM_TABLE* getTermTable() { return pTermReaderImpl->pTermTable; }

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    freq_t docFreq(Term* term);


    void close() ;

    TermReader* clone() ;

    void updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset);

    OrderPreservingTermReaderImpl* getTermReaderImpl()
    {
        return pOrderedTermReaderImpl;
    }

protected:
    TermInfo* termInfo(Term* term);

protected:
    DiskTermReaderMode termReaderMode;	

    TermReaderImpl* pTermReaderImpl;

    OrderPreservingTermReaderImpl* pOrderedTermReaderImpl;

    TermInfo* pCurTermInfo;

    bool ownTermReaderImpl;

    friend class DiskTermIterator;
    friend class CollectionIndexer;

};

class InMemoryTermReader : public TermReader
{
public:
    InMemoryTermReader(void);

    InMemoryTermReader(const char* field,FieldIndexer* pIndexer);

    virtual ~InMemoryTermReader(void);
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);
    /**
     * get the term iterator
     * @param pLowerTerm lower bound
     * @param pUpperTerm upper bound
     * @return term iterator, MUST be deleted by caller
     */
    TermIterator* termIterator(Term* pLowerTerm,Term* pUpperTerm);

    /**
     * get the term iterator
     * @param field field name
     * @return term iterator, MUST be deleted by caller
     */
    TermIterator* termIterator(const char* field);

    /**
     * seek a term
     * @param pTerm term
     * @return true if success, otherwise false, if the return value is true then {@link termDocFreqs()} and
     * {@link termPositions()}can be called.
     */
    bool seek(Term* term);

    /**
     * get term's document postings,must be called after call seek() success
     * @return return document postings,need to be deleted outside
     */
    TermDocFreqs* termDocFreqs();

    /**
     * get term's position postings,must be called after call seek() success
     * @return return position postings,need to be deleted outside
     */
    TermPositions* termPositions();

    freq_t docFreq(Term* term);

    void close();

    /**
     * clone the term reader
     * @return term reader, MUST be deleted by caller.
     */
    TermReader* clone() ;

public:
    TermInfo* termInfo(Term* term);
    /**
     * get in-memory posting
     * @return reference to in-memory posting
     */
    InMemoryPosting* inMemoryPosting();
protected:
    string sField;

    FieldIndexer* pIndexer;

    TermInfo* pCurTermInfo;

    InMemoryPosting* pCurPosting;

    TermInfo* pTermInfo;

    friend class InMemoryTermIterator;
};


}

NS_IZENELIB_IR_END

#endif
