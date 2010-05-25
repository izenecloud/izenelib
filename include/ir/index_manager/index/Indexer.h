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

#define MANAGER_TYPE_LOCAL 0x0100				/// Deployed in a single machine
#define MANAGER_TYPE_CLIENTPROCESS 0x0200		/// Deployed as the client indexer
#define MANAGER_TYPE_SERVERPROCESS 0x0400		/// Deployed as the server indexer

#define MANAGER_INDEXING_FORWARD 0x0001		///has fowardindex
#define MANAGER_INDEXING_BTREE 0x0002			///has btree index

typedef uint16_t ManagerType;

enum IndexStatus
{
    EMPTY,  /// There are no index files in the directory
    CORRUPT, /// The index is corrupt
    CONSISTENT  /// The index is OK and later index could be built incrementally
};

class BarrelsInfo;
class UDTFSAgent;
class BTreeIndexerClient;
class BTreeIndexerServer;
class IndexReader;
/**
*The interface class of IndexManager component in SF1v5.0
 * @brief It is the interface component of the IndexManager.
*/
class Indexer: private boost::noncopyable
{
public:

    Indexer(ManagerType managerType = MANAGER_TYPE_LOCAL|MANAGER_INDEXING_BTREE);

    virtual ~Indexer();
public:
    /// API for indexing
	
    ///pDoc should be deleted by the user
    int insertDocumentPhysically(IndexerDocument* pDoc);
    ///pDoc will be destroyed by Indexer
    int insertDocument(IndexerDocument* pDoc);
    ///mark a document as deleted
    int removeDocument(collectionid_t colID, docid_t docId);
    ///update document
    int updateDocument(IndexerDocument* pDoc);	
    /// flush in-memory index to disk
    void flush();
    /// merge all index barrels into a single barrel
    void optimizeIndex();

    int removeCollection(collectionid_t colID);

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

    void add_index_process_node(string ip, string batchport, string rpcport);

    pair<string,pair<string, string> >& get_curr_index_process();

    bool change_curr_index_process();

    bool destroy_connection(pair<string,pair<string, string> >& node);

    bool initialize_connection(pair<string,pair<string, string> >& node, bool wait=false);

    const std::string& getVersionString() const { return version_; }

    void set_property_name_id_map(const std::map<std::string, IndexerCollectionMeta>& collections);

    const std::map<std::string, IndexerCollectionMeta>& getCollectionsMeta();

    izenelib::util::ReadWriteLock& getLock() { return mutex_; }

public:
    BarrelsInfo* getBarrelsInfo() { return pBarrelsInfo_; }

    Directory* getDirectory() { return pDirectory_; }

    void setBasePath(std::string basePath);

    void setDirty(bool bDirty);

    bool isDirty() { return dirty_; }

    IndexWriter* getIndexWriter(){return pIndexWriter_;}
    
    IndexReader* getIndexReader() { return pIndexReader_;}

    BTreeIndexerInterface* getBTreeIndexer();

    fieldid_t getPropertyIDByName(collectionid_t colID, string property);

    std::string getBasePath();

protected:
    void initIndexManager();

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

    izenelib::util::ReadWriteLock mutex_;

    BTreeIndexer* pBTreeIndexer_;

    BTreeIndexerClient* pBTreeIndexerClient_;

    BTreeIndexerServer* pBTreeIndexerServer_;

    std::map<collectionid_t, std::map<string, fieldid_t> > property_name_id_map_;

    std::deque<pair<string, pair<string, string> > > index_process_address_;

    UDTFSAgent* pAgent_;

    std::string version_;

    friend class IndexWriter;

    friend class IndexReader;

    friend class IndexBarrelWriter;	

    friend class IndexBarrelReader;

    friend class IndexMerger;
};

}
NS_IZENELIB_IR_END

#endif
