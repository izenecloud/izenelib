/**
* @file        IndexBarrelReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index Barrel Reader
*/
#ifndef INDEXBARRELREADER_H
#define INDEXBARRELREADER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/store/Directory.h>

#include <map>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class TermReader;
class Indexer;
/**
* The base class of SingleIndexBarrelReader and MultiIndexBarrelReader
* If there is only one barrel, then SingleIndexBarrelReader will be generated
* ,or else IndexReader will generate the instance of MultiIndexBarrelReader
*/
class IndexBarrelReader
{
public:
    IndexBarrelReader(Indexer* pIndex);
    IndexBarrelReader();
    virtual ~IndexBarrelReader(void);
public:
    /**
    * Return the TermReader instance
    */
    virtual TermReader* termReader(collectionid_t colID) = 0;

    virtual TermReader* termReader(collectionid_t colID,const char* field) { return NULL; }

    virtual void deleteDocumentPhysically(IndexerDocument* pDoc) = 0;

    virtual size_t getDistinctNumTerms(collectionid_t colID,const std::string& property) = 0;
	
    virtual void close() = 0;

    virtual void reopen() = 0;

    Indexer* getIndexer() { return pIndexer_; }
protected:
    Indexer* pIndexer_;
};

}

NS_IZENELIB_IR_END
	
#endif
