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
#include <ir/index_manager/index/ForwardIndex.h>

#include <util/ThreadModel.h>

#include <3rdparty/am/stx/btree_map>

#include <boost/thread.hpp>
#include <string>
#include <deque>

using namespace izenelib::util;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
//Since TermID is got from hashfunc, DynamicArray is not suitable to be used as the container.
typedef stx::btree_map<unsigned int, InMemoryPosting* > InMemoryPostingMap;

class TermReader;
/**
*@brief  FieldIndexer will is the internal indexer of CollectionIndexer, when indexing a document, it will choose the according
* FieldIndexer to process. Each Field has its own FieldIndexer
*/
class Indexer;
class FieldIndexer
{
public:
    FieldIndexer(MemCache* pMemcache,Indexer* pIndexer);

    ~FieldIndexer();
public:
    const char* getField() { return field.c_str(); }

     void setField(const char* strfield) { field = strfield;}

    void addField(docid_t docid, boost::shared_ptr<LAInput> laInput);

    void addField(docid_t docid, boost::shared_ptr<ForwardIndex> forwardindex);

    void reset();

    uint64_t distinctNumTerms() {return postingMap_.size();}

    fileoffset_t write(OutputDescriptor* pWriterDesc);

    void setFilePointer(fileoffset_t off) {vocFilePointer_ = off;};

    fileoffset_t getFilePointer() { return vocFilePointer_;};

    TermReader* termReader();

private:
    InMemoryPostingMap postingMap_;

    std::string field;

    MemCache* pMemCache_;

    Indexer* pIndexer_;

    fileoffset_t vocFilePointer_;

    izenelib::util::ReadWriteLock rwLock_;

    friend class InMemoryTermReader;
    friend class InMemoryTermIterator;
};

}

NS_IZENELIB_IR_END

#endif
