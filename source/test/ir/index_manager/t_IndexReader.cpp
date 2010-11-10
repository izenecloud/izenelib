#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <utility> // std::pair

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/utility/system.h>

#define LOG_DOC_OPERATION
//#define LOG_CHECK_OPERATION
//#define LOG_TERM_ID
#define LOG_QUERY_OPERATION

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

namespace
{
const char* INDEX_FILE_DIR = "./index";

const unsigned int COLLECTION_ID = 1;

const char* INDEX_MODE_REALTIME = "realtime";
const char* INVERTED_FIELD = "content";

const int TEST_DOC_NUM = 10;
const int TEST_BARREL_NUM = 3;

const int TEST_DOC_LEN_RANGE = 10 * TEST_DOC_NUM;
const int TEST_TERM_ID_RANGE = 100 * TEST_DOC_NUM;
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
    ///< in @p nextOrSkipTo(), 1 to use @p TermDocFreqs::next(), 0 to use @p TermDocFreqs::skipTo()
    boost::variate_generator<mt19937, uniform_int<> > skipToRand_; 

    /**
     * as the max doc id in indexer is the max doc id ever indexed,
     * no matter whether that doc is deleted,
     * we have to maintain that max doc id here.
     */
    docid_t maxDocID_;

    ///< docid => doc length
    typedef map<docid_t, size_t> DocIdLenMapT;
    DocIdLenMapT mapDocIdLen_;

    ///< termid => (doc freq, collection term freq)
    typedef map<termid_t, pair<freq_t, int64_t> > CTermIdMapT;
    CTermIdMapT mapCTermId_;

    ///< term position list
    typedef vector<loc_t> LocListT;
    ///< termid => term position list of one doc
    typedef map<termid_t, LocListT> DTermIdMapT;

public:
    IndexerTest(unsigned int docNum, bool isDocNumRand = false, unsigned int docIDSkipMax = 1)
        : indexer_(0)
          ,newDocNum_(docNum)
          ,isDocNumRand_(isDocNumRand)
          ,docLenRand_(randEngine_, uniform_int<>(1, TEST_DOC_LEN_RANGE))
          ,termIDRand_(randEngine_, uniform_int<>(1, TEST_TERM_ID_RANGE))
          ,docIDSkipRand_(randEngine_, uniform_int<>(1, docIDSkipMax))
          ,docNumRand_(randEngine_, uniform_int<>(1, newDocNum_))
          ,skipToRand_(randEngine_, uniform_int<>(0, 1))
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
     * Print below statistics:
     * - doc count
     * - unique term count
     * - sum of doc length in collection.
     */
    void printStats() const {
        cout << endl;
        cout << "printing statistics..." << endl;
        cout << "doc count: " << mapDocIdLen_.size() << endl;
        cout << "unique term count: " << mapCTermId_.size() << endl;

        int64_t docLenSum = 0;
        for(CTermIdMapT::const_iterator it = mapCTermId_.begin();
            it != mapCTermId_.end();
            ++it)
        {
            docLenSum += it->second.second;
        }

        cout << "sum of doc length: " << docLenSum << endl;
        cout << endl;
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

        for(DocIdLenMapT::const_iterator lenMapIt = mapDocIdLen_.begin();
            lenMapIt != mapDocIdLen_.end(); ++lenMapIt)
        {
#ifdef LOG_CHECK_OPERATION
            cout << "check: " << lenMapIt->first << endl;
#endif
            BOOST_CHECK_EQUAL(pIndexReader->docLength(lenMapIt->first, indexer_->getPropertyIDByName(COLLECTION_ID, INVERTED_FIELD)), lenMapIt->second);
        }

        DVLOG(2) << "<= IndexerTest::checkDocLength()";
    }

    void checkTermDocFreqs() {
        DVLOG(2) << "=> IndexerTest::checkTermDocFreqs()";

        IndexReader* pIndexReader = indexer_->getIndexReader();
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

#ifdef LOG_QUERY_OPERATION
        cout << "check TermDocFreqs::docFreq(), getCTF() on " << mapCTermId_.size() << " terms." << endl;
#endif
        Term term(INVERTED_FIELD);
        for(CTermIdMapT::const_iterator termIt = mapCTermId_.begin();
            termIt != mapCTermId_.end(); ++termIt)
        {
            term.setValue(termIt->first);
#ifdef LOG_TERM_ID
            cout << "check term id: " << termIt->first << endl;
#endif
            BOOST_CHECK(pTermReader->seek(&term));

            boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());
            BOOST_CHECK_EQUAL(pTermDocFreqs->docFreq(), termIt->second.first);
            BOOST_CHECK_EQUAL(pTermDocFreqs->getCTF(), termIt->second.second);
        }

        DVLOG(2) << "<= IndexerTest::checkTermDocFreqs()";
    }

    void checkNextSkipTo() {
        DVLOG(2) << "=> IndexerTest::checkNextSkipTo()";

        IndexReader* pIndexReader = indexer_->getIndexReader();
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

        // regenerate term ids for each doc
        boost::mt19937 randEngine;
        boost::variate_generator<mt19937, uniform_int<> > docLenRand(randEngine, uniform_int<>(1, TEST_DOC_LEN_RANGE));
        boost::variate_generator<mt19937, uniform_int<> > termIDRand(randEngine, uniform_int<>(1, TEST_TERM_ID_RANGE));
        boost::variate_generator<mt19937, uniform_int<> > docIDSkipRand(randEngine, uniform_int<>(1, 1));
        boost::variate_generator<mt19937, uniform_int<> > docNumRand(randEngine, uniform_int<>(1, newDocNum_));

        docid_t docID = 0;
        for(unsigned int i = 1; i <= (isDocNumRand_ ? docNumRand() : maxDocID_); i++)
        {
            docID += docIDSkipRand();
            DTermIdMapT docTermIdMap;

            const unsigned int docLen = docLenRand();
            for(unsigned int j = 0; j < docLen; ++j)
            {
                unsigned int termId = termIDRand();
#ifdef LOG_TERM_ID
                cout << "term id: " << termId << endl;
#endif

                docTermIdMap[termId].push_back(j);
            }
#ifdef LOG_TERM_ID
            cout << endl;
#endif

#ifdef LOG_QUERY_OPERATION
            cout << "check TermDocFreqs for doc id: " << docID << ", doc length: " << docLen << ", unique terms: " << docTermIdMap.size() << endl;
#endif
            Term term(INVERTED_FIELD);
            for(DTermIdMapT::const_iterator termIt = docTermIdMap.begin();
                    termIt != docTermIdMap.end();
                    ++termIt)
            {
                term.setValue(termIt->first);
                BOOST_CHECK(pTermReader->seek(&term));

                boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());
                nextOrSkipTo(pTermDocFreqs.get(), docID);
                BOOST_CHECK_EQUAL(pTermDocFreqs->freq(), termIt->second.size());

                boost::scoped_ptr<TermPositions> pTermPositions(pTermReader->termPositions());
                nextOrSkipTo(pTermPositions.get(), docID);
                BOOST_CHECK_EQUAL(pTermPositions->freq(), termIt->second.size());
                for(LocListT::const_iterator locIter = termIt->second.begin();
                        locIter != termIt->second.end();
                        ++locIter)
                {
                    BOOST_CHECK_EQUAL(pTermPositions->nextPosition(), *locIter);
                }
                BOOST_CHECK_EQUAL(pTermPositions->nextPosition(), BAD_POSITION);
            }
        }

        DVLOG(2) << "<= IndexerTest::checkNextSkipTo()";
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

        // wait for merge finish
        IndexWriter* pWriter = indexer_->getIndexWriter();
        pWriter->waitForMergeFinish();

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

        DVLOG(2) << "<= IndexerTest::optimizeBarrel()";
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

        // wait for merge finish
        IndexWriter* pWriter = indexer_->getIndexWriter();
        pWriter->waitForMergeFinish();

        IndexReader* pIndexReader = indexer_->getIndexReader();
        BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

        BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
        BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

        if(indexer_->getIndexManagerConfig()->indexStrategy_.indexMode_ != INDEX_MODE_REALTIME)
        {
            BOOST_CHECK(pBarrelsInfo->getBarrelCount() >= 1 && pBarrelsInfo->getBarrelCount() <= barrelNum+1);
        }

        DVLOG(2) << "<= IndexerTest::createAfterOptimizeBarrel()";
    }

private:
    /**
     * Move the cursor to @p docID using either @p TermDocFreqs::next() or @p TermDocFreqs::skipTo(),
     * these two methods are selected randomly.
     */
    void nextOrSkipTo(TermDocFreqs* pTermDocFreqs, docid_t docID) {
        if(skipToRand_())
        {
            while(pTermDocFreqs->next())
            {
                if(pTermDocFreqs->doc() == docID)
                    break;
            }
        }
        else
        {
            BOOST_CHECK_EQUAL(pTermDocFreqs->skipTo(docID), docID);
        }

        BOOST_CHECK_EQUAL(pTermDocFreqs->doc(), docID);
    }

    void removeIndexFiles() {
        boost::filesystem::path indexPath(INDEX_FILE_DIR);
        boost::filesystem::remove_all(indexPath);
    }

    void initIndexer(const string& indexmode, int skipinterval = 0, int skiplevel = 0) {
        indexer_ = new Indexer;

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

        std::vector<std::string> propertyList;
        propertyList.push_back(INVERTED_FIELD);
        propertyList.push_back("date");
        propertyList.push_back("provider");

        std::sort(propertyList.begin(),propertyList.end());
        IndexerCollectionMeta indexCollectionMeta;
        indexCollectionMeta.setName("testcoll");
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

        indexer_->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
    }

    void prepareDocument(IndexerDocument& document, unsigned int docId, bool filter = true) {
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
#ifdef LOG_TERM_ID
            cout << "term id: " << unit.termid_ << endl;
#endif
            document.add_to_property(unit);

            docTermIdMap[unit.termid_].push_back(i);
        }
#ifdef LOG_TERM_ID
        cout << endl;
#endif

        for(DTermIdMapT::const_iterator it=docTermIdMap.begin();
            it!=docTermIdMap.end(); ++it)
        {
            ++mapCTermId_[it->first].first;
            mapCTermId_[it->first].second += it->second.size();
        }

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
};

BOOST_AUTO_TEST_SUITE( t_IndexReader )

//BOOST_AUTO_TEST_CASE(index)
//{
    //DVLOG(2) << "=> TEST_CASE::index";
    //IndexerTest indexerTest(TEST_DOC_NUM);

    //indexerTest.setUp();

    //// create barrel 0
    //indexerTest.createDocument();
    //indexerTest.checkDocLength();

    //// create more barrels
    //for(int i=0; i<TEST_BARREL_NUM; ++i)
    //{
        //indexerTest.createDocument();
        //indexerTest.checkDocLength();
    //}
    //indexerTest.tearDown();

    //// new Indexer instance, create more barrels
    //indexerTest.setUp(false);
    //indexerTest.checkDocLength();
    //for(int i=0; i<TEST_BARREL_NUM; ++i)
    //{
        //indexerTest.createDocument();
        //indexerTest.checkDocLength();
    //}
    //indexerTest.tearDown();
    //DVLOG(2) << "<= TEST_CASE::index";
//}

//BOOST_AUTO_TEST_CASE(update)
//{
    //DVLOG(2) << "=> TEST_CASE::update";
    //IndexerTest indexerTest(TEST_DOC_NUM);

    //indexerTest.setUp();
    //indexerTest.createDocument();

    //for(int i=0; i<TEST_BARREL_NUM; ++i)
    //{
        //indexerTest.updateDocument();
        //indexerTest.checkDocLength();
    //}
    //indexerTest.tearDown();
    //DVLOG(2) << "<= TEST_CASE::update";
//}

//BOOST_AUTO_TEST_CASE(remove)
//{
    //DVLOG(2) << "=> TEST_CASE::remove";
    //IndexerTest indexerTest(TEST_DOC_NUM);

    //indexerTest.setUp();
    //indexerTest.createDocument();

    //while(! indexerTest.isDocEmpty())
    //{
        //indexerTest.removeDocument();
        //indexerTest.checkDocLength();
    //}
    //indexerTest.checkDocLength();

    //indexerTest.tearDown();
    //DVLOG(2) << "<= TEST_CASE::remove";
//}

//BOOST_AUTO_TEST_CASE(barrelInfo_check)
//{
    //DVLOG(2) << "=> TEST_CASE::barrelInfo_check";
    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp();
        //indexerTest.checkBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}

    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp(true, INDEX_MODE_REALTIME);
        //indexerTest.checkBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}
    //DVLOG(2) << "<= TEST_CASE::barrelInfo_check";
//}

//BOOST_AUTO_TEST_CASE(barrelInfo_optimize)
//{
    //DVLOG(2) << "=> TEST_CASE::barrelInfo_optimize";
    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp();
        //indexerTest.optimizeBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}

    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp(true, INDEX_MODE_REALTIME);
        //indexerTest.optimizeBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}
    //DVLOG(2) << "<= TEST_CASE::barrelInfo_optimize";
//}

//BOOST_AUTO_TEST_CASE(barrelInfo_create_after_optimize)
//{
    //DVLOG(2) << "=> TEST_CASE::barrelInfo_create_after_optimize";
    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp();
        //indexerTest.createAfterOptimizeBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}

    //{
        //IndexerTest indexerTest(TEST_DOC_NUM);
        //indexerTest.setUp(true, INDEX_MODE_REALTIME);
        //indexerTest.createAfterOptimizeBarrel(TEST_BARREL_NUM);
        //indexerTest.tearDown();
    //}
    //DVLOG(2) << "<= TEST_CASE::barrelInfo_create_after_optimize";
//}

BOOST_AUTO_TEST_CASE(TermDocFreqs_check)
{
    DVLOG(2) << "=> TEST_CASE::TermDocFreqs_check";

    IndexerTest indexerTest(TEST_DOC_NUM);
    indexerTest.setUp();

    for(int i=0; i<TEST_BARREL_NUM; ++i)
    {
        indexerTest.createDocument();
        indexerTest.checkDocLength();
    }
    indexerTest.printStats();
    indexerTest.checkTermDocFreqs();
    indexerTest.checkNextSkipTo();
    indexerTest.tearDown();

    DVLOG(2) << "<= TEST_CASE::TermDocFreqs_check";
}

BOOST_AUTO_TEST_SUITE_END()

