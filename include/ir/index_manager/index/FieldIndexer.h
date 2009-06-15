/**
* @file        FieldIndexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef FIELDINDEXER_H
#define FIELDINDEXER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/LAInput.h>

#include <util/DynamicArray.h>

#include <boost/thread.hpp>
#include <string>
#include <deque>

using namespace izenelib::util;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
typedef DynamicArray<InMemoryPosting*,Const_NullValue<InMemoryPosting*> > DynPostingArray;

class TermReader;
/**
*@brief  FieldIndexer will is the internal indexer of CollectionIndexer, when indexing a document, it will choose the according
* FieldIndexer to process. Each Field has its own FieldIndexer
*/

class FieldIndexer
{
public:
    FieldIndexer(MemCache* pMemcache);

    ~FieldIndexer();
public:
    const char* getField() { return field.c_str(); }

     void setField(const char* strfield) { field = strfield;}

    void addField(docid_t docid, boost::shared_ptr<LAInput> laInput);

    void removeField(docid_t docid, boost::shared_ptr<LAInput> laInput);

    void reset();

    uint64_t distinctNumTerms() {return array_.length();}

    fileoffset_t write(OutputDescriptor* pWriterDesc);

    void setFilePointer(fileoffset_t off) {vocFilePointer_ = off;};

    fileoffset_t getFilePointer() { return vocFilePointer_;};

    TermReader* termReader();

    boost::mutex& getLock() { return mutex_;}
private:
    DynPostingArray array_;

    std::string field;

    MemCache* pMemCache_;

    fileoffset_t vocFilePointer_;

    boost::mutex mutex_;

    friend class InMemoryTermReader;
    friend class InMemoryTermIterator;
};

}

NS_IZENELIB_IR_END

#endif
