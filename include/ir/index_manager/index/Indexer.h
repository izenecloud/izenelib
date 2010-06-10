/**
* @file        Indexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The interface class of IndexManager component in SF1v5.0
*/

#ifndef INDEXER_H
#define INDEXER_H

#include <ir/index_manager/utility/IndexManagerConfig.h>

#include <ir/index_manager/index/CommonItem.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/BTreeIndex.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/BitVector.h>

#include <util/ThreadModel.h>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <deque>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#define MANAGER_PURE_INDEX 0x0
#define MANAGER_INDEXING_BTREE 0x0001			///has btree index
#define MANAGER_INDEXING_STANDALONE_MERGER 0x0002  //merge index in a stand alone thread

typedef uint16_t ManagerType;

enum IndexStatus
{
    EMPTY,  /// There are no index files in the directory
    CORRUPT, /// The index is corrupt
    CONSISTENT  /// The index is OK and later index could be built incrementally
};

class BarrelsInfo;
class IndexReader;
/**
*The interface class of IndexManager component in SF1v5.0
 * @brief It is the interface component of the IndexManager.
*/
class Indexer: private boost::noncopyable
{
public:

    Indexer(ManagerType managerType = MANAGER_INDEXING_BTREE);

    virtual ~Indexer();
public:
    /// API for indexing
    ///insert document
    int insertDocument(IndexerDocument& doc);
    ///mark a document as deleted
    int removeDocument(collectionid_t colID, docid_t docId);
    ///update document
    int updateDocument(IndexerDocument& doc);	
    /// flush in-memory index to disk
    void flush();
    /// merge all index barrels into a single barrel
    void optimizeIndex();

    ///check whether the integrity of indices, always used when starts up
    IndexStatus checkIntegrity();

public:
    ///API for query
    size_t getDistinctNumTermsByProperty(collectionid_t colID, const std::string& property);
	
    bool getDocsByTermInProperties(termid_t termID, collectionid_t colID, std::vector<std::string> properties, std::deque<docid_t>& docIds);

    bool getDocsByTermInProperties(termid_t termID, collectionid_t colID, std::vector<std::string> properties, std::deque<CommonItem>& commonSet);

    bool getTermFrequencyInCollectionByTermId ( const std::vector<termid_t>& termIdList, const unsigned int collectionId, const std::vector<std::string>& propertyList, std::vector<unsigned int>& termFrequencyList );

public:
    ///API for BTreeIndex
    bool getDocsByPropertyValue(collectionid_t colID, std::string property, PropertyType value, BitVector& docs);

    bool getDocsByPropertyValueRange(collectionid_t colID, std::string property, PropertyType value1, PropertyType value2, BitVector& docs);

    bool getDocsByPropertyValueLessThan(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueLessThanOrEqual(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueGreaterThan(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueGreaterThanOrEqual(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueIn(collectionid_t colID, std::string property, std::vector<PropertyType> values, BitVector&docList);

    bool getDocsByPropertyValueNotIn(collectionid_t colID, std::string property, std::vector<PropertyType> values, BitVector&docList);

    bool getDocsByPropertyValueNotEqual(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueStart(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueEnd(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

    bool getDocsByPropertyValueSubString(collectionid_t colID, std::string property, PropertyType value, BitVector&docList);

public:
    ///API for configuration
    void setIndexManagerConfig(const IndexManagerConfig& config,const std::map<std::string, uint32_t>& collectionIdMapping);

    IndexManagerConfig* getIndexManagerConfig() { return pConfigurationManager_;}

    ManagerType getIndexerType() {return managerType_;}

    const std::map<std::string, IndexerCollectionMeta>& getCollectionsMeta();

    izenelib::util::ReadWriteLock& getLock() { return mutex_; }

public:
    BarrelsInfo* getBarrelsInfo() { return pBarrelsInfo_; }

    Directory* getDirectory() { return pDirectory_; }

    void setBasePath(std::string basePath);

    void setDirty(bool dirty);

    bool isDirty() { return dirty_; }

    IndexWriter* getIndexWriter(){return pIndexWriter_;}
    
    IndexReader* getIndexReader() { return pIndexReader_;}

    BTreeIndexer* getBTreeIndexer() { return pBTreeIndexer_; }

    fieldid_t getPropertyIDByName(collectionid_t colID, string property)
    {
        return property_name_id_map_[colID][property];
    }

    std::string getBasePath();

protected:

    void openDirectory(const std::string& storagePolicy);

    void close();

protected:
    ManagerType managerType_;

    Directory* pDirectory_;

    volatile bool dirty_;

    BarrelsInfo* pBarrelsInfo_;

    IndexWriter* pIndexWriter_;

    IndexReader* pIndexReader_;

    IndexManagerConfig* pConfigurationManager_;

    BTreeIndexer* pBTreeIndexer_;

    std::map<collectionid_t, std::map<string, fieldid_t> > property_name_id_map_;

    izenelib::util::ReadWriteLock mutex_;

    friend class IndexWriter;

    friend class IndexReader;

    friend class IndexBarrelWriter;	

    friend class IndexBarrelReader;

    friend class IndexMerger;

    friend class FieldIndexer;

    friend class IndexMergeManager;
};

}
NS_IZENELIB_IR_END

#endif
