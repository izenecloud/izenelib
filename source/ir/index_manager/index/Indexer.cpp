#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/ParallelTermPosition.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/ForwardIndexReader.h>
#include <ir/index_manager/store/FSDirectory.h>
#include <ir/index_manager/store/RemoteDirectory.h>
#include <ir/index_manager/store/UDTFSAgent.h>
#include <ir/index_manager/store/RAMDirectory.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/index/BTreeIndexerClient.h>
#include <ir/index_manager/index/BTreeIndexerServer.h>

#ifdef SF1_TIME_CHECK
#include <wiselib/profiler/ProfilerGroup.h>
#endif

#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>


#include <algorithm>


using namespace izenelib::ir::indexmanager;

static int degree = 16;
static size_t cacheSize = 50000;
static size_t maxDataSize = 1000;
IndexerFactory indexerFactory;

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define PATCH_VERSION "20090801" // update date

Indexer::Indexer( bool indexingForward, ManagerType managerType)
        :managerType_(managerType)
        ,indexingForward_(indexingForward)
        ,pDirectory_(NULL)
        ,dirty_(false)
        ,pBarrelsInfo_(NULL)
        ,pIndexWriter_(NULL)
        ,pIndexReader_(NULL)
        ,pConfigurationManager_(NULL)
        ,pBTreeIndexer_(NULL)
        ,pBTreeIndexerClient_(NULL)
        ,pBTreeIndexerServer_(NULL)
        ,pAgent_(NULL)
{
    version_ = "Index Manager - ver. alpha ";
    version_ += MAJOR_VERSION;
    version_ += ".";
    version_ += MINOR_VERSION;
    version_ += ".";
    version_ += PATCH_VERSION;
}

Indexer::~Indexer()
{
    close();

    if (pConfigurationManager_) 
        delete pConfigurationManager_;
    pConfigurationManager_ = NULL;

    if(pAgent_)
    {
        delete pAgent_;
        pAgent_ = NULL;
    }

    if(pBTreeIndexerClient_)
        delete pBTreeIndexerClient_;
    if(pBTreeIndexerServer_)
        delete pBTreeIndexerServer_;
}

const std::map<std::string, IndexerCollectionMeta>& Indexer::getCollectionsMeta()
{
    if (!pConfigurationManager_)
        SF1V5_THROW(ERROR_FILEIO,"Configuration values have not been set" );

    return pConfigurationManager_->getCollectionMetaNameMap();
}

void Indexer::setIndexManagerConfig(IndexManagerConfig* pConfigManager,
                                       const std::map<std::string, uint32_t>& collectionIdMapping)
{
    if (pConfigurationManager_)
        delete pConfigurationManager_;
    pConfigurationManager_ = NULL;

    pConfigurationManager_ = new IndexManagerConfig();

    *pConfigurationManager_ = *pConfigManager;

    const std::map<std::string, IndexerCollectionMeta>& collectionsMeta = pConfigManager->getCollectionMetaNameMap();
    std::map<std::string, uint32_t>::const_iterator idIter;
    std::map<std::string, IndexerCollectionMeta>::const_iterator iter;
    map<std::string, IndexerCollectionMeta> collectionList;
    for (iter = collectionsMeta.begin(); iter != collectionsMeta.end(); iter++)
    {
        idIter = collectionIdMapping.find(iter->first);
        if (idIter == collectionIdMapping.end())
        {
            SF1V5_THROW(ERROR_FILEIO,"Invalid input paramter" );
        }

        IndexerCollectionMeta collection(iter->second);
        collection.setColId(idIter->second);
        collectionList.insert( pair<std::string, IndexerCollectionMeta>(iter->first, collection) );
    }
    pConfigurationManager_->setCollectionMetaNameMap( collectionList );

    initIndexManager();
}

void Indexer::initIndexManager()
{
    if (pConfigurationManager_==NULL) return;

    close();

    set_property_name_id_map(pConfigurationManager_->getCollectionMetaNameMap());

    InMemoryPosting::UPTIGHT_ALLOC_CHUNKSIZE = pConfigurationManager_->advance_.uptightAlloc_.chunkSize_;
    InMemoryPosting::UPTIGHT_ALLOC_MEMSIZE = pConfigurationManager_->advance_.uptightAlloc_.memSize_;

    int32_t strategy = 1,n = 32,k = 0,l = 0;

    vector<string> ps = split(pConfigurationManager_->advance_.MMS_,":");
    if (ps.size() >= 2)
    {
        if (!strcasecmp(ps[0].c_str(),"exp"))
        {
            strategy = InMemoryPosting::STRATEGY_ALLOC_EXP;
            if (ps.size() >= 3)
            {
                n = atoi(ps[1].c_str());
                k = atoi(ps[2].c_str());
            }
            else
            {
                n = 32;
                k = 2;
            }
        }
        else if (!strcasecmp(ps[0].c_str(),"explimit"))
        {
            strategy = InMemoryPosting::STRATEGY_ALLOC_EXPLIMIT;
            if (ps.size() >= 4)
            {
                n = atoi(ps[1].c_str());
                k = atoi(ps[2].c_str());
                l = atoi(ps[3].c_str());
            }
            else
            {
                n = 32;
                k = 2;
                l = 256;
            }
        }
        else
        {
            strategy = InMemoryPosting::STRATEGY_ALLOC_CONST;
            n = atoi(ps[1].c_str());
        }
    }
    InMemoryPosting::ALLOCSTRATEGY.strategy = (InMemoryPosting::MEMALLOC_STRATEGY)strategy;
    InMemoryPosting::ALLOCSTRATEGY.n = n;
    InMemoryPosting::ALLOCSTRATEGY.k = k;
    InMemoryPosting::ALLOCSTRATEGY.l = l;

    if(managerType_ == MANAGER_TYPE_CLIENTPROCESS)
    {
        vector<string> nodes = split(pConfigurationManager_->distributeStrategy_.iplist_,"|");
        for(vector<string>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
            add_index_process_node((*iter),pConfigurationManager_->distributeStrategy_.batchport_,
            pConfigurationManager_->distributeStrategy_.rpcport_.c_str());

        pBTreeIndexerClient_ = new BTreeIndexerClient;
        initialize_connection(index_process_address_.front(), true);
        pDirectory_ = new RemoteDirectory();
        pBarrelsInfo_ = new BarrelsInfo();
    }
    else
    {
        if (!strcasecmp(pConfigurationManager_->indexStrategy_.accessMode_.c_str(),"w" ))
            accessMode_ = ACCESS_CREATE;
        else if (!strcasecmp(pConfigurationManager_->indexStrategy_.accessMode_.c_str(),"a"))
            accessMode_ = ACCESS_APPEND;
        else
        {
            SF1V5_THROW(ERROR_FILEIO,"Wrong index access mode" );
        }

        openDirectory();

        if (!strcasecmp(pConfigurationManager_->storeStrategy_.param_.c_str(),"file"))
            pBTreeIndexer_ = new BTreeIndexer(pConfigurationManager_->indexStrategy_.indexLocation_, degree, cacheSize, maxDataSize);

    }
    pIndexWriter_ = new IndexWriter(this);
    pIndexReader_ = new IndexReader(this);

    if(managerType_ == MANAGER_TYPE_SERVERPROCESS)
    {
        pBTreeIndexerServer_ = new BTreeIndexerServer(pConfigurationManager_->distributeStrategy_.rpcport_, pBTreeIndexer_);
    
        boost::thread rpcServerThread(boost::bind(&BTreeIndexerServer::run, pBTreeIndexerServer_));
        rpcServerThread.detach();

        pAgent_ = new UDTFSAgent(pConfigurationManager_->distributeStrategy_.batchport_, this);
        boost::thread agentThread(boost::bind(&UDTFSAgent::run, pAgent_));
        agentThread.join();
    }

}


void Indexer::set_property_name_id_map(const std::map<std::string, IndexerCollectionMeta>& collections)
{
    collectionid_t colID;

    for (std::map<std::string, IndexerCollectionMeta>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
    {
        colID = iter->second.getColId();

        std::map<std::string, fieldid_t> propertyMap;

        std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> documentSchema = (iter->second).getDocumentSchema();
		
        for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator it = documentSchema.begin(); it != documentSchema.end(); it++ )
        {
            if (it->getPropertyId() != BAD_PROPERTY_ID)
            {
                propertyMap.insert(make_pair(it->getName(), it->getPropertyId()));
            }
        }

        property_name_id_map_.insert(make_pair(colID, propertyMap));
    }
}


fieldid_t Indexer::getPropertyIDByName(collectionid_t colID, string property)
{
    return property_name_id_map_[colID][property];
}

void Indexer::openDirectory()
{
    close();

    string path = pConfigurationManager_->indexStrategy_.indexLocation_;
    if ((accessMode_ & ACCESS_APPEND) == ACCESS_APPEND )
    {
        string strBarrel = path;
        strBarrel += "/";
        strBarrel += BARRELS_INFONAME;
        if (!Utilities::dirExists(path.c_str()) || !Utilities::dirExists(strBarrel.c_str()))
        {
            accessMode_ = accessMode_ & (~ACCESS_APPEND);
            accessMode_ = accessMode_ |ACCESS_CREATE;
        }
    }

    if (!strcasecmp(pConfigurationManager_->storeStrategy_.param_.c_str(),"file"))
        pDirectory_ = FSDirectory::getDirectory(path,((accessMode_ & ACCESS_CREATE) == ACCESS_CREATE));
    else
        pDirectory_ = new RAMDirectory();

    pBarrelsInfo_ = new BarrelsInfo();

    pBarrelsInfo_->read(pDirectory_);
    if ((accessMode_ & ACCESS_CREATE) == ACCESS_CREATE)
    {
        pBarrelsInfo_->remove(getDirectory());
    }
    else
    {
        if (strcasecmp(pBarrelsInfo_->getVersion(),SF1_VERSION))
        {
            delete pBarrelsInfo_;
            pBarrelsInfo_ = NULL;
            SF1V5_THROW(ERROR_VERSION,"incompatible version.");
        }
    }
}

void Indexer::setBasePath(std::string basePath)
{
    if(managerType_ != MANAGER_TYPE_FORWARDREADER_AND_MERGER)
        return;
    if (pDirectory_)
        pDirectory_->close();
    pDirectory_ = FSDirectory::getDirectory(basePath,(ACCESS_APPEND & ACCESS_CREATE));

    pIndexReader_ = new IndexReader(this);
    pIndexWriter_ = new IndexWriter(this);
}

std::string Indexer::getBasePath()
{
    string path = pConfigurationManager_->indexStrategy_.indexLocation_;
    if (path[path.length()-1] != '/' )
    {
        path += "/";
    }
    return path;
}

void Indexer::add_index_process_node(string ip, string batchport, string rpcport)
{
    assert(managerType_ == MANAGER_TYPE_CLIENTPROCESS);
    index_process_address_.push_back(make_pair(ip,make_pair(batchport,rpcport)));
}

pair<string,pair<string, string> >& Indexer::get_curr_index_process()
{
    return index_process_address_.front();
}

bool Indexer::change_curr_index_process()
{
    assert(managerType_ == MANAGER_TYPE_CLIENTPROCESS);
		
    pair<string,pair<string, string> > node = index_process_address_.front();
    index_process_address_.pop_front();
    index_process_address_.push_back(node);
    destroy_connection(node);
    initialize_connection(index_process_address_.front());

    return true;
}

BTreeIndexerInterface* Indexer::getBTreeIndexer()
{
    if(managerType_ == MANAGER_TYPE_CLIENTPROCESS)
        return pBTreeIndexerClient_;
    else
        return pBTreeIndexer_;
}

bool Indexer::destroy_connection(pair<string,pair<string, string> >& node)
{
    assert(managerType_ == MANAGER_TYPE_CLIENTPROCESS);

    UDTFile::destroy();
    return true;
}

bool Indexer::initialize_connection(pair<string,pair<string, string> >& node, bool wait)
{
    assert(managerType_ == MANAGER_TYPE_CLIENTPROCESS);

    UDTFile::init(node.first, atoi(node.second.first.c_str()));

    if(wait)
    {
        while(true)
        {
            if(UDTFSError::OK != UDTFile::try_connect())
            {
                cout << "Waiting for Index Process:"<<node.first<< endl;
                boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(1000));
            }
            else
                break;
        }
    }
    else
        if(UDTFSError::OK != UDTFile::try_connect())
            return false;

    pBTreeIndexerClient_->switchServer(node.first, node.second.second);

    return true;
}

void Indexer::close()
{
    if (pIndexReader_)
    {
        delete pIndexReader_;
        pIndexReader_ = NULL;
    }
    if (pIndexWriter_)
    {
        pIndexWriter_->close();
        delete pIndexWriter_;
        pIndexWriter_ = NULL;
    }
    if (pBarrelsInfo_)
    {
        delete pBarrelsInfo_;
        pBarrelsInfo_ = NULL;
    }
    if (pDirectory_)
    {
        pDirectory_->close();
        pDirectory_ = NULL;
    }
    dirty_ = false;
    if (pBTreeIndexer_)
    {
        pBTreeIndexer_->flush();
        delete pBTreeIndexer_;
        pBTreeIndexer_ = NULL;
    }
}


void Indexer::setDirty(bool bDirty)
{
    dirty_ = bDirty;
}

int Indexer::insertDocumentPhysically(IndexerDocument* pDoc)
{
    pIndexWriter_->indexDocument(pDoc);
    pIndexReader_->setDirty(true);
    return 1;
}

int Indexer::insertDocument(IndexerDocument* pDoc)
{
    pIndexWriter_->addDocument(pDoc);
    pIndexReader_->setDirty(true);
    return 1;
}

int Indexer::removeDocumentPhysically(IndexerDocument* pDoc)
{
    pIndexReader_->deleteDocumentPhysically(pDoc);
    pIndexReader_->setDirty(true);
    return 1;
}

int Indexer::removeDocument(docid_t docId)
{

    return 1;
}

int Indexer::removeCollection(collectionid_t colID)
{
    count_t count = 0;
    pIndexWriter_->removeCollection(colID,count);
    pIndexReader_->setDirty(true);
    return 1;
}

void Indexer::flush()
{
    pIndexWriter_->flush();
    pIndexReader_->setDirty(true);
    pBTreeIndexer_->flush();
}

size_t Indexer::getDistinctNumTermsByProperty(collectionid_t colID, std::string property)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    return pIndexReader_->getDistinctNumTerms(colID, fid);
}

///To be optimized: Using TermDocFreqs instead of TermPositions
bool Indexer::getDocsByTermInProperties(termid_t termID, collectionid_t colID, vector<string> properties, deque<docid_t>& docIds)
{
    if (properties.size() > 1)
    {
        ParallelTermPosition parallelTermPosition(colID, pIndexReader_,properties);
        if (!parallelTermPosition.isValid())
            return false;
        if (parallelTermPosition.seek(termID))
        {
            while(parallelTermPosition.next())
            {
                docIds.push_back(parallelTermPosition.doc());
            }
        }
        else
        {
            cout<<"can not find term"<<endl;
            return false;
        }
    }
    else
    {
        TermReader* pTermReader = pIndexReader_->getTermReader(colID);
        if (NULL == pTermReader)
            return false;

        Term term(properties[0].c_str(), termID);
        if (pTermReader->seek(&term))
        {
            TermPositions* pPositions = pTermReader->termPositions();

            while (pPositions->next())
            {
                docIds.push_back(pPositions->doc());
            }

            delete pPositions;
        }
        else
        {
            cout<<"can not find term"<<endl;
            delete pTermReader;
            return false;
        }
        delete pTermReader;
    }

    return true;
}

bool Indexer::getDocsByTermInProperties(termid_t termID, collectionid_t colID, vector<string> properties, deque<CommonItem>& commonSet)
{
    if (properties.size() > 1)
    {
        ParallelTermPosition parallelTermPosition(colID, pIndexReader_,properties);
        if (!parallelTermPosition.isValid())
            return false;
        if (parallelTermPosition.seek(termID))
        {
            while(parallelTermPosition.next())
            {
                CommonItem item;
                item.setDocID(parallelTermPosition.doc());
                item.setCollectionID(colID);
                std::map<string, PropertyItem> result;
                parallelTermPosition.getPositions(result);
                item.add(result);
                commonSet.push_back(item);
            }
            
        }
        else
        {
            cout<<"can not find term"<<endl;
            return false;
        }
    }
    else
    {
        TermReader* pTermReader = pIndexReader_->getTermReader(colID);
        if (NULL == pTermReader)
            return false;

        Term term(properties[0].c_str(), termID);
        if (pTermReader->seek(&term))
        {
            TermPositions* pPositions = pTermReader->termPositions();

            while (pPositions->next())
            {
                CommonItem item;
                item.setDocID(pPositions->doc());
                boost::shared_ptr<std::deque<unsigned int> > positions(new std::deque<unsigned int>);
                loc_t pos = pPositions->nextPosition();
                loc_t subpos = pPositions->nextPosition();
                while (pos != BAD_POSITION)
                {
                    positions->push_back(pos);
                    pos = pPositions->nextPosition();
                    subpos = pPositions->nextPosition();
                }
                item.addProperty(properties[0], positions, pPositions->freq());
                commonSet.push_back(item);
            }

            delete pPositions;
        }
        else
        {
            cout<<"can not find term"<<endl;
            delete pTermReader;
            return false;
        }
        delete pTermReader;
    }

    return true;
}

bool Indexer::getWordOffsetListOfQueryByDocumentProperty (const vector<termid_t>& queryTermIdList,  collectionid_t colId,  docid_t docId, string propertyName, deque<deque<pair<unsigned int, unsigned int> > >& wordOffsetListOfQuery )
{
    assert(indexingForward_ == true);
    fieldid_t fid = getPropertyIDByName(colId,propertyName);
    ForwardIndexReader* pForwardIndexReader = pIndexReader_->getForwardIndexReader();
    bool ret = pForwardIndexReader->getTermOffsetList(queryTermIdList, docId, fid, wordOffsetListOfQuery);
    delete pForwardIndexReader;
    return ret;
}

bool Indexer::getForwardIndexByDocumentProperty(collectionid_t colId, docid_t docId, string propertyName, ForwardIndex& forwardIndex)
{
    assert(indexingForward_ == true);
    fieldid_t fid = getPropertyIDByName(colId,propertyName);
    ForwardIndexReader* pForwardIndexReader = pIndexReader_->getForwardIndexReader();
    bool ret = pForwardIndexReader->getForwardIndexByDoc(docId, fid, forwardIndex);
    delete pForwardIndexReader;
    return ret;
}


bool Indexer::getTermFrequencyInCollectionByTermId( const vector<termid_t>& termIdList, const unsigned int collectionId, const vector<string>& propertyList, vector<unsigned int>& termFrequencyList )
{
    TermReader* pTermReader = pIndexReader_->getTermReader(collectionId);
    if (NULL == pTermReader)
        return false;

    size_t propertySize = propertyList.size();
    size_t termSize = termIdList.size();
    for (size_t i = 0; i < termSize; i++)
    {
        unsigned int ctf = 0;
        for (size_t j = 0; j < propertySize; j++)
        {
            Term term(propertyList[j].c_str(), termIdList[i]);
            if (pTermReader->seek(&term))
            {
                TermPositions* pPositions = pTermReader->termPositions();
                ctf+=pPositions->getCTF();
                delete pPositions;
            }
        }
        termFrequencyList.push_back(ctf);
    }
    delete pTermReader;

    return true;
}


bool Indexer::getDocsByPropertyValue(collectionid_t colID, string property, PropertyType value, BitVector&docs)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValue(colID, fid, value, docs);
    return true;
}

bool Indexer::getDocsByPropertyValueRange(collectionid_t colID, string property, PropertyType value1, PropertyType value2, BitVector&docs)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueBetween(colID, fid, value1, value2, docs);
    return true;
}

bool Indexer::getDocsByPropertyValueLessThan(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueLess(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueLessThanOrEqual(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueLessEqual(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueGreaterThan(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueGreat(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueGreaterThanOrEqual(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueGreatEqual(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueIn(collectionid_t colID, string property, vector<PropertyType> values, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueIn(colID, fid, values, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueNotIn(collectionid_t colID, string property, vector<PropertyType> values, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueNotIn(colID, fid, values, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueNotEqual(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueNotEqual(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueStart(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueStart(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueEnd(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueEnd(colID, fid, value, docList);
    return true;
}

bool Indexer::getDocsByPropertyValueSubString(collectionid_t colID, string property, PropertyType value, BitVector&docList)
{
    fieldid_t fid = getPropertyIDByName(colID,property);
    pBTreeIndexer_->getValueSubString(colID, fid, value, docList);
    return true;
}

void Indexer::optimizeIndex()
{
    IndexMerger* pIndexMerger = new OfflineIndexMerger(pDirectory_, pBarrelsInfo_->getBarrelCount());

    pIndexMerger->setParam(pConfigurationManager_->mergeStrategy_.param_.c_str());

    pIndexWriter_->mergeIndex(pIndexMerger);
}


