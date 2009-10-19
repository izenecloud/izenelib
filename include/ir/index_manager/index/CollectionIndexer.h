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
#include <ir/index_manager/index/ForwardIndexWriter.h>

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

    void setForwardIndexWriter(ForwardIndexWriter* pForwardIndexWriter){pForwardIndexWriter_ = pForwardIndexWriter;}

    void addDocument(IndexerDocument* pDoc);

    bool deleteDocument(IndexerDocument* pDoc,string barrelName, Directory* pDirectory, OutputDescriptor* desc, bool inMemory);

    void write(OutputDescriptor* desc);

    void reset();

    FieldIndexer* getFieldIndexer(fieldid_t fid);

    FieldIndexer* getFieldIndexer(const char* field);

    FieldsInfo* getFieldsInfo() { return pFieldsInfo_;}

private:

    bool removeDocumentInField(docid_t docid, FieldInfo* pFieldInfo, 
        boost::shared_ptr<LAInput> laInput, string barrelName, Directory* pDirectory, OutputDescriptor* desc);

    bool removeDocumentInField(docid_t docid, FieldInfo* pFieldInfo, 
        boost::shared_ptr<ForwardIndex> forwardindex, string barrelName, Directory* pDirectory, OutputDescriptor* desc);

private:
    collectionid_t colID_;

    MemCache* pMemCache_;

    map<string, boost::shared_ptr<FieldIndexer> > fieldIndexerMap_;

    FieldsInfo* pFieldsInfo_;

    Indexer* pIndexer_;

    ForwardIndexWriter* pForwardIndexWriter_;
};

}

NS_IZENELIB_IR_END

#endif
