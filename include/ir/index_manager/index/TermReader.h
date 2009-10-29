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

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

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
    TermReaderImpl(FieldInfo* pFieldInfo);

    ~TermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    bool seek(Term* pTerm);

    void close() ;

    void updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset);

    TermInfo* termInfo(Term* term);

public:
    FieldInfo* pFieldInfo_;

    TERM_TABLE* pTermTable_;

    InputDescriptor* pInputDescriptor_;

    int32_t nTermCount_;

    int64_t nVocLength_;
};

class DiskTermReader:public TermReader
{
public:
    DiskTermReader();

    DiskTermReader(TermReaderImpl* pTermReaderImpl);

    virtual ~DiskTermReader(void);
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    TermIterator* termIterator(const char* field);

    bool seek(Term* pTerm);

    TERM_TABLE* getTermTable() { return pTermReaderImpl_->pTermTable_; }

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    freq_t docFreq(Term* term);


    void close() ;

    TermReader* clone() ;

    void updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset);

    TermReaderImpl* getTermReaderImpl(){ return pTermReaderImpl_;}

protected:
    TermInfo* termInfo(Term* term);

protected:
    TermReaderImpl* pTermReaderImpl_;

    TermInfo* pCurTermInfo_;

    bool ownTermReaderImpl_;

    friend class DiskTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

class InMemoryTermReader : public TermReader
{
public:
    InMemoryTermReader(void);

    InMemoryTermReader(const char* field,FieldIndexer* pIndexer);

    virtual ~InMemoryTermReader(void);
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    TermIterator* termIterator(Term* pLowerTerm,Term* pUpperTerm);

    TermIterator* termIterator(const char* field);

    bool seek(Term* term);

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    freq_t docFreq(Term* term);

    void close();

    TermReader* clone() ;

    TermInfo* termInfo(Term* term);

    InMemoryPosting* inMemoryPosting();
protected:
    string field_;

    FieldIndexer* pIndexer_;

    TermInfo* pCurTermInfo_;

    InMemoryPosting* pCurPosting_;

    TermInfo* pTermInfo_;

    friend class InMemoryTermIterator;
};


}

NS_IZENELIB_IR_END

#endif
