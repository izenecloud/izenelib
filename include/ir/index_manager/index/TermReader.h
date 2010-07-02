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

struct TERM_TABLE
{
    termid_t tid;
    TermInfo ti;
};

#define SPARSE_FACTOR 512
#define VOC_ENTRY_LENGTH 52

/**
* Internal class of VocReader
* We use this class because there would exist concurrent read, without this class,
* we have to repeat constructing the vocabulary in memory when read the index concurrently
*/
class TermReaderImpl
{
public:
    TermReaderImpl(FieldInfo* pFieldInfo);

    ~TermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname);

    void reopen();

    void close() ;

    TermInfo* termInfo(Term* term);

public:
    FieldInfo* pFieldInfo_;

    TERM_TABLE* pTermTable_;

    InputDescriptor* pInputDescriptor_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;

    Directory* pDirectory_;
};
/*
* The difference between VocReader and DiskTermReader
* is it will load vocabulary all into memory, it is suitable to do
* so for a small index
*/

class VocReader:public TermReader
{
public:
    VocReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    VocReader(TermReaderImpl* pTermReaderImpl);

    virtual ~VocReader(void);
public:
    void open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    void reopen();

    TermIterator* termIterator(const char* field);

    bool seek(Term* pTerm);

    TERM_TABLE* getTermTable() { return pTermReaderImpl_->pTermTable_; }

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    freq_t docFreq(Term* term);


    void close() ;

    TermReader* clone() ;

    TermReaderImpl* getTermReaderImpl(){ return pTermReaderImpl_;}

private:
    TermInfo* termInfo(Term* term);

private:
    TermReaderImpl* pTermReaderImpl_;

    TermInfo* pCurTermInfo_;

    bool ownTermReaderImpl_;

    InputDescriptor* pInputDescriptor_;

    friend class DiskTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

/**
* Internal class of DiskTermReader
* We use this class because there would exist concurrent read, without this class,
* we have to repeat constructing the vocabulary in memory when read the index concurrently
*/
class SparseTermReaderImpl
{
public:
    SparseTermReaderImpl(FieldInfo* pFieldInfo);

    ~SparseTermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname);

    void reopen();

    void close() ;

public:
    FieldInfo* pFieldInfo_;

    TERM_TABLE* sparseTermTable_;

    InputDescriptor* pInputDescriptor_;

    int32_t nTermCount_;

    int32_t nSparseSize_;

    int64_t nVocLength_;

    int64_t nBeginOfVoc_;

    std::string barrelName_;

    Directory* pDirectory_;
};

/**
* @brief DiskTermReader
*/
class DiskTermReader: public TermReader
{
public:
    DiskTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    DiskTermReader(SparseTermReaderImpl* pTermReaderImpl);

    virtual ~DiskTermReader(void);
public:
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    void reopen();

    TermIterator* termIterator(const char* field);

    bool seek(Term* pTerm);

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    freq_t docFreq(Term* term);

    void close() ;

    TermReader* clone() ;

private:
    TermInfo* termInfo(Term* term);

    TermInfo* searchBuffer(termid_t termId, int end);

    int fillBuffer(int pos);

private:
    SparseTermReaderImpl* pTermReaderImpl_;

    TermInfo* pCurTermInfo_;

    bool ownTermReaderImpl_;

    IndexInput* pVocInput_;

    TERM_TABLE* bufferTermTable_;

    TERM_TABLE* sparseTermTable_;

    int32_t nTermCount_;

    int64_t nBeginOfVoc_;

    friend class DiskTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

/**
* @brief InMemoryTermReader
*/
class InMemoryTermReader : public TermReader
{
public:
    InMemoryTermReader(const char* field,FieldIndexer* pIndexer);

    virtual ~InMemoryTermReader(void);
public:
    void open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    void reopen(){}

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
private:
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
