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
#include <ir/index_manager/index/EPostingReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class PostingReader;
class InputDescriptor;
class VocReader;
/**
* Iterate terms from index barrel files(*.voc), the vocabulary should
* first be all loaded into memory by VocReader
*/
class VocIterator : public TermIterator
{
public:
    VocIterator(VocReader* termReader);

    ~VocIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    PostingReader* termPosting();

private:
    VocReader* pTermReader_;      ///parent term reader

    Term* pCurTerm_;         ///current term in this iterator

    TermInfo* pCurTermInfo_;      ///current term info in this iterator

    PostingReader* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
};

class Directory;
class FieldInfo;
/*
* Disk Term Iterator that does not need to load vocabulary into memory
* it is only used for index merging
*/
class RTDiskTermIterator : public TermIterator
{
public:
    RTDiskTermIterator(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    ~RTDiskTermIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    PostingReader* termPosting();

private:
    Directory* pDirectory_;

    FieldInfo* pFieldInfo_;

    IndexInput* pVocInput_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;

    Term* pCurTerm_;		 ///current term in this iterator

    TermInfo* pCurTermInfo_;	  ///current term info in this iterator

    RTDiskPostingReader* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
};


class MemTermReader;
/**
* Iterator terms from memory
*/
class MemTermIterator : public TermIterator
{
public:
    MemTermIterator(MemTermReader* pTermReader);

    virtual ~MemTermIterator();
public:

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    PostingReader* termPosting();

protected:
    MemTermReader* pTermReader_;

    Term* pCurTerm_;         ///current term in this iterator

    TermInfo* pCurTermInfo_;      ///current term info in this iterator

    MemPostingReader* pCurTermPosting_;   ///current term's posting in this iterator

    InMemoryPostingMap::iterator postingIterator_;

    InMemoryPostingMap::iterator postingIteratorEnd_;
};


/*
* Disk Term Iterator that does not need to load vocabulary into memory
* it is only used for index merging
*/
class BlockTermIterator : public TermIterator
{
public:
    BlockTermIterator(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    ~BlockTermIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    PostingReader* termPosting();

private:
    Directory* pDirectory_;

    FieldInfo* pFieldInfo_;

    IndexInput* pVocInput_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;

    Term* pCurTerm_;		 ///current term in this iterator

    TermInfo* pCurTermInfo_;	  ///current term info in this iterator

    BlockPostingReader* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
};


/*
* Disk Term Iterator that does not need to load vocabulary into memory
* it is only used for index merging
*/
class ChunkTermIterator : public TermIterator
{
public:
    ChunkTermIterator(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo, IndexLevel indexLevel);

    ~ChunkTermIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    PostingReader* termPosting();

private:
    Directory* pDirectory_;

    FieldInfo* pFieldInfo_;

    IndexInput* pVocInput_;

    int32_t nVersion_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;

    Term* pCurTerm_;		 ///current term in this iterator

    TermInfo* pCurTermInfo_;	  ///current term info in this iterator

    ChunkPostingReader* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
};

}

NS_IZENELIB_IR_END

#endif
