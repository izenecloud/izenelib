#ifndef ABS_TERM_READER_H
#define ABS_TERM_READER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/store/Directory.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class TermIterator;

class TermReader
{
public:
    TermReader(void);

    TermReader(FieldInfo* pFieldInfo);

    virtual ~TermReader(void);
public:
    virtual void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    virtual TermIterator* termIterator(const char* field) = 0;

    virtual bool seek(Term* pTerm) = 0;

    virtual TermDocFreqs*	termDocFreqs() = 0;

    virtual TermPositions*	termPositions() = 0;

    virtual freq_t docFreq(Term* term) = 0;

    virtual void close() = 0;

    virtual TermReader*	clone() = 0;

    virtual TermInfo* termInfo(Term* term) { return NULL;};
public:
    FieldInfo* getFieldInfo() { return pFieldInfo_;}

    void setFieldInfo(FieldInfo* pFieldInfo) { pFieldInfo_ = pFieldInfo; }
protected:
    FieldInfo* pFieldInfo_;	///reference to field info

    friend class TermDocFreqs;
    friend class MultiFieldTermReader;
    friend class IndexReader;
    friend class InMemoryTermReader;
};



}

NS_IZENELIB_IR_END

#endif
