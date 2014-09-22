/**
* @file       t_InMemoryBarrel.cpp
* @author     Kevin Lin
* @brief confirm features of InMemoryBarrel:
* - Barrels.xml consistency
* - Reload 
*/
/*  
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/AbsTermIterator.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/store/FSDirectory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/utility/XML.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/id_manager/IDManager.h>
#include <util/ustring/UString.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm> // sort
#include <utility> // pair

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <boost/test/unit_test.hpp>

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

#include <boost/scoped_ptr.hpp>

#include "IndexerTestFixture.h"
#include "t_BarrelsInfo.h"

using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;


namespace t_InMemoryBarrel
{
namespace bfs = boost::filesystem;

std::map<std::string, unsigned int> propertyMap;
boost::uniform_int<> distribution(0xFFFF, 0x7FFFFFFF) ;
boost::mt19937 engine ;
boost::variate_generator<boost::random::mt19937, uniform_int<> > myrandom(engine, distribution);

typedef struct
{
    ///barrel name
    string barrelName;
    ///map of document base id of different collections in a certain barrel
    map<collectionid_t,docid_t> baseDocIDMap;
    ///document count of this barrel
    count_t nNumDocs;
    ///whether this barrel contains updated documents
    bool isUpdate;
    ///whether this barrel is in-memory barrel 
    bool inMemoryBarrel;
    ///max doc of this barrel
    docid_t maxDocId;

    bool searchable;
}BarrelInfo;

typedef struct
{
    string version;

    int32_t nBarrelCounter; ///barrel counter

    vector<BarrelInfo> barrelInfos;

    docid_t maxDoc;

}BarrelsInfo;


void read(const char* name, BarrelsInfo& barrelsInfo)
{
    Directory* pDirectory = new FSDirectory("index");
    if (pDirectory->fileExists(name))
    {
        XMLElement* pDatabase = NULL;
        try
        {
            IndexInput* pIndexInput = pDirectory->openInput(name);
            string str;
            str.resize((size_t)pIndexInput->length());
            pIndexInput->read((char*)str.c_str(),str.length());
            delete pIndexInput;

            pDatabase = XMLElement::fromString(str.c_str(),true);
            if (!pDatabase) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///get <version></version> element
            XMLElement* pItem = pDatabase->getElementByName("version");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            barrelsInfo.version = pItem->getValue();

            ///get <barrel_counter></barrel_counter> element
            pItem = pDatabase->getElementByName("barrel_counter");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            barrelsInfo.nBarrelCounter = atoi(pItem->getValue().c_str());

            ///get <barrel_count></barrel_count> element
            pItem = pDatabase->getElementByName("barrel_count");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///get <maxdoc></maxdoc> element
            pItem = pDatabase->getElementByName("maxdoc");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            barrelsInfo.maxDoc = atoi(pItem->getValue().c_str());

            XMLElement* pBarrelsItem = pDatabase->getElementByName("barrels");
            if (!pBarrelsItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///read barrel information
            BarrelInfo* pBarrelInfo = NULL;
            ElementIterator  iter = pBarrelsItem->getElementIterator();
            XMLElement*	pBarrelItem;
            while (pBarrelsItem->hasNextElement(iter))
            {
                pBarrelItem = pBarrelsItem->getNextElement(iter);
                BarrelInfo barrelInfo;
                pBarrelInfo = &barrelInfo;

                ///get <name></name> element
                pItem = pBarrelItem->getElementByName("name");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->barrelName = pItem->getValue();

                ///get <doc_begin></doc_begin> element
                pItem = pBarrelItem->getElementByName("doc_begin");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

                vector<string>ps = izenelib::ir::indexmanager::split(pItem->getValue(),",");
                for (vector<string>::iterator it = ps.begin(); it != ps.end(); it++)
                {
                    if ((*it).empty())
                        continue;
                    vector<string>pair = izenelib::ir::indexmanager::split((*it),":");
                    assert(pair.size() == 2);
                    pBarrelInfo->baseDocIDMap.insert(make_pair(atoi(pair[0].c_str()),atoi(pair[1].c_str())));
                }

                ///get <doc_count></doc_count> element
                pItem = pBarrelItem->getElementByName("doc_count");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->nNumDocs = atoi(pItem->getValue().c_str());

                ///get <max_doc></max_doc> element
                pItem = pBarrelItem->getElementByName("max_doc");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->maxDocId = atoi(pItem->getValue().c_str());

                ///get <is_update></is_update> element
                pItem = pBarrelItem->getElementByName("is_update");
                if (!pItem) pBarrelInfo->isUpdate = false;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->isUpdate = true;
                    else
                        pBarrelInfo->isUpdate = false;
                }

                ///get <searchable></searchable> element
                pItem = pBarrelItem->getElementByName("searchable");
                if (!pItem) pBarrelInfo->searchable = true;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->searchable = true;
                    else
                        pBarrelInfo->searchable = false;
                }
                
                ///get <is_in_memory_barrel></is_in_memory_barrel> element
                pItem = pBarrelItem->getElementByName("is_in_memory_barrel");
                if (!pItem) pBarrelInfo->inMemoryBarrel = false;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->inMemoryBarrel = true;
                    else
                        pBarrelInfo->inMemoryBarrel = false;
                }


                ///get <compress></compress> element
                pItem = pBarrelItem->getElementByName("compress");
                
                barrelsInfo.barrelInfos.push_back(*pBarrelInfo);
            }
            delete pDatabase;
            pDatabase = NULL;
            delete pDirectory;
            pDirectory = NULL;
        }
        catch (IndexManagerException& e)
        {
            if (pDatabase)
                delete pDatabase;
            SF1V5_RETHROW(e);
        }
        catch (...)
        {
            SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
        }
    }
}

int32_t getDocCount(BarrelsInfo& barrelsInfo)
{
    int32_t docCount = 0;
    vector<BarrelInfo>::iterator it = barrelsInfo.barrelInfos.begin();
    for (; it != barrelsInfo.barrelInfos.end(); it++)
    {
        docCount += (*it).nNumDocs;
    }
    return docCount;
}

void checkBarrelsAfterOptimize(BarrelsInfo& lv, BarrelsInfo& rv)
{
        BOOST_CHECK_EQUAL(getDocCount(lv),getDocCount(rv));
}

void checkBarrelsAfterReload(BarrelsInfo& lv, BarrelsInfo& rv)
{
    BOOST_CHECK_EQUAL(lv.barrelInfos.size(), rv.barrelInfos.size()); 
    vector<BarrelInfo>::iterator itLv = lv.barrelInfos.begin();
    for (; itLv != lv.barrelInfos.end(); itLv++)
    {
        vector<BarrelInfo>::iterator itRv = rv.barrelInfos.begin();
        for (; itRv != rv.barrelInfos.end(); itRv++)
        {
            if ((*itLv).barrelName == (*itRv).barrelName)
                break;
        }
        BOOST_CHECK(itRv != rv.barrelInfos.end());
        BOOST_CHECK_EQUAL((*itLv).nNumDocs, (*itRv).nNumDocs);
        BOOST_CHECK_EQUAL((*itLv).maxDocId, (*itRv).maxDocId);
        BOOST_CHECK_EQUAL((*itLv).inMemoryBarrel, (*itRv).inMemoryBarrel);
    }
}



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

}
BOOST_AUTO_TEST_SUITE( t_InMemoryBarrel )

BOOST_AUTO_TEST_CASE(optimize)
{
    t_InMemoryBarrel::clearIndices();

    boost::shared_ptr<Indexer> indexer;
    t_InMemoryBarrel::initIndexer(indexer);

    for(unsigned int i = 1; i < 100; i++)
    {
        IndexerDocument document;
        t_InMemoryBarrel::prepareDocument(i, document);
        indexer->insertDocument(document);
    }

    indexer->flush();

    t_InMemoryBarrel::initIndexer(indexer, "realtime");
    
    for(unsigned int i = 1; i < 62; i++)
    {
        IndexerDocument document;
        t_InMemoryBarrel::prepareDocument(i, document);
        indexer->insertDocument(document);
    }

    indexer->flush(false);

    t_InMemoryBarrel::BarrelsInfo beforeOpt;
    t_InMemoryBarrel::read("barrels", beforeOpt);

    indexer->optimizeIndex();

    t_InMemoryBarrel::BarrelsInfo afterOpt;
    t_InMemoryBarrel::read("barrels", afterOpt);
    
    t_InMemoryBarrel::checkBarrelsAfterOptimize(beforeOpt, afterOpt);
}

BOOST_AUTO_TEST_CASE(reload)
{
    t_InMemoryBarrel::clearIndices();
    
    {
        boost::shared_ptr<Indexer> indexer;
        t_InMemoryBarrel::initIndexer(indexer);

        for(unsigned int i = 1; i < 100; i++)
        {
            IndexerDocument document;
            t_InMemoryBarrel::prepareDocument(i, document);
            indexer->insertDocument(document);
        }

        indexer->flush();

        t_InMemoryBarrel::initIndexer(indexer, "realtime");
    
        for(unsigned int i = 1; i < 62; i++)
        {
            IndexerDocument document;
            t_InMemoryBarrel::prepareDocument(i, document);
            indexer->insertDocument(document);
        }

        indexer->flush(false);
    }
    
    t_InMemoryBarrel::BarrelsInfo beforeReload;
    t_InMemoryBarrel::read("barrels", beforeReload);

    {
        boost::shared_ptr<Indexer> indexer;
        t_InMemoryBarrel::initIndexer(indexer);
        indexer->flush(false);
    }
    
    t_InMemoryBarrel::BarrelsInfo afterReload;
    t_InMemoryBarrel::read("barrels", afterReload);

    t_InMemoryBarrel::checkBarrelsAfterReload(beforeReload, afterReload);
}

BOOST_AUTO_TEST_SUITE_END()


	*/
