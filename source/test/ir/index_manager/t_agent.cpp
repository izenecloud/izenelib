#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

#include <iostream>
#include <fstream>


#include "XmlConfigParser.h"

#include <ir/index_manager/index/Indexer.h>

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;


void  display(const IndexManagerConfig& config)
{
    map<string, IndexerCollectionMeta> collectionList;
    collectionList = config.getCollectionMetaNameMap();
    map<string, IndexerCollectionMeta>::iterator  collectionList_iter;
    for (collectionList_iter = collectionList.begin(); collectionList_iter != collectionList.end(); collectionList_iter++)
    {
        std::cout << "IndexerCollectionMeta name: " << collectionList_iter->second.getName() << std::endl;
        IndexerCollectionMeta collectionMeta(collectionList_iter->second);
        std::cout << "IndexerCollectionMeta name: " << collectionMeta.getName() << std::endl;
        std::set<IndexerPropertyConfig> shopMallDocSchema = collectionMeta.getDocumentSchema();
        for (std::set<IndexerPropertyConfig>::const_iterator it = shopMallDocSchema.begin(); it != shopMallDocSchema.end(); it++ )
        {
            std::cout << it->toString();
        }
    }


    //setting up <indexstrategy>
    std::cout << "indexLocation: " << config.indexStrategy_.indexLocation_ << std::endl;
    std::cout << "config.indexStrategy_.accessMode_ =   " << config.indexStrategy_.accessMode_ << std::endl;
    std::cout << "indexStrategy_.memory_ = " << config.indexStrategy_.memory_ << std::endl;
    std::cout << "indexStrategy_.maxIndexTerms_ = " << config.indexStrategy_.maxIndexTerms_ << std::endl;
    std::cout << "indexStrategy_.cacheDocs_ = " << config.indexStrategy_.cacheDocs_ << std::endl;

    //setting up <mergestrategy>
    std::cout << "mergeStrategy_.strategy_ =  " << config.mergeStrategy_.strategy_  << std::endl;
    std::cout << "mergeStrategy_.param_ = " << config.mergeStrategy_.param_ << std::endl;

    //setting up <storestrategy>
    std::cout << "storeStrategy_.param_ = "  << config.storeStrategy_.param_ << std::endl;

    //setting up <distributestrategy>
    std::cout << "distributeStrategy_.batchport_ = " << config.distributeStrategy_.batchport_<< std::endl;
    std::cout << "distributeStrategy_.rpcport_ = " << config.distributeStrategy_.rpcport_ << std::endl;; //PARSED IN INDEX-MANAGER
    std::cout << "distributeStrategy_.iplist_ = " << config.distributeStrategy_.iplist_ << std::endl;; //PARSED IN INDEX-MANAGER

    //setting up <distributestrategy>
    std::cout << "advance_.MMS_ = " << config.advance_.MMS_  << std::endl;
    std::cout << "uptightAlloc_.memSize_ = " << config.advance_.uptightAlloc_.memSize_ << std::endl;
    std::cout << "uptightAlloc_.chunkSize_ =  " << config.advance_.uptightAlloc_.chunkSize_ << std::endl;

    std::cout << "logLevel_ = " << config.logLevel_ << std::endl;
}


int main(int argc, char** argv)
{
    map<string, uint32_t> collectionIdMapping;

    XmlConfigParser parser;
    parser.parseConfigFile( "sf1-index-manager.xml" );

    IndexManagerConfig indexManagerConfig = parser.getIndexManagerConfig();
    collectionIdMapping.insert(pair<string, uint32_t>("coll1", 1));

    Indexer indexer(MANAGER_TYPE_INDEXPROCESS);
    display(indexManagerConfig);
    indexer.setIndexManagerConfig(&indexManagerConfig, collectionIdMapping);

    return 1;
}


