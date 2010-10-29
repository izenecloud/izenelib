#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/AbsTermReader.h>

#define LOG_DOC_OPERATION
//#define LOG_CHECK_OPERATION
//#define LOG_TERM_ID

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

namespace
{
const char* INDEX_FILE_DIR = "./index";

const unsigned int COLLECTION_ID = 1;

const char* INDEX_MODE_REALTIME = "realtime";

const int TEST_DOC_NUM = 3000;

const int TEST_BARREL_NUM = 10;
};

class IndexerTest
{
private:
    std::map<std::string, unsigned int> propertyMap_;

    Indexer* indexer_;

    unsigned int newDocNum_; ///< the max number of documents created in createDocument()
    bool isDocNumRand_; ///< true for doc number range [1, rand(newDocNum_)], false for range [1, newDocNum_]
    boost::mt19937 randEngine_;
    boost::variate_generator<mt19937, uniform_int<> > docLenRand_;
    boost::variate_generator<mt19937, uniform_int<> > termIDRand_;
    boost::variate_generator<mt19937, uniform_int<> > docIDSkipRand_;
    boost::variate_generator<mt19937, uniform_int<> > docNumRand_;

    typedef map<docid_t, size_t> DocIdLenMapT;
    DocIdLenMapT mapDocIdLen_;

    /**
     * as the max doc id in indexer is the max doc id ever indexed,
     * no matter whether that doc is deleted,
     * we have to maintain that max doc id here.
     */
    docid_t maxDocID_;

public:
    IndexerTest(unsigned int docNum, bool isDocNumRand = false, unsigned int docIDSkipMax = 1)
        : indexer_(0)
          ,newDocNum_(docNum)
          ,isDocNumRand_(isDocNumRand)
          ,docLenRand_(randEngine_, uniform_int<>(1, 40*newDocNum_))
          ,termIDRand_(randEngine_, uniform_int<>(1, 400*newDocNum_))
          ,docIDSkipRand_(randEngine_, uniform_int<>(1, docIDSkipMax))
          ,docNumRand_(randEngine_, uniform_int<>(1, newDocNum_))
          ,maxDocID_(0)
    {}

    void setUp(bool isRemoveIndexFiles = true, const string& indexmode = "default") {
        if(isRemoveIndexFiles)
            removeIndexFiles();

        initIndexer(indexmode);
    }

    void tearDown() {
        delete indexer_;
    }

    bool isDocEmpty() const {
        return mapDocIdLen_.empty();
    }

    /**
     * Get a randum number between [1, mapDocIdLen_.size()].
     * @return if mapDocIdLen_ is empty, it returns 0 instead.
     */
    int randDocNum() const {
        if(mapDocIdLen_.empty())
            return 0;

        boost::variate_generator<mt19937, uniform_int<> > numRand(randEngine_, uniform_int<>(1, mapDocIdLen_.size()));
        return numRand();
    }

    /** Only create \e newDocNum_ documents. */
    void createDocument() {
        DVLOG(2) << "=> IndexerTest::createDocument()";

        docid_t docID = maxDocID_;
        for(unsigned int i = 1; i <= (isDocNumRand_ ? docNumRand_() : newDocNum_); i++)
        {
            docID += docIDSkipRand_();
#ifdef LOG_DOC_OPERATION
            cout << "create doc id: " << docID << endl;
#endif
            IndexerDocument document;
            prepareDocument(document, docID);
            BOOST_CHECK_EQUAL(indexer_->insertDocument(document), 1);
        }

        indexer_->flush();
        DVLOG(2) << "<= IndexerTest::createDocument()";
    }

    /** Update all exsting documents. */
    void updateDocument() {
        DVLOG(2) << "=> IndexerTest::updateDocument()";

        docid_t newDocID = maxDocID_;
        DocIdLenMapT oldMap;
        oldMap.swap(mapDocIdLen_);
        for(DocIdLenMapT::const_iterator it=oldMap.begin(); it!=oldMap.end(); ++it)
        {
            newDocID += docIDSkipRand_();
#ifdef LOG_DOC_OPERATION
            cout << "update doc id: " << it->first << " to new doc id: " << newDocID << endl;
#endif
            IndexerDocument document;
            document.setId(it->first);
            prepareDocument(document, newDocID);
            BOOST_CHECK_EQUAL(indexer_->updateDocument(document), 1);
        }

        indexer_->flush();
        DVLOG(2) << "<= IndexerTest::updateDocument()";
    }

    /**
     * Remove random number of documents, and also remove documents exceed max doc id.
     */
    void removeDocument() {
        DVLOG(2) << "=> IndexerTest::removeDocument()";

        if(mapDocIdLen_.empty())
            return;

        const int removeNum = randDocNum(); // range [1, size]
#ifdef LOG_DOC_OPERATION
        cout << "start removing " << removeNum << " docs..." << endl;
#endif
        for(int i=0; i<removeNum; ++i)
        {
            const int removePos = randDocNum() - 1; // range [0, size-1]
            DocIdLenMapT::iterator it = mapDocIdLen_.begin();
            for(int j=0; j<removePos; ++j)
                ++it;

#ifdef LOG_DOC_OPERATION
            cout << "remove doc id: " << it->first << endl;
#endif
            indexer_->removeDocument(COLLECTION_ID, it->first);
            mapDocIdLen_.erase(it);
        }

        // remove doc id exceed the range
        docid_t overId = maxDocID_ + 1;
#ifdef LOG_DOC_OPERATION
        cout << "remove exceed doc id: " << overId << endl;
#endif
        indexer_->removeDocument(COLLECTION_ID, overId);

        indexer_->flush();
        DVLOG(2) << "<= IndexerTest::removeDocument()";
    }

    void checkDocLength() {
        DVLOG(2) << "=> IndexerTest::checkDocLength()";

        IndexReader* pIndexReader = indexer_->getIndexReader();

        BOOST_CHECK_EQUAL(pIndexReader->numDocs(), mapDocIdLen_.size());
        BOOST_CHECK_EQUAL(pIndexReader->maxDoc(), maxDocID_);

        DocIdLenMapT::const_iterator mapIt = mapDocIdLen_.begin();
        DocIdLenMapT::const_iterator mapItEnd = mapDocIdLen_.end();

        for(; mapIt!=mapItEnd; ++mapIt)
        {
#ifdef LOG_CHECK_OPERATION
            cout << "check: " << mapIt->first << endl;
#endif
            BOOST_CHECK_EQUAL(pIndexReader->docLength(mapIt->first, indexer_->getPropertyIDByName(COLLECTION_ID, "content")), mapIt->second);
        }

        //TermReader* pTermReader = pIndexReader->getTermReader(COLLECTION_ID);
        //delete pTermReader;

        DVLOG(2) << "<= IndexerTest::checkDocLength()";
    }

    /**
     * Create barrels and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void checkBarrel(int barrelNum) {
        DVLOG(2) << "=> IndexerTest::checkBarrel()";

        for(int i=0; i<barrelNum; ++i)
            createDocument(); // create barrel i

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);

        if(indexer_->getIndexManagerConfig()->indexStrategy_.indexMode_ != INDEX_MODE_REALTIME)
        {
            BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), barrelNum);
            BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

            for(int i=0; i<barrelNum; ++i)
            {
                BarrelInfo* pBarrelInfo = (*pBarrelsInfo)[i];
                BOOST_CHECK_EQUAL(pBarrelInfo->getDocCount(), newDocNum_);
                BOOST_CHECK_EQUAL(pBarrelInfo->getBaseDocID(), i*newDocNum_+1);
                BOOST_CHECK_EQUAL(pBarrelInfo->getMaxDocID(), (i+1)*newDocNum_);
            }
        }

        DVLOG(2) << "<= IndexerTest::checkBarrel()";
    }

    /**
     * Create barrels, optimize barrels (merge them into one), and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void optimizeBarrel(int barrelNum) {
        DVLOG(2) << "=> IndexerTest::optimizeBarrel()";

        for(int i=0; i<barrelNum; ++i)
            createDocument(); // create barrel i

        indexer_->optimizeIndex();

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

        DVLOG(2) << "<= IndexerTest::optimizeBarrel()";
    }

    /**
     * Check optimize barrels result.
     */
    void checkOptimize() {
        DVLOG(2) << "=> IndexerTest::checkOptimize()";

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

        DVLOG(2) << "<= IndexerTest::checkOptimize()";
    }

    /**
     * Create barrels, optimize barrels (merge them into one), and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void createAfterOptimizeBarrel(int barrelNum) {
        DVLOG(2) << "=> IndexerTest::createAfterOptimizeBarrel()";

        for(int i=0; i<barrelNum; ++i)
            createDocument(); // create barrel i

        indexer_->optimizeIndex();

        for(int i=0; i<barrelNum; ++i)
            createDocument(); // create barrel i

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
        BOOST_CHECK(pBarrelsInfo->getBarrelCount() >= 1 && pBarrelsInfo->getBarrelCount() <= 2*barrelNum);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

        DVLOG(2) << "<= IndexerTest::createAfterOptimizeBarrel()";
    }

private:
    void removeIndexFiles() {
        boost::filesystem::path indexPath(INDEX_FILE_DIR);
        boost::filesystem::remove_all(indexPath);
    }

    void initIndexer(const string& indexmode, bool btree = true, int skipinterval = 0, int skiplevel = 0) {
        if (btree)
            indexer_ = new Indexer();
        else
            indexer_ = new Indexer(MANAGER_PURE_INDEX);

        IndexManagerConfig indexManagerConfig;

        boost::filesystem::path path(INDEX_FILE_DIR);

        indexManagerConfig.indexStrategy_.indexLocation_ = path.string();
        indexManagerConfig.indexStrategy_.indexMode_ = indexmode;
        indexManagerConfig.indexStrategy_.memory_ = 30000000;
        indexManagerConfig.indexStrategy_.indexDocLength_ = true;
        indexManagerConfig.indexStrategy_.skipInterval_ = skipinterval;
        indexManagerConfig.indexStrategy_.maxSkipLevel_ = skiplevel;
        indexManagerConfig.mergeStrategy_.param_ = "dbt";
        indexManagerConfig.storeStrategy_.param_ = "mmap";
        std::map<std::string, unsigned int> collectionIdMapping;
        collectionIdMapping.insert(std::pair<std::string, unsigned int>("testcoll", COLLECTION_ID));

        std::vector<std::string> propertyList(1, "content");
        if(btree)
        {
            std::string date("date");
            propertyList.push_back(date);
            std::string url("provider");
            propertyList.push_back(url);
        }
        std::sort(propertyList.begin(),propertyList.end());
        IndexerCollectionMeta indexCollectionMeta;
        indexCollectionMeta.setName("testcoll");
        for (std::size_t i=0;i<propertyList.size();i++)
        {
            IndexerPropertyConfig indexerPropertyConfig(1+i, propertyList[i], true, true);
            propertyMap_[propertyList[i]] = 1+i;
            if(propertyList[i] != "content")
            {
                indexerPropertyConfig.setIsForward(false);
                indexerPropertyConfig.setIsFilter(true);
            }
            indexCollectionMeta.addPropertyConfig(indexerPropertyConfig);
        }
        indexManagerConfig.addCollectionMeta(indexCollectionMeta);

        indexer_->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
    }

    void prepareDocument(IndexerDocument& document, unsigned int docId, bool filter = true) {
        document.setDocId(docId, COLLECTION_ID);

        IndexerPropertyConfig propertyConfig(propertyMap_["content"],"content",true,true);
        propertyConfig.setIsLAInput(true);

        boost::shared_ptr<LAInput> laInput(new LAInput);
        document.insertProperty(propertyConfig, laInput);

        const unsigned int docLen = docLenRand_();
        mapDocIdLen_[docId] = docLen;
        maxDocID_ = docId;
        for(unsigned int i = 0; i < docLen; ++i)
        {
            LAInputUnit unit;
            unit.termid_ = termIDRand_();
#ifdef LOG_TERM_ID
            cout << "term id: " << unit.termid_ << endl;
#endif
            unit.wordOffset_ = i;
            document.add_to_property(unit);
        }
#ifdef LOG_TERM_ID
        cout << endl;
#endif

        if(filter)
        {
            using namespace boost::posix_time;
            using namespace boost::gregorian;

            propertyConfig.setPropertyId(propertyMap_["date"]);
            propertyConfig.setName("date");
            propertyConfig.setIsForward(false);
            propertyConfig.setIsFilter(true);
            propertyConfig.setIsLAInput(false);

            struct tm atm;
            ptime now = second_clock::local_time();
            atm = to_tm(now);
            int64_t time = mktime(&atm);

            document.insertProperty(propertyConfig, time);
        }
    }
};

BOOST_AUTO_TEST_SUITE( t_IndexReader )

BOOST_AUTO_TEST_CASE(index)
{
    IndexerTest indexerTest(TEST_DOC_NUM);

    indexerTest.setUp();

    // create barrel 0
    indexerTest.createDocument();
    indexerTest.checkDocLength();

    // create barrel 1, 2, 3
    for(int i=0; i<TEST_BARREL_NUM; ++i)
    {
        indexerTest.createDocument();
        indexerTest.checkDocLength();
    }
    indexerTest.tearDown();

    // new Indexer instance, create barrel 4, 5, 6
    indexerTest.setUp(false);
    indexerTest.checkDocLength();
    for(int i=0; i<TEST_BARREL_NUM; ++i)
    {
        indexerTest.createDocument();
        indexerTest.checkDocLength();
    }
    indexerTest.tearDown();
}

BOOST_AUTO_TEST_CASE(update)
{
    IndexerTest indexerTest(TEST_DOC_NUM);

    indexerTest.setUp();
    indexerTest.createDocument();

    for(int i=0; i<TEST_BARREL_NUM; ++i)
    {
        indexerTest.updateDocument();
        indexerTest.checkDocLength();
    }
    indexerTest.tearDown();
}

BOOST_AUTO_TEST_CASE(remove)
{
    IndexerTest indexerTest(TEST_DOC_NUM);

    indexerTest.setUp();
    indexerTest.createDocument();

    while(! indexerTest.isDocEmpty())
    {
        indexerTest.removeDocument();
        indexerTest.checkDocLength();
    }
    indexerTest.checkDocLength();

    indexerTest.tearDown();
}

BOOST_AUTO_TEST_CASE(barrelInfo_check)
{
    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp();
        indexerTest.checkBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();
    }

    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp(true, INDEX_MODE_REALTIME);
        indexerTest.checkBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();
    }
}

BOOST_AUTO_TEST_CASE(barrelInfo_optimize)
{
    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp();
        indexerTest.optimizeBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();

        indexerTest.setUp(false);
        indexerTest.checkOptimize();
        indexerTest.tearDown();
    }

    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp(true, INDEX_MODE_REALTIME);
        indexerTest.optimizeBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();

        indexerTest.setUp(false);
        indexerTest.checkOptimize();
        indexerTest.tearDown();
    }
}

BOOST_AUTO_TEST_CASE(barrelInfo_create_after_optimize)
{
    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp();
        indexerTest.createAfterOptimizeBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();
    }

    {
        IndexerTest indexerTest(TEST_DOC_NUM);
        indexerTest.setUp(true, INDEX_MODE_REALTIME);
        indexerTest.createAfterOptimizeBarrel(TEST_BARREL_NUM);
        indexerTest.tearDown();
    }
}

BOOST_AUTO_TEST_SUITE_END()

