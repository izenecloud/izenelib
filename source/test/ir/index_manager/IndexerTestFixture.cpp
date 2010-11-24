/**
* @file       IndexerTestFixture.cpp
* @author     Jun
* @version    SF1 v5.0
* @brief Fixture implementation to test Indexer module.
*
*/

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include "IndexerTestFixture.h"
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexWriter.h>

using namespace std;
using namespace boost;

//#define LOG_DOC_OPERATION

const char* IndexerTestFixture::INDEX_FILE_DIR = "./index";
const unsigned int IndexerTestFixture::COLLECTION_ID = 1;
const char* IndexerTestFixture::INDEX_MODE_REALTIME = "realtime";
const char* IndexerTestFixture::INVERTED_FIELD = "content";

IndexerTestFixture::IndexerTestFixture()
    :indexer_(new Indexer)
    ,newDocNum_(0)
    ,docLenRand_(randEngine_, uniform_int<>(1, 1))
    ,termIDRand_(randEngine_, uniform_int<>(1, 1))
    ,docNumRand_(randEngine_, uniform_int<>(1, 1))
    ,skipToRand_(randEngine_, uniform_int<>(0, 1))
    ,maxDocID_(0)
{
    removeIndexFiles();
}

IndexerTestFixture::~IndexerTestFixture()
{
    VLOG(2) << "=> IndexerTestFixture::~IndexerTestFixture()";
    delete indexer_;
    VLOG(2) << "<= IndexerTestFixture::~IndexerTestFixture()";
}

void IndexerTestFixture::renewIndexer()
{
    delete indexer_;
    indexer_ = new Indexer;
}

void IndexerTestFixture::configTest(const IndexerTestConfig& testConfig)
{
    testConfig_ = testConfig;

    VLOG(2) << "=> IndexerTestFixture::configTest(), " << testConfig_.str();
    BOOST_TEST_MESSAGE(testConfig_.str());

    // set random generators range
    newDocNum_ = testConfig_.docNum_;
    docLenRand_.distribution() = uniform_int<>(1, 10 * newDocNum_);
    termIDRand_.distribution() = uniform_int<>(1, 100 * newDocNum_);

    IndexManagerConfig indexManagerConfig;
    boost::filesystem::path path(INDEX_FILE_DIR);

    indexManagerConfig.indexStrategy_.indexLocation_ = path.string();
    indexManagerConfig.indexStrategy_.indexMode_ = testConfig_.indexMode_;
    indexManagerConfig.indexStrategy_.memory_ = 30000000;
    indexManagerConfig.indexStrategy_.indexDocLength_ = true;
    indexManagerConfig.indexStrategy_.skipInterval_ = 0;
    indexManagerConfig.indexStrategy_.maxSkipLevel_ = 0;
    indexManagerConfig.mergeStrategy_.param_ = testConfig_.isMerge_ ? "default" : "no";
    indexManagerConfig.storeStrategy_.param_ = "mmap";

    std::vector<std::string> propertyList;
    propertyList.push_back(INVERTED_FIELD);
    propertyList.push_back("date");
    propertyList.push_back("provider");

    std::sort(propertyList.begin(),propertyList.end());
    IndexerCollectionMeta indexCollectionMeta;
    indexCollectionMeta.setName("testcoll");
    propertyMap_.clear();
    for (std::size_t i=0;i<propertyList.size();i++)
    {
        IndexerPropertyConfig indexerPropertyConfig(1+i, propertyList[i], true, true);
        propertyMap_[propertyList[i]] = 1+i;
        if(propertyList[i] != INVERTED_FIELD)
        {
            indexerPropertyConfig.setIsAnalyzed(false);
            indexerPropertyConfig.setIsFilter(true);
        }
        indexCollectionMeta.addPropertyConfig(indexerPropertyConfig);
    }
    indexManagerConfig.addCollectionMeta(indexCollectionMeta);

    map<string, unsigned int> collectionIdMapping;
    collectionIdMapping.insert(pair<string, unsigned int>("testcoll", COLLECTION_ID));

    indexer_->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
    VLOG(2) << "<= IndexerTestFixture::configTest()";
}

void IndexerTestFixture::createDocument()
{
    VLOG(2) << "=> IndexerTestFixture::createDocument()";

    docid_t docID = maxDocID_ + 1;
    for(unsigned int i = 1; i <= newDocNum_; i++, ++docID)
    {
#ifdef LOG_DOC_OPERATION
        BOOST_TEST_MESSAGE("create doc id: " << docID);
#endif
        IndexerDocument document;
        prepareDocument(document, docID);
        BOOST_CHECK_EQUAL(indexer_->insertDocument(document), 1);
    }

    indexer_->flush();
    VLOG(2) << "<= IndexerTestFixture::createDocument()";
}

int IndexerTestFixture::randDocNum()
{
    if(static_cast<unsigned int>(docNumRand_.distribution().max()) != mapDocIdLen_.size())
        docNumRand_.distribution() = uniform_int<>(1, mapDocIdLen_.size());

    return docNumRand_();
}

void IndexerTestFixture::updateDocument()
{
    VLOG(2) << "=> IndexerTestFixture::updateDocument()";

    if(mapDocIdLen_.empty())
        return;

    const int updateNum = randDocNum();
    list<docid_t> removeDocList;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("start updating " << updateNum << " docs...");
#endif
    docid_t newDocID = maxDocID_ + 1;
    for(int i=0; i<updateNum; ++i, ++newDocID)
    {
        const int updatePos = randDocNum() - 1; // position starts from 0
        DocIdLenMapT::iterator it = mapDocIdLen_.begin();
        for(int j=0; j<updatePos; ++j)
            ++it;

#ifdef LOG_DOC_OPERATION
        BOOST_TEST_MESSAGE("update doc id: " << it->first << " to new doc id: " << newDocID);
#endif
        IndexerDocument document;
        document.setId(it->first);
        prepareDocument(document, newDocID);
        BOOST_CHECK_EQUAL(indexer_->updateDocument(document), 1);

        removeDocList.push_back(it->first);
        mapDocIdLen_.erase(it);
    }

    removeDocList.sort();
    removeFixtureDocs(removeDocList);

    indexer_->flush();
    VLOG(2) << "<= IndexerTestFixture::updateDocument()";
}

void IndexerTestFixture::removeDocument()
{
    VLOG(2) << "=> IndexerTestFixture::removeDocument()";

    if(mapDocIdLen_.empty())
        return;

    const int removeNum = randDocNum();
    list<docid_t> removeDocList;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("start removing " << removeNum << " docs...");
#endif
    for(int i=0; i<removeNum; ++i)
    {
        const int removePos = randDocNum() - 1; // position starts from 0
        DocIdLenMapT::iterator it = mapDocIdLen_.begin();
        for(int j=0; j<removePos; ++j)
            ++it;

#ifdef LOG_DOC_OPERATION
        BOOST_TEST_MESSAGE("remove doc id: " << it->first);
#endif
        indexer_->removeDocument(COLLECTION_ID, it->first);
        removeDocList.push_back(it->first);
        mapDocIdLen_.erase(it);
    }

    removeDocList.sort();
    removeFixtureDocs(removeDocList);

    // remove doc id exceed the range
    docid_t overId = maxDocID_ + 1;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("remove exceed doc id: " << overId);
#endif
    indexer_->removeDocument(COLLECTION_ID, overId);

    indexer_->flush();
    VLOG(2) << "<= IndexerTestFixture::removeDocument()";
}

void IndexerTestFixture::removeIndexFiles()
{
    boost::filesystem::path indexPath(INDEX_FILE_DIR);
    boost::filesystem::remove_all(indexPath);
}

void IndexerTestFixture::prepareDocument(IndexerDocument& document, unsigned int docId, bool filter)
{
    DTermIdMapT docTermIdMap;

    document.setDocId(docId, COLLECTION_ID);

    IndexerPropertyConfig propertyConfig(propertyMap_[INVERTED_FIELD],INVERTED_FIELD,true,true);

    boost::shared_ptr<LAInput> laInput(new LAInput);
    document.insertProperty(propertyConfig, laInput);

    const unsigned int docLen = docLenRand_();
    mapDocIdLen_[docId] = docLen;
    maxDocID_ = docId;
    for(unsigned int i = 0; i < docLen; ++i)
    {
        LAInputUnit unit;
        unit.docId_ = docId;
        unit.termid_ = termIDRand_();
        unit.wordOffset_ = i;
        document.add_to_property(unit);

        docTermIdMap[unit.termid_].push_back(i);
    }

    addFixtureDoc(docTermIdMap);

    if(filter)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        propertyConfig.setPropertyId(propertyMap_["date"]);
        propertyConfig.setName("date");
        propertyConfig.setIsAnalyzed(false);
        propertyConfig.setIsFilter(true);

        struct tm atm;
        ptime now = second_clock::local_time();
        atm = to_tm(now);
        int64_t time = mktime(&atm);

        document.insertProperty(propertyConfig, time);
    }
}
