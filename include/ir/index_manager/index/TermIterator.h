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

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class Posting;
class InputDescriptor;
class DiskTermReader;
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

    Posting* termPosting();

    size_t setBuffer(char* pBuffer,size_t bufSize);

private:
    VocReader* pTermReader_;      ///parent term reader

    Term* pCurTerm_;         ///current term in this iterator

    TermInfo* pCurTermInfo_;      ///current term info in this iterator

    Posting* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
};

class Directory;
class FieldInfo;
/*
* Disk Term Iterator that does not need to load vocabulary into memory
* it is only used for index merging
*/
class DiskTermIterator : public TermIterator
{
public:
    DiskTermIterator(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    ~DiskTermIterator();

    bool next();

    const Term* term();

    const TermInfo* termInfo();

    Posting* termPosting();

private:
    Directory* pDirectory_;	

    FieldInfo* pFieldInfo_;
	
    IndexInput* pVocInput_;

    int32_t nTermCount_;

    int64_t nVocLength_;

    std::string barrelName_;
	
    Term* pCurTerm_;		 ///current term in this iterator

    TermInfo* pCurTermInfo_;	  ///current term info in this iterator

    Posting* pCurTermPosting_;   ///current term's posting in this iterator

    InputDescriptor* pInputDescriptor_;

    int32_t nCurPos_;
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

