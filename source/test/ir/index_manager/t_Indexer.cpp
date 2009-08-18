#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

#include <iostream>
#include <fstream>

#include "ScdParser.h"

#include "XmlConfigParser.h"

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>


#include <ir/id_manager/IDManager.h>

#include <sf1v5/la-manager/LAManager.h>

using namespace std;
using namespace boost;

using namespace sf1v5;

using namespace izenelib::ir::idmanager;
using namespace izenelib::ir::indexmanager;

void ReportUsage(void)
{
    cout<<"\nUSAGE:./indexer menuoption\n";
    cout<<"./indexer 1  scdpath numberofdocuments =====================for insert collection.\n";
    cout<<"./indexer 2  ======================================= for search inverted files.\n";
    cout<<"	3 for remove a document.\n";
    cout<<"	4 for remove collection.\n";
    cout<<"	5 for term frequency testing.\n";
    cout<<"	7 for document frequency testing.\n";
    cout<<"	8 for b tree query test.\n";
    cout<<"	9 getDocsByPropertyValueLessThanOrEqual.\n";
    cout<<"	10 getDocsByPropertyValueNotIn.\n";
    cout<<"	11 getDocsByPropertyValueStart.\n";
    cout<<"	12 getDocsByPropertyValueEnd.\n";
    cout<<"	13 getDocsByPropertyValueSubString.\n";
    cout<<"	14 getProgressStatus.\n";
    cout<<"	19 for optimize index.\n";
    cout<<"	20 for memory index testing.\n";
    cout<<"	21 test getDocsByTermInProperties.\n";
}

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

void test_run_insert(Indexer* pIndexer, IndexManagerConfig&indexManagerConfig, string scdPath, int numberOfDocuments, unsigned int collectionId , string collectionName)
{
    IDManager idManager;
    LAManager laManager;


    ScdParser scdparser;
    if (scdparser.load(scdPath) == false)
    {
        cout<<"parse error"<<endl;
        return;
    }
/*	
    unsigned int n = scdparser.getCount();
    vector<vector<pair<wiselib::UString, wiselib::UString> > > documents(n);
    if (scdparser.getDocuments(documents, 0, numberOfDocuments) == false)
    {
        cout<<"get documents failes"<<endl;
        return;
    }
*/

    map<string, IndexerCollectionMeta> collectionList = indexManagerConfig.getCollectionMetaNameMap();
    IndexerCollectionMeta collectionMeta = collectionList[collectionName];
    std::set<IndexerPropertyConfig> collectionProperty = collectionMeta.getDocumentSchema();

    unsigned int n = 0;
    ScdParser::iterator iter_end = scdparser.end();
    for(ScdParser::iterator iter = scdparser.begin(); iter != iter_end; ++iter,++n)
    {
        if(n%1000 == 0)
            cout<<"indexed "<<n<<endl;
        SCDDocPtr doc = (*iter);

        wiselib::UString propertyName(" ", wiselib::UString::CP949);
        wiselib::UString propertyValueU(" ", wiselib::UString::CP949);
        IndexerDocument document;


        AnalysisInfo analysisInfo;
        analysisInfo.clear();
        analysisInfo.methodId_ = "la_allkor";

        //vector<pair<wiselib::UString, wiselib::UString> >::const_iterator p;
        SCDDoc::const_iterator p;
        for (p = doc->begin(); p != doc->end(); p++)
        {
        
            std::set<IndexerPropertyConfig>::iterator iter;
            for (iter = collectionProperty.begin(); iter != collectionProperty.end(); iter++)
            {
                if ( (wiselib::UString(iter->getName(), wiselib::UString::CP949)) == p->first)
                    break;
            }
            propertyValueU.clear();
            propertyValueU = p->second;
            if (p->first == wiselib::UString("DOCID", wiselib::UString::CP949))
            {
                unsigned int docId;
                idManager.getDocIdByDocName(collectionId, propertyValueU, docId);
                document.setDocId(docId,collectionId);
            }
            else
            {
                if (iter->isIndex())
                {
                    if (iter->isForward())
                    {
                        boost::shared_ptr<LAInput> laInput(new LAInput);
                        document.insertProperty(*iter, laInput);
                        LAInformationList rawTermList;      // la information of the base terms
                        LAInformationList laTermList;
                        if(laManager.extractTerms_full(propertyValueU, analysisInfo, rawTermList, laTermList)==false)
                        {
                            //propertyValueU.displayStringValue(wiselib::UString::CP949,cout);
                            //cout<<endl;
                        }
                        for (LAInformationList::iterator pp = laTermList.begin(); pp != laTermList.end(); pp++)
                        {
                            LAInputUnit u;
                            u.wordOffset_ = pp->wordOffset_;
                            u.byteOffset_ = 0;
                            idManager.getTermIdByTermString(pp->termString_, u.termId_);
                            document.add_to_property(u);
                        }
                    }
                    else
                    {
                        document.insertProperty(*iter, propertyValueU);
                    }
                }
            }
        }

        pIndexer->insertDocument(document);
    }

}


void test_run_query_singlethread(Indexer* pIndexer)
{
    try
    {
        vector<CommonItem> commonSet;
        vector<string> properties;
        properties.push_back("Content");
        //properties.push_back("title");
        time_t start = time(0);
        for (termid_t termid = 0; termid <10; termid++)
        {
            int ret = pIndexer->getDocsByTermInProperties(termid, 1, properties, commonSet);
            if (0 == ret)
            {
                cout<<"can not find "<<termid<<endl;
                continue;
            }
            /*			cout<<"termid: "<<termid<<endl;
            			for(vector<CommonItem>::iterator iter = commonSet.begin(); iter != commonSet.end(); ++iter)
            			{
            				cout<<"docid: "<<(*iter).docid<<" positions: ";
            				map<string,PositionPtr > commonItemProperty;
            				for(map<string,PositionPtr >::iterator it = (*iter).commonItemProperty.begin(); it != (*iter).commonItemProperty.end(); ++it)
            				{
            					cout<<" field: "<<it->first;
            					for(vector<loc_t>::iterator p = it->second->begin(); p != it->second->end(); ++p)
            						cout<<"  "<<*p<<" ";
            				}
            				cout<<endl;
            			}
            */

        }
        cout<<"time elapsed:"<<time(0)-start<<endl;

    }
    catch (IndexManagerException& e)
    {
        cout<<e.what()<<endl;
    }
}

int main(int argc, char** argv)
{
    map<string, uint32_t> collectionIdMapping;

    XmlConfigParser parser;
    parser.parseConfigFile( "sf1-index-manager.xml" );

    IndexManagerConfig indexManagerConfig = parser.getIndexManagerConfig();

    unsigned int collectionId = 1;
    collectionIdMapping.insert(pair<string, uint32_t>("coll1", collectionId));

    display(indexManagerConfig);

    if (argc < 2)
    {
        ReportUsage();
        return 0;
    }


    //Indexer* pIndexer = new Indexer(MANAGER_TYPE_DATAPROCESS);
    Indexer* pIndexer = new Indexer();

    pIndexer->setIndexManagerConfig(&indexManagerConfig, collectionIdMapping);


    int ch = atoi(argv[1]);
    switch (ch)
    {
    case 1:
        if (argc != 4)
        {
            ReportUsage();
            return 0;
        }
        test_run_insert(pIndexer,indexManagerConfig,argv[2],atoi(argv[3]),1,"coll1");
        break;
    case 2:
        test_run_query_singlethread(pIndexer);
        break;
    default:
        cout<<"invalid option.\n";
        delete pIndexer;
        return 0;
    }

    delete pIndexer;
    return 1;
}

