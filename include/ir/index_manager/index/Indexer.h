/**
* @file        Indexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The interface class of IndexManager component in SF1v5.0
*/

#ifndef INDEXER_H
#define INDEXER_H

#include <ir/index_manager/utility/IndexManagerConfig.h>
#include <ir/index_manager/adaptor/DocumentManagerClient.h>

#include <ir/index_manager/index/CommonItem.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/BTreeIndex.h>
#include <ir/index_manager/index/IndexingProgressStatus.h>
#include <ir/index_manager/index/IndexerDocument.h>

#include <ir/index_manager/store/Directory.h>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <ext/hash_map>
//#include <boost/unordered_map.hpp>
#include <boost/serialization/hash_map.hpp>

#include <map>
#include <deque>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

enum ManagerType
{
    MANAGER_TYPE_LOCAL,       /// Deployed in a single machine
    MANAGER_TYPE_DATAPROCESS, /// Deployed with Data Process
    MANAGER_TYPE_INDEXPROCESS /// Deployed as the Index Process
};

typedef __gnu_cxx::hash_map    <unsigned int,float> ID_FREQ_MAP_T;
//typedef boost::unordered_map	  <unsigned int,float>			ID_FREQ_MAP_T;
typedef std::map<std::string, ID_FREQ_MAP_T > DocumentFrequencyInProperties;

typedef int32_t ACCESS_MODE;

const static int32_t ACCESS_CREATE = 0x10;   /// can add or delete files but can't search over it,will delete existed index database

const static int32_t ACCESS_APPEND = 0x20;   /// will append index data do existed index database.


class BarrelsInfo;
class UDTFSAgent;
class BTreeIndexerClient;
class BTreeIndexerServer;
/**
*The interface class of IndexManager component in SF1v5.0
 * @brief It is the interface component of the IndexManager.
*/
class Indexer: private boost::noncopyable
{
public:

    Indexer(ManagerType managerType = MANAGER_TYPE_LOCAL);

    ~Indexer();
public:
    /**
    * @brief   This function inserts a document of a document collection.
    *
    *  @param: document: document to be inserted
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int insertDocument(IndexerDocument& document);

    int insertDocument(collectionid_t colID, docid_t docID);

    int removeDocument(collectionid_t colID, docid_t docID);

    /**
    * @brief   This function creates index of a document collection.
    *
    *  @param colID:   document   collection  identifier
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int insertCollection(collectionid_t colID);
    /**
    * @brief   This function deletes an entire document collection. It removes all the documents of the document collection.
    *
    *  @param colID:   document   collection  identifier
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int removeCollection(collectionid_t colID);
    /**
    * @brief   This function inserts a list of documents  into a document collection.
    *
    * @param colID:   document   collection  identifier
    * @param docIdList:  document id list to be inserted
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int insertDocuments(collectionid_t colID, std::vector<docid_t> docIdList);
    /**
    * @brief   This function remove a list of documents from a document collection.
    *
    *  @param colID:   document   collection  identifier
    *  @param docIdList:  document id list to be removed
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int removeDocuments(collectionid_t colID, std::vector<docid_t> docIdList);
    /**
    * @brief   This function updates a document of a document collection. It first removes the document, and then, adds the new   content of the document to the index of
    * the document collection.
    *
    *  @param colID:   document   collection  identifier
    *  @param docId:  document id list to be updated
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    int updateDocument(collectionid_t colID, docid_t docID);
    /**
    * @brief This function gets status of indexing progress of each collection. The status should contain the number of documents remains unindexed,
    * and estimate time to wait until the indexing is finished.
    *
    *  @param colID:   document  collection  identifier
    *  @param currentProgress:  current index process to be returned
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getIndexingProgressbyCollectionId(collectionid_t colID, IndexingProgressStatus& currentProgress);
    /**
    * @brief   This function gets a commonSet that is a list of commonItem. A commonItem is a part of a document. A commonItem consists of a list of properties and
    *  positions of the terms in each property.
    *
    *  @param termID: term identifier
    *  @param colID:     document collection identifier
    *  @param properties: list of property name
    *  @param commonSet: list of commonItem.A commonItem is a part of document. It consists of a list of   commonItemProperty. A commonItemProperty
    *      is the document property the input  term   occurs.   The commonItemProperty contains the term positions in the property.
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByTermInProperties(termid_t termID, collectionid_t colID, std::vector<std::string> properties, std::vector<CommonItem>& commonSet);
    /**
    * @brief   This function gets a commonSet that is a list of commonItem. A commonItem is a part of a document. A commonItem consists of a list of properties and
    *  positions of the terms in each property.
    *
    *  @param termIDs: term identifiers
    *  @param colID:     document collection identifier
    *  @param properties: list of property name
    *  @param commonSet: list of commonItem.A commonItem is a part of document. It consists of a list of   commonItemProperty. A commonItemProperty
    *      is the document property the input  term   occurs.   The commonItemProperty contains the term positions in the property.
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByTermsInProperties(std::vector<termid_t> termIDs, collectionid_t colID, std::vector<std::string> properties, std::vector<CommonItem>& commonSet);
    /**
    * @brief   This function gets a word offset list given a sequence of query terms
    *  positions of the terms in each property.
    *
    *  @param queryTermIdList: query term list
    *  @param docId: document identifier
    *  @param colId:   document collection identifier
    *  @param propertyname: 
    *  @param wordOffsetListOfQuery: word offset list
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getWordOffsetListOfQueryByDocumentProperty (const std::vector<termid_t>& queryTermIdList, collectionid_t colId, docid_t docId,  string propertyName, std::vector<std::vector<std::pair<unsigned int, unsigned int> > >& wordOffsetListOfQuery );
  
    /**
    * @brief   This function gets a list of TermFrequency according to termid and collection id
    *
    *  @param termIdList: term identifier
    *  @param colID:     document collection identifier
    *  @param properyList: list of property name
    *  @param termFrequencyList: list of TermFrequency
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getTermFrequencyInCollectionByTermId ( const std::vector<termid_t>& termIdList, const unsigned int collectionId, const std::vector<std::string>& propertyList, std::vector<unsigned int>& termFrequencyList );
    /**
    * @brief   This function gets a map of Document Frequency according to property and term id
    *
    * @param termIdList: list of term Ids.
    * @param collectionId: id of the collection.
    * @param propertyList: list of properties.
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    //bool getDocumentFrequencyListByTermIdList(const vector<termid_t>& termIdList, const unsigned int collectionId, const vector<string>&  propertyList, vector<unsigned int>& documentFrequencyList);
    bool getDocumentFrequencyInPropertiesByTermIdList(const std::vector<termid_t>& termIdList, const unsigned int collectionId, const std::vector<std::string>&  propertyList, DocumentFrequencyInProperties& documentFrequencyList);

    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is equal to the input value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docs: list of document identifiers
    *
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValue(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>& docs);
    /**
    * @brief   This function retrieves a list of document identifiers by property value range. The property value of each document in the list is in the input range.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value1: lower value of PropertyType
    *  @param value2: higher value of PropertyType
    *  @param docs: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueRange(collectionid_t colID, std::string property, PropertyType value1, PropertyType value2, std::vector<docid_t>& docs);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is less than a given value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueLessThan(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is less than or equal to a given value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueLessThanOrEqual(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is greater than a given value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueGreaterThan(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is greater than or equal to a given value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueGreaterThanOrEqual(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is in a given set of values.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueIn(collectionid_t colID, std::string property, std::vector<PropertyType> values, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is not in   a given set of values.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param values: vector of PropertyType
    *  @param  docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueNotIn(collectionid_t colID, std::string property, std::vector<PropertyType> values, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list is different from a given value.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueNotEqual(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief This function retrieves a list of document identifiers by property value. The property value of each document in the list starts with a given value. This function is used
    * only with string. For example: the string "indexer" starts with "index"
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueStart(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list ends with a given value. This function is used
    * only with string.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueEnd(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);
    /**
    * @brief   This function retrieves a list of document identifiers by property value. The property value of each document in the list contains a given string. This function is used only
    *  with string.
    *
    *  @param colID:   document   collection  identifier
    *  @param property: property name
    *  @param value: PropertyType
    *  @param docList: list of document identifiers
    *  @return  1: operation completed successfully
    *       0: fail to complete the operation
    *
    */
    bool getDocsByPropertyValueSubString(collectionid_t colID, std::string property, PropertyType value, std::vector<docid_t>&docList);


#ifdef SF1_TIME_CHECK
    /**
     * @brief   Notifies the Document process to print out its  time check results
     */
    void printDocumentProcessTimeCheck();
#endif

public:
    void setIndexManagerConfig(IndexManagerConfig* pConfigManager,const std::map<std::string, uint32_t>& collectionIdMapping);

    IndexManagerConfig* getIndexManagerConfig()
    {
        return pConfigurationManager_;
    }

    void setDocumentManagerClient(DocumentManagerClientType* pDocumentManager)
    {
        pDocumentManager_ = pDocumentManager;
    }

    ManagerType getIndexerType()
    {
        return managerType_;
    }

    void add_index_process_node(string ip, string batchport, string rpcport);

    pair<string,pair<string, string> >& get_curr_index_process();

    bool change_curr_index_process();

    bool destroy_connection(pair<string,pair<string, string> >& node);

    bool initialize_connection(pair<string,pair<string, string> >& node, bool wait=false);

    const std::string& getVersionString() const
    {
        return version_;
    }

    void optimizeIndex();

    void set_property_name_id_map(const std::map<std::string, IndexerCollectionMeta>& collections);

    const std::map<std::string, IndexerCollectionMeta>& getCollectionsMeta();

    BarrelsInfo* getBarrelsInfo()
    {
        return pBarrelsInfo_;
    }

    Directory* getDirectory()
    {
        return pDirectory_;
    }

    void setDirectory(Directory* pDir);

    void setDirty(bool bDirty);

    IndexWriter* getIndexWriter()
    {
        return pIndexWriter_;
    }

    BTreeIndexerInterface* getBTreeIndexer();

    fieldid_t getPropertyIDByName(collectionid_t colID, string property);

private:
    void initIndexManager();

    void openDirectory();

    void close();

private:
    ManagerType managerType_;

    Directory* pDirectory_;

    ACCESS_MODE accessMode_;

    bool dirty_;

    BarrelsInfo* pBarrelsInfo_;

    IndexWriter* pIndexWriter_;

    IndexReader* pIndexReader_;

    IndexManagerConfig* pConfigurationManager_;

    DocumentManagerClientType* pDocumentManager_;

    std::map<collectionid_t, IndexingProgressStatus *> indexProgressStatusMap_;

    boost::mutex mutex_;

    BTreeIndexer* pBTreeIndexer_;

    BTreeIndexerClient* pBTreeIndexerClient_;

    BTreeIndexerServer* pBTreeIndexerServer_;

    std::map<collectionid_t, std::map<string, fieldid_t> > property_name_id_map_;

    std::deque<pair<string, pair<string, string> > > index_process_address_;

    UDTFSAgent* pAgent_;

    friend class IndexWriter;

    friend class IndexReader;

    friend class IndexBarrelReader;

    std::string version_;
};

class IndexerFactory
{
public:
    IndexerFactory():pIndexer_(NULL) {}
    ~IndexerFactory()
    {
        if (pIndexer_)
            delete pIndexer_;
    }
public:
    Indexer* getIndexer()
    {
        if (NULL == pIndexer_)
            pIndexer_ = new Indexer();
        return pIndexer_;
    }
private:
    Indexer* pIndexer_;
};
///extern Indexer indexer;
extern IndexerFactory indexerFactory;
}

NS_IZENELIB_IR_END

#endif
