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
#include <ir/index_manager/index/FieldInfo.h>

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

struct TERM_TABLE
{
    termid_t tid;
    TermInfo ti;
};

#define SPARSE_FACTOR 512
//#define VOC_ENTRY_LENGTH 52

/**
* Internal class of VocReader
* We use this class because there would exist concurrent read, without this class,
* we have to repeat constructing the vocabulary in memory when read the index concurrently
*/
class TermReaderImpl
{
public:
    TermReaderImpl(const FieldInfo& fieldInfo, IndexLevel indexLevel);

    ~TermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname);

    void reopen();

    void close() ;

    TermInfo* termInfo(Term* term);

public:
    FieldInfo fieldInfo_;

    TERM_TABLE* pTermTable_;

    InputDescriptor* pInputDescriptor_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;

    Directory* pDirectory_;

    IndexLevel indexLevel_;
};
/*
* The difference between VocReader and RTDiskTermReader
* is it will load vocabulary all into memory, it is suitable to do
* so for a small index
*/

class VocReader:public TermReader
{
public:
    VocReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    VocReader(const boost::shared_ptr<TermReaderImpl>& pTermReaderImpl);

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

    TermReaderImpl* getTermReaderImpl(){ return pTermReaderImpl_.get();}

    FieldInfo* getFieldInfo() { return &pTermReaderImpl_->fieldInfo_; }

private:
    TermInfo* termInfo(Term* term);

private:
    boost::shared_ptr<TermReaderImpl> pTermReaderImpl_;

    TermInfo* pCurTermInfo_;

    InputDescriptor* pInputDescriptor_;

    friend class RTDiskTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

/**
* Internal class of RTDiskTermReader
* We use this class because there would exist concurrent read, without this class,
* we have to repeat constructing the vocabulary in memory when read the index concurrently
*/
class SparseTermReaderImpl
{
public:
    SparseTermReaderImpl(const FieldInfo& fieldInfo, IndexLevel indexLevel);

    ~SparseTermReaderImpl();
public:
    void open(Directory* pDirectory,const char* barrelname);

    void reopen();

    void close() ;

public:
    FieldInfo fieldInfo_;

    boost::shared_array<TERM_TABLE> sparseTermTable_;

    InputDescriptor* pInputDescriptor_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int32_t nSparseSize_;

    int64_t nVocLength_;

    int64_t nBeginOfVoc_;

    std::string barrelName_;

    Directory* pDirectory_;

    IndexLevel indexLevel_;

    unsigned VOC_ENTRY_LENGTH;
};

/**
* @brief RTDiskTermReader
*/
class RTDiskTermReader: public TermReader
{
public:
    RTDiskTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    RTDiskTermReader(const boost::shared_ptr<SparseTermReaderImpl>& pTermReaderImpl);

    virtual ~RTDiskTermReader();
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

protected:
    TermInfo* termInfo(Term* term);

    TermInfo* searchBuffer(termid_t termId, int end);

    int fillBuffer(int pos);

    FieldInfo* getFieldInfo() { return &pTermReaderImpl_->fieldInfo_; }

protected:
    boost::shared_ptr<SparseTermReaderImpl> pTermReaderImpl_;

    TermInfo* pCurTermInfo_;

    IndexInput* pVocInput_;

    boost::scoped_array<TERM_TABLE> bufferTermTable_;

    boost::shared_array<TERM_TABLE> sparseTermTable_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int64_t nBeginOfVoc_;

    friend class RTDiskTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

/**
* @brief MemTermReader
*/
class MemTermReader : public TermReader
{
public:
    MemTermReader(const char* field,FieldIndexer* pIndexer);

    virtual ~MemTermReader();
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

private:
    std::string field_;

    FieldIndexer* pIndexer_;

    TermInfo curTermInfo_;

    boost::shared_ptr<RTPostingWriter> pCurPosting_;

    friend class MemTermIterator;
};

/**
* @brief BlockTermReader
*/
class BlockTermReader: public RTDiskTermReader
{
public:
    BlockTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    BlockTermReader(const boost::shared_ptr<SparseTermReaderImpl>& pTermReaderImpl);

public:
    TermIterator* termIterator(const char* field);

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    TermReader* clone() ;

protected:
    friend class BlockTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};

/**
* @brief ChunkTermReader
*/
class ChunkTermReader: public RTDiskTermReader
{
public:
    ChunkTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    ChunkTermReader(const boost::shared_ptr<SparseTermReaderImpl>& pTermReaderImpl);

public:
    TermIterator* termIterator(const char* field);

    TermDocFreqs* termDocFreqs();

    TermPositions* termPositions();

    TermReader* clone() ;

protected:
    friend class ChunkTermIterator;
    friend class CollectionIndexer;
    friend class SingleIndexBarrelReader;
};


}

NS_IZENELIB_IR_END

#endif
