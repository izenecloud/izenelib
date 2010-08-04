#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

#include <iostream>
#include <fstream>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>

using namespace std;
using namespace boost;

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
#if 0
void  initMeta(IndexManagerConfig& config)
{
    map<string, IndexerCollectionMeta> collectionList = config.getCollectionMetaNameMap();
    map<string, IndexerCollectionMeta>::iterator  collectionList_iter;
    for (collectionList_iter = collectionList.begin(); collectionList_iter != collectionList.end(); collectionList_iter++)
    {
        std::cout << "IndexerCollectionMeta name: " << collectionList_iter->second.getName() << std::endl;
        const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>& schema =collectionList_iter->second.getDocumentSchema();

        std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> new_schema;
        unsigned int id = 1;
        for (std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator it = schema.begin(); it != schema.end(); it++ )
        {
            IndexerPropertyConfig propertyConfig = *it;
            propertyConfig.setPropertyId(id++);
            new_schema.insert(propertyConfig);
        }

        collectionList_iter->second.setDocumentSchema(new_schema);

        for (std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator it = collectionList_iter->second.getDocumentSchema().begin(); it != collectionList_iter->second.getDocumentSchema().end(); it++ )
        {
            std::cout << it->toString()<<endl;
        }
    }
    config.setCollectionMetaNameMap( collectionList );
}

bool makeForwardIndex_(
    const izenelib::util::UString& text,
    const AnalysisInfo& analysisInfo,
    boost::shared_ptr<ForwardIndex>& forwardIndex
)
{
    la::TermList termList;   
    if (laManager_->getTermList(text, analysisInfo, termList ) == false)
    {
        return false;
    }
    unsigned int id = 0;
    for (la::TermList::iterator p = termList.begin(); p != termList.end(); p++)
    {
        idManager_.getTermIdByTermString(p->text_, id);
        if (forwardIndex->insertTermOffset(id, p->wordOffset_) == false)
        {
            continue;
        }
    }

    forwardIndex->docLength_ = termList.size();
    return true;
}


bool prepareDocument_(const SCDDoc& doc,
                                    IndexerDocument* indexDocument,
                                    const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>& collectionProperty)
{
    sf1v5::docid_t docId = 0;
    string fieldStr;
    vector<unsigned int> sentenceOffsetList;
    AnalysisInfo analysisInfo;
    analysisInfo.analyzerId_ = "la_korall";
    analysisInfo.tokenizerNameList_.insert("tok_divide");
	
    vector<pair<izenelib::util::UString, izenelib::util::UString> >::const_iterator p;

    for (p = doc.begin(); p != doc.end(); p++)
    {
        bool extraProperty = false;
        std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator iter;
        p->first.convertString( fieldStr, izenelib::util::UString::UTF_8 );
        IndexerPropertyConfig temp;
        temp.setName(fieldStr);
        iter = collectionProperty.find( temp );

        IndexerPropertyConfig indexerPropertyConfig;
        if ( iter == collectionProperty.end())
            extraProperty = true;

        const izenelib::util::UString & propertyValueU = p->second; // preventing copy

        if(!extraProperty)
        {
            indexerPropertyConfig.setPropertyId(iter->getPropertyId());
            indexerPropertyConfig.setName(iter->getName());
            indexerPropertyConfig.setIsIndex(iter->isIndex());
            indexerPropertyConfig.setIsForward(iter->isForward());
            indexerPropertyConfig.setIsLAInput(false);
        }

        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8;
        if ( (p->first == izenelib::util::UString("DOCID", encoding) ) && (!extraProperty) )
        {
            bool ret = idManager_.getDocIdByDocName(propertyValueU, docId);
            if(ret)
                return false;

            indexDocument->setDocId(docId, 1);
        }
        else if ( (p->first == izenelib::util::UString("DATE", encoding) ) && (!extraProperty) )
        {
            /// format <DATE>20091009163011
            //int64_t time = sf1v5::Utilities::convertDate(propertyValueU);
            //indexerPropertyConfig.setIsIndex(true);
            //indexDocument->insertProperty(indexerPropertyConfig, time);
            continue;
        }
        else if (!extraProperty)
        {
            ///process for properties that requires forward index to be created
            if ( (iter->isIndex() == true) && (!extraProperty) )
            {
                boost::shared_ptr<ForwardIndex> propertyForwardIndex(new ForwardIndex);

                if (makeForwardIndex_(propertyValueU, analysisInfo, propertyForwardIndex) == false)
                {
                    cout << "makeForwardIndex() failed " << endl;
                    return false;
                }
                indexDocument->insertProperty(indexerPropertyConfig, propertyForwardIndex);
            }
            // insert property name and value for other properties that is not DOCID and neither required to be indexed
            else
            {
                //insert only if property that exist in collection configuration
                if (!extraProperty)
                {
                    //other extra properties that need not be in index manager
                    indexDocument->insertProperty(indexerPropertyConfig, propertyValueU);
                }
            }
        }
    }
    return true;
}

void test_run_insert(Indexer* pIndexer, IndexManagerConfig&indexManagerConfig, string scdPath, int numberOfDocuments, unsigned int collectionId , string collectionName)
{
    // get la manger config
    LAManagerConfig config;

    TokenizerConfigUnit tkUnit;
    tkUnit.id_ = "tok_divide";
    tkUnit.method_ = "divide";
    tkUnit.value_ = "@#$";
    config.addTokenizerConfig( tkUnit );

    LAConfigUnit laConfig;
    laConfig.id_       = "la_korall";
    laConfig.analysis_ = "korean";
    laConfig.mode_     = "all";
    laConfig.option_ = "R+H+S+";
    laConfig.specialChar_ = "#.";
    laConfig.dictionaryPath_ = "/home/newpeak/CodeBase/kma/knowledge";   // defined macro
    config.addLAConfig(laConfig);

    AnalysisInfo analysisInfo;
    analysisInfo.analyzerId_ = "la_korall";
    analysisInfo.tokenizerNameList_.insert("tok_divide");
    config.addAnalysisPair(analysisInfo);

    if(LAPool::getInstance()->init(config))
    {
        laManager_.reset(new LAManager());
    }
    else
    {
        std::cout << "Create Extractor fail.." << std::endl;
    }

    ScdParser scdparser(izenelib::util::UString::UTF_8);
    if (scdparser.load(scdPath) == false)
    {
        cout<<"parse error"<<endl;
        return;
    }

    map<string, IndexerCollectionMeta> collectionList = indexManagerConfig.getCollectionMetaNameMap();
    IndexerCollectionMeta collectionMeta = collectionList[collectionName];
    const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>& collectionProperty = collectionMeta.getDocumentSchema();

    unsigned int n = 0;
    ScdParser::iterator iter_end = scdparser.end();
    for(ScdParser::iterator iter = scdparser.begin(); iter != iter_end; ++iter,++n)
    {
        if(n%1000 == 0)
            cout<<"indexed "<<n<<endl;
        SCDDocPtr doc = (*iter);

        IndexerDocument* document = new IndexerDocument;

        bool ret = prepareDocument_(*doc, document, collectionProperty);
        if ( !ret )
            continue;

        pIndexer->insertDocument(document);
    }
    pIndexer->flush();
    idManager_.flush();
}


void test_run_query_singlethread(Indexer* pIndexer, izenelib::util::UString& term)
{
    try
    {
        deque<CommonItem> commonSet;
        vector<string> properties;
        properties.push_back("Content");
        //properties.push_back("Title");
        time_t start = time(0);
        uint32_t termid;
        idManager_.getTermIdByTermString(term, termid);
        int ret = pIndexer->getDocsByTermInProperties(termid, 1, properties, commonSet);
        if (0 == ret)
       	{
            cout<<"can not find "<<termid<<endl;
            return;
       	}
       cout<<"commonSet "<<commonSet.size()<<endl;

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
    collectionIdMapping.insert(pair<string, uint32_t>("EngWiki", collectionId));

    initMeta(indexManagerConfig);

    if (argc < 2)
    {
        ReportUsage();
        return 0;
    }

    Indexer* pIndexer = new Indexer();

    pIndexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);


    int ch = atoi(argv[1]);
    switch (ch)
    {
    case 1:
        if (argc != 4)
        {
            ReportUsage();
            return 0;
        }
        test_run_insert(pIndexer,indexManagerConfig,argv[2],atoi(argv[3]),1,"EngWiki");
        break;
    case 2:
	{
        std::string s(argv[2]);
        izenelib::util::UString term(s,izenelib::util::UString::UTF_8);
        test_run_query_singlethread(pIndexer,term);
    	}
        break;
    case 3:
        pIndexer->optimizeIndex();
	break;
    default:
        cout<<"invalid option.\n";
        delete pIndexer;
        return 0;
    }

    delete pIndexer;
    return 1;
}
#endif

BOOST_AUTO_TEST_SUITE( t_Indexer )

BOOST_AUTO_TEST_CASE(index)
{
    Indexer indexer;
}

BOOST_AUTO_TEST_SUITE_END()

