/*  #include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>



using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

namespace bfs = boost::filesystem;

std::map<std::string, unsigned int> propertyMap;
boost::uniform_int<> distribution(0xFFFF, 0x7FFFFFFF) ;
boost::mt19937 engine ;
boost::variate_generator<mt19937, uniform_int<> > myrandom (engine, distribution);

void clearIndices()
{
    bfs::path indexPath(bfs::path(".") /"index");
    boost::filesystem::remove_all(indexPath);
}

void initIndexer(boost::shared_ptr<Indexer>& indexer,
                                    std::string indexmode = "default",
                                    int skipinterval = 0,
                                    int skiplevel = 0
                                    )
{
    indexer.reset(new Indexer);
    IndexManagerConfig indexManagerConfig;

    bfs::path path(bfs::path(".") /"index");

    indexManagerConfig.indexStrategy_.indexLocation_ = path.string();
    indexManagerConfig.indexStrategy_.indexMode_ = indexmode;
    indexManagerConfig.indexStrategy_.memory_ = 30000000;
    indexManagerConfig.indexStrategy_.indexDocLength_ = true;
    indexManagerConfig.indexStrategy_.skipInterval_ = skipinterval;
    indexManagerConfig.indexStrategy_.maxSkipLevel_ = skiplevel;
    indexManagerConfig.mergeStrategy_.param_ = "default";
    indexManagerConfig.storeStrategy_.param_ = "mmap";
    unsigned int collectionId = 1;
    std::map<std::string, unsigned int> collectionIdMapping;
    collectionIdMapping.insert(std::pair<std::string, unsigned int>("testcoll", collectionId));

    std::vector<std::string> propertyList;
    propertyList.push_back("content");
    propertyList.push_back("date");
    propertyList.push_back("provider");

    std::sort(propertyList.begin(),propertyList.end());
    IndexerCollectionMeta indexCollectionMeta;
    indexCollectionMeta.setName("testcoll");
    for (std::size_t i=0;i<propertyList.size();i++)
    {
        IndexerPropertyConfig indexerPropertyConfig(1+i, propertyList[i], true, true);
        propertyMap[propertyList[i]] = 1+i;
        if(propertyList[i] != "content")
        {
            indexerPropertyConfig.setIsAnalyzed(false);
            indexerPropertyConfig.setIsFilter(true);
        }
        indexCollectionMeta.addPropertyConfig(indexerPropertyConfig);
    }
    indexManagerConfig.addCollectionMeta(indexCollectionMeta);

    indexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
}

void prepareDocument(unsigned int docId, IndexerDocument& document, bool filter = true)
{
    document.setOldId(docId);
    document.setDocId(docId, 1);

    IndexerPropertyConfig propertyConfig(propertyMap["content"],"content",true,true);

    boost::shared_ptr<LAInput> laInput(new LAInput);
    document.insertProperty(propertyConfig, laInput);

    for(unsigned int i = 0; i < 1000; ++i)
    {
        LAInputUnit unit;
        unit.docId_ = docId;
        unit.termid_ = myrandom() % 500;
        unit.wordOffset_ = i;
        document.add_to_property(unit);
    }

    if(filter)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        propertyConfig.setPropertyId(propertyMap["date"]);
        propertyConfig.setName("date");
        propertyConfig.setIsAnalyzed(false);
        propertyConfig.setIsFilter(true);

        struct tm atm;
        ptime now = second_clock::local_time();
        atm = to_tm(now);
        int64_t time = mktime(&atm);

        document.insertProperty(propertyConfig, time);

        propertyConfig.setPropertyId(propertyMap["provider"]);
        propertyConfig.setName("provider");


    }
}

BOOST_AUTO_TEST_SUITE( t_Indexer )

BOOST_AUTO_TEST_CASE(index)
{
    clearIndices();

    boost::shared_ptr<Indexer> indexer;
    initIndexer(indexer);

    for(unsigned int i = 1; i < 100; i++)
    {
        IndexerDocument document;
        prepareDocument(i, document);
        indexer->insertDocument(document);
    }

    indexer->flush();
cout<<"Created!"<<endl;
}

BOOST_AUTO_TEST_CASE(update)
{
    clearIndices();

    boost::shared_ptr<Indexer> indexer;
    initIndexer(indexer);

    for(unsigned int i = 1; i < 1000; i++)
    {
        IndexerDocument document;
        prepareDocument(i, document);
        indexer->insertDocument(document);
    }
    for(unsigned int i = 1001; i < 2000; i++)
    {
        IndexerDocument document;
        prepareDocument(i, document);
        document.setOldId(i - 1000);
        indexer->updateDocument(document);
    }
    indexer->flush();
}

BOOST_AUTO_TEST_CASE(remove)
{
    clearIndices();

    boost::shared_ptr<Indexer> indexer;
    initIndexer(indexer);

    for(unsigned int i = 1; i < 1000; i++)
    {
        IndexerDocument document;
        prepareDocument(i, document);
        indexer->insertDocument(document);
    }
    indexer->flush();
    for(unsigned int i = 1; i < 1000; i++)
    {
        indexer->removeDocument(1, i);
    }
}

BOOST_AUTO_TEST_SUITE_END()

	*/
