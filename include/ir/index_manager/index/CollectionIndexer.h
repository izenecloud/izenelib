/**
* @file        CollectionIndexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef COLLECTIONINDEXER_H
#define COLLECTIONINDEXER_H

#include <boost/serialization/shared_ptr.hpp>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>

#include <ir/index_manager/index/IndexerDocument.h>

#include <ir/index_manager/index/IndexerPropertyConfig.h>

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/BTreeIndex.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/DocLengthWriter.h>

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
    CollectionIndexer(collectionid_t id,MemCache* pCache,Indexer* pIndexer);

    ~CollectionIndexer();
public:

    void setSchema(const IndexerCollectionMeta& schema);

    void setFieldIndexers();

    void addDocument(IndexerDocument* pDoc);

    void write(OutputDescriptor* desc);

    void reset();

    FieldIndexer* getFieldIndexer(fieldid_t fid);

    FieldIndexer* getFieldIndexer(const char* field);

    FieldsInfo* getFieldsInfo() { return pFieldsInfo_;}

private:
    collectionid_t colID_;

    MemCache* pMemCache_;

    map<string, boost::shared_ptr<FieldIndexer> > fieldIndexerMap_;

    FieldsInfo* pFieldsInfo_;

    Indexer* pIndexer_;

    DocLengthWriter* pDocLengthWriter_;

    size_t docLengthWidth_;
};

}

NS_IZENELIB_IR_END

#endif
