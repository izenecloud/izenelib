/**
* @file        CollectionIndexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef COLLECTIONINDEXER_H
#define COLLECTIONINDEXER_H

#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/DocLengthWriter.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>

#include <util/ustring/UString.h>

#include <boost/serialization/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class OutputDescriptor;
class Indexer;
/**
*@brief  CollectionIndexer will is the internal indexer of IndexBarrelWriter, when indexing a document, it will choose the according
* CollectionIndexer to process. Each collection has its own CollectionIndexer
*/
class CollectionIndexer
{
public:
    CollectionIndexer(collectionid_t id,Indexer* pIndexer);

    ~CollectionIndexer();
public:

    void checkbinlog();

    void deletebinlog();

    void setSchema(const IndexerCollectionMeta& schema);

    void setIndexMode(boost::shared_ptr<MemCache> pMemCache, bool realtime);

    void setFieldIndexers();

    void addDocument(IndexerDocument& doc);

    void write(OutputDescriptor* desc);

    void reset();

    void flushDocLen() {
        if (pDocLengthWriter_) pDocLengthWriter_->flush();
    }

    FieldIndexer* getFieldIndexer(fieldid_t fid);

    FieldIndexer* getFieldIndexer(const char* field);

    FieldsInfo* getFieldsInfo() { return pFieldsInfo_;}

    size_t getNumFieldIndexers(){ return fieldIndexerMap_.size();}
private:
    collectionid_t colID_;

    std::map<std::string, boost::shared_ptr<FieldIndexer> > fieldIndexerMap_;

    FieldsInfo* pFieldsInfo_;

    Indexer* pIndexer_;

    DocLengthWriter* pDocLengthWriter_;

    size_t docLengthWidth_;
};

}

NS_IZENELIB_IR_END

#endif
