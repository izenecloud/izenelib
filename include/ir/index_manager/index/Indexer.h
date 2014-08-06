/**
* @file        Indexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The interface class of IndexManager component in SF1v5.0
*/

#ifndef INDEXER_H
#define INDEXER_H


#include <ir/index_manager/utility/IndexManagerConfig.h>
#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>
#include <ir/index_manager/index/CommonItem.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/Bitset.h>

#include <util/ThreadModel.h>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>

#include <map>
#include <deque>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

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

    Indexer();

    virtual ~Indexer();
public:
    /// API for indexing
    ///insert document
    int insertDocument(IndexerDocument& doc);
    ///mark a document as deleted
    int removeDocument(collectionid_t colID, docid_t docId);
    ///update not R-type document
    int updateDocument(IndexerDocument& doc);
    //update R-type document
    int updateRtypeDocument(IndexerDocument& oldDoc, IndexerDocument& doc);
    /// flush in-memory index to disk
    /// force = true means to flush all indices(inverted index, btreeindex,doclength)
    /// force = false means to only flush (btreeindex,doclength)
    void flush(bool force = true);
    /// merge all index barrels into a single barrel
    void optimizeIndex();
    ///check whether the integrity of indices, always used when starts up
    IndexStatus checkIntegrity();

    /**
     * Pause current merging activity.
     * Notes: this function only works when IndexManagerConfig.mergeStrategy_.isAsync_ is true.
     * Notes: if merging thread is removing barrel files currently, which might make query functions return no result,
     *        this function would wait until the end of barrels removal and merged barrel creation.
     */
    void pauseMerge();

    /**
     * Continue the merging activity, which is paused by previous call of @p pauseMerge().
     * Notes: this function only works when IndexManagerConfig.mergeStrategy_.isAsync_ is true.
     */
    void resumeMerge();

    /**
     * Block the calling thread until the merge thread finishes its all tasks,
     * and create a new thread for future merge request.
     * Notes: this function only works when IndexManagerConfig.mergeStrategy_.isAsync_ is true.
     */
    void waitForMergeFinish();

public:
    ///API for query
    size_t getDistinctNumTermsByProperty(collectionid_t colID, const std::string& property);

    bool getDocsByTermInProperties(termid_t termID, collectionid_t colID, const std::vector<std::string>& properties, std::deque<docid_t>& docIds);

    bool getDocsByTermInProperties(termid_t termID, collectionid_t colID, const std::vector<std::string>& properties, std::deque<CommonItem>& commonSet);

    bool getTermFrequencyInCollectionByTermId ( const std::vector<termid_t>& termIdList, const unsigned int collectionId, const std::vector<std::string>& propertyList, std::vector<unsigned int>& termFrequencyList );

public:
    ///API for BTreeIndex
    bool seekTermFromBTreeIndex(collectionid_t colID, const std::string& property, const PropertyType& value);

    bool getDocsByNumericValue(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docs);

    bool getDocsByPropertyValue(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docs);

    bool getDocsByPropertyValue(collectionid_t colID, const std::string& property, const PropertyType& value, BTreeIndexerManager::ValueType& docList);

    template <typename word_t>
    bool getDocsByPropertyValue(collectionid_t colID, const std::string& property, const PropertyType& value, EWAHBoolArray<word_t>& docs);

    bool getDocsByPropertyValueRange(collectionid_t colID, const std::string& property, const PropertyType& value1, const PropertyType& value2, Bitset& docs);

    bool getDocsByPropertyValueLessThan(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueLessThanOrEqual(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueGreaterThan(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueGreaterThanOrEqual(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueIn(collectionid_t colID, const std::string& property, const std::vector<PropertyType>& values, Bitset& docList);

    template <typename word_t>
    bool getDocsByPropertyValueIn(collectionid_t colID, const std::string& property, const std::vector<PropertyType>& values, Bitset& bitset, EWAHBoolArray<word_t>& docsList);

    bool getDocsByPropertyValueNotIn(collectionid_t colID, const std::string& property, const std::vector<PropertyType>& values, Bitset& docList);

    bool getDocsByPropertyValueNotEqual(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueStart(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueEnd(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

    bool getDocsByPropertyValueSubString(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);
    bool getDocsByPropertyValuePGS(collectionid_t colID, const std::string& property, const PropertyType& value, Bitset& docList);

public:
    ///API for configuration
    void setIndexManagerConfig(const IndexManagerConfig& config,const std::map<std::string, uint32_t>& collectionIdMapping);

    IndexManagerConfig* getIndexManagerConfig() { return pConfigurationManager_;}

    void setIndexMode(const std::string& mode);

    const std::map<std::string, IndexerCollectionMeta>& getCollectionsMeta();

    izenelib::util::ReadWriteLock& getLock() { return mutex_; }

public:
    void checkbinlog() { pIndexWriter_->checkbinlog(); }

    void deletebinlog() { pIndexWriter_->deletebinlog(); }

    BarrelsInfo* getBarrelsInfo() { return pBarrelsInfo_; }

    Directory* getDirectory() { return pDirectory_; }

    void setBasePath(const std::string& basePath);

    void setDirty();

    bool isDirty() { return dirty_; }

    IndexWriter* getIndexWriter(){return pIndexWriter_;}

    IndexReader* getIndexReader();

    BTreeIndexerManager* getBTreeIndexer() { return pBTreeIndexer_; }

    size_t numDocs() { return pBarrelsInfo_->getDocCount(); }

    fieldid_t getPropertyIDByName(collectionid_t colID, const std::string& property)
    {
        return property_name_id_map_[colID][property];
    }

    std::string getBasePath();

    int getSkipInterval() { return skipInterval_; }

    int getMaxSkipLevel() { return maxSkipLevel_; }

    bool isRealTime() { return realTime_; }

    CompressionType getIndexCompressType() { return indexingType_; }

    void close();

protected:

    void openDirectory(const std::string& storagePolicy);

protected:
    Directory* pDirectory_;

    volatile bool dirty_;

    BarrelsInfo* pBarrelsInfo_;

    IndexWriter* pIndexWriter_;

    IndexReader* pIndexReader_;

    IndexManagerConfig* pConfigurationManager_;

//     BTreeIndexer* pBTreeIndexer_;
    BTreeIndexerManager* pBTreeIndexer_;

    int skipInterval_;

    int maxSkipLevel_;

    bool realTime_;

    CompressionType indexingType_;

    std::map<collectionid_t, std::map<string, fieldid_t> > property_name_id_map_;

    izenelib::util::ReadWriteLock mutex_;

    boost::mutex indexMutex_;

    friend class IndexWriter;

    friend class IndexReader;

    friend class IndexBarrelWriter;

    friend class IndexBarrelReader;

    friend class IndexMerger;

    friend class FieldIndexer;

    friend class IndexMergeManager;
};

template <typename word_t>
bool Indexer::getDocsByPropertyValue(collectionid_t colID, const std::string& property, const PropertyType& value, EWAHBoolArray<word_t>& docs)
{
    BOOST_ASSERT(pConfigurationManager_->indexStrategy_.isIndexBTree_);
    pBTreeIndexer_->getValue(property, value, docs);
    return true;
}

template <typename word_t>
bool Indexer::getDocsByPropertyValueIn(collectionid_t colID, const std::string& property, const std::vector<PropertyType>& values, Bitset& bitset, EWAHBoolArray<word_t>& docList)
{
    BOOST_ASSERT(pConfigurationManager_->indexStrategy_.isIndexBTree_);
    pBTreeIndexer_->getValueIn(property, values, bitset, docList);
    return true;
}

}
NS_IZENELIB_IR_END

#endif
