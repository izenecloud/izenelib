#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include "IndexerTestFixture.h"
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/AbsTermReader.h>

using namespace std;
using namespace boost;

#define LOG_DOC_OPERATION
//#define LOG_CHECK_OPERATION
//#define LOG_TERM_ID
//#define LOG_QUERY_OPERATION

namespace
{
const char* INDEX_FILE_DIR = "./index";

const unsigned int COLLECTION_ID = 1;

const char* INDEX_MODE_REALTIME = "realtime";
const char* INVERTED_FIELD = "content";
};

IndexerTestFixture::IndexerTestFixture()
    :indexer_(new Indexer)
    ,newDocNum_(0)
    ,docLenRand_(randEngine_, uniform_int<>(1, 1))
    ,termIDRand_(randEngine_, uniform_int<>(1, 1))
    ,skipToRand_(randEngine_, uniform_int<>(0, 1))
    ,maxDocID_(0)
{
    removeIndexFiles();

    //IndexerTestConfig config = {0, 10, 10, "default", true};
    //configTest(config);
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

void IndexerTestFixture::printStats() const
{
    BOOST_TEST_MESSAGE("\nprinting statistics...");
    BOOST_TEST_MESSAGE("doc count: " << mapDocIdLen_.size());
    BOOST_TEST_MESSAGE("unique term count: " << mapCTermId_.size());

    int64_t docLenSum = 0;
    for(CTermIdMapT::const_iterator it = mapCTermId_.begin();
            it != mapCTermId_.end();
            ++it)
    {
        docLenSum += it->second.second;
    }

    BOOST_TEST_MESSAGE("sum of doc length: " << docLenSum << "\n");
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

void IndexerTestFixture::updateDocument()
{
    VLOG(2) << "=> IndexerTestFixture::updateDocument()";

    if(mapDocIdLen_.empty())
        return;

    RandGeneratorT randDocNum(randEngine_, uniform_int<>(1, mapDocIdLen_.size()));

    const int updateNum = randDocNum(); // range [1, size]
    list<docid_t> removeDocList;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("start updating " << updateNum << " docs...");
#endif
    docid_t newDocID = maxDocID_ + 1;
    for(int i=0; i<updateNum; ++i, ++newDocID)
    {
        const int updatePos = randDocNum() % mapDocIdLen_.size();
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
    removeDocTerms(removeDocList);

    indexer_->flush();
    VLOG(2) << "<= IndexerTestFixture::updateDocument()";
}

void IndexerTestFixture::removeDocument()
{
    VLOG(2) << "=> IndexerTestFixture::removeDocument()";

    if(mapDocIdLen_.empty())
        return;

    RandGeneratorT randDocNum(randEngine_, uniform_int<>(1, mapDocIdLen_.size()));

    const int removeNum = randDocNum(); // range [1, size]
    list<docid_t> removeDocList;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("start removing " << removeNum << " docs...");
#endif
    for(int i=0; i<removeNum; ++i)
    {
        const int removePos = randDocNum() % mapDocIdLen_.size();
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
    removeDocTerms(removeDocList);

    // remove doc id exceed the range
    docid_t overId = maxDocID_ + 1;
#ifdef LOG_DOC_OPERATION
    BOOST_TEST_MESSAGE("remove exceed doc id: " << overId);
#endif
    indexer_->removeDocument(COLLECTION_ID, overId);

    indexer_->flush();
    VLOG(2) << "<= IndexerTestFixture::removeDocument()";
}

void IndexerTestFixture::checkDocLength()
{
    VLOG(2) << "=> IndexerTestFixture::checkDocLength()";

    IndexReader* pIndexReader = indexer_->getIndexReader();

    BOOST_CHECK_EQUAL(pIndexReader->numDocs(), mapDocIdLen_.size());
    BOOST_CHECK_EQUAL(pIndexReader->maxDoc(), maxDocID_);

    for(DocIdLenMapT::const_iterator lenMapIt = mapDocIdLen_.begin();
            lenMapIt != mapDocIdLen_.end(); ++lenMapIt)
    {
#ifdef LOG_CHECK_OPERATION
        BOOST_TEST_MESSAGE("check: " << lenMapIt->first);
#endif
        BOOST_CHECK_EQUAL(pIndexReader->docLength(lenMapIt->first, indexer_->getPropertyIDByName(COLLECTION_ID, INVERTED_FIELD)), lenMapIt->second);
    }

    VLOG(2) << "<= IndexerTestFixture::checkDocLength()";
}

void IndexerTestFixture::checkTermDocFreqs()
{
    VLOG(2) << "=> IndexerTestFixture::checkTermDocFreqs()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

#ifdef LOG_QUERY_OPERATION
    BOOST_TEST_MESSAGE("check TermDocFreqs::docFreq(), getCTF() on " << mapCTermId_.size() << " terms.");
#endif
    Term term(INVERTED_FIELD);
    for(CTermIdMapT::const_iterator termIt = mapCTermId_.begin();
            termIt != mapCTermId_.end(); ++termIt)
    {
        term.setValue(termIt->first);
#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAGE("check term id: " << termIt->first);
#endif
        BOOST_CHECK(pTermReader->seek(&term));

        boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());
        BOOST_CHECK_EQUAL(pTermDocFreqs->docFreq(), termIt->second.first);
        BOOST_CHECK_EQUAL(pTermDocFreqs->getCTF(), termIt->second.second);
    }

    VLOG(2) << "<= IndexerTestFixture::checkTermDocFreqs()";
}

void IndexerTestFixture::checkNextSkipTo()
{
    VLOG(2) << "=> IndexerTestFixture::checkNextSkipTo()";

    bool isQueryFailed = true;
    while(isQueryFailed)
    {
        try
        {
            checkNextSkipToImpl();
            isQueryFailed = false;
        }
        catch(IndexManagerException& e)
        {
            LOG(ERROR) << "start query again as exception found: " << e.what();
        }
    }

    VLOG(2) << "<= IndexerTestFixture::checkNextSkipTo()";
}

void IndexerTestFixture::checkBarrel(int barrelNum)
{
    VLOG(2) << "=> IndexerTestFixture::checkBarrel()";

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

    VLOG(2) << "<= IndexerTestFixture::checkBarrel()";
}

void IndexerTestFixture::optimizeBarrel(int barrelNum)
{
    VLOG(2) << "=> IndexerTestFixture::optimizeBarrel()";

    for(int i=0; i<barrelNum; ++i)
        createDocument(); // create barrel i

    indexer_->optimizeIndex();

    // wait for merge finish
    indexer_->waitForMergeFinish();

    IndexReader* pIndexReader = indexer_->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

    VLOG(2) << "<= IndexerTestFixture::optimizeBarrel()";
}

void IndexerTestFixture::createAfterOptimizeBarrel(int barrelNum)
{
    VLOG(2) << "=> IndexerTestFixture::createAfterOptimizeBarrel()";

    for(int i=0; i<barrelNum; ++i)
        createDocument(); // create barrel i

    indexer_->optimizeIndex();

    for(int i=0; i<barrelNum; ++i)
        createDocument(); // create barrel i

    // wait for merge finish
    indexer_->waitForMergeFinish();

    IndexReader* pIndexReader = indexer_->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

    if(indexer_->getIndexManagerConfig()->indexStrategy_.indexMode_ != INDEX_MODE_REALTIME)
    {
        BOOST_CHECK(pBarrelsInfo->getBarrelCount() >= 1 && pBarrelsInfo->getBarrelCount() <= barrelNum+1);
    }

    VLOG(2) << "<= IndexerTestFixture::createAfterOptimizeBarrel()";
}

void IndexerTestFixture::pauseResumeMerge(int barrelNum)
{
    VLOG(2) << "=> IndexerTestFixture::pauseResumeMerge()";

    indexer_->pauseMerge();

    for(int i=0; i<barrelNum; ++i)
        createDocument(); // create barrel i

    IndexReader* pIndexReader = indexer_->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();
    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), barrelNum);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

    indexer_->resumeMerge();
    indexer_->optimizeIndex();

    // wait for merge finish
    indexer_->waitForMergeFinish();

    pIndexReader = indexer_->getIndexReader();
    pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);
    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), maxDocID_);
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), mapDocIdLen_.size());

    VLOG(2) << "<= IndexerTestFixture::pauseResumeMerge()";
}

void IndexerTestFixture::removeDocTerms(const list<docid_t>& removeDocList)
{
    if(removeDocList.empty())
        return;

    // regenerate term ids for each doc
    boost::mt19937 randEngine;
    RandGeneratorT docLenRand(randEngine, docLenRand_.distribution());
    RandGeneratorT termIDRand(randEngine, termIDRand_.distribution());

    list<docid_t>::const_iterator removeIt = removeDocList.begin();
    for(docid_t docID = 1; docID <= maxDocID_; ++docID)
    {
        DTermIdMapT docTermIdMap;

        bool isDocRemoved = (docID == *removeIt);
#ifdef LOG_TERM_ID
        if(isDocRemoved)
            BOOST_TEST_MESSAGE("remove term id for doc id: " << docID);
#endif

        const unsigned int docLen = docLenRand();
        for(unsigned int j = 0; j < docLen; ++j)
        {
            unsigned int termId = termIDRand();
#ifdef LOG_TERM_ID
            if(isDocRemoved)
                BOOST_TEST_MESSAGE("remove term id: " << termId);
#endif

            if(isDocRemoved)
                docTermIdMap[termId].push_back(j);
        }
#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAG("");
#endif

        if(isDocRemoved)
        {
            for(DTermIdMapT::const_iterator it = docTermIdMap.begin();
                    it != docTermIdMap.end();
                    ++it)
            {
                CTermIdMapT::iterator colIt = mapCTermId_.find(it->first);
                BOOST_CHECK(colIt != mapCTermId_.end());
                colIt->second.second -= it->second.size();
                if(--colIt->second.first == 0)
                {
                    BOOST_CHECK_EQUAL(colIt->second.second, 0);
                    mapCTermId_.erase(colIt);
                }
            }

            if(++removeIt == removeDocList.end())
                break;
        }
    }
}

void IndexerTestFixture::checkNextSkipToImpl()
{
    VLOG(2) << "=> IndexerTestFixture::checkNextSkipToImpl()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

    if(mapDocIdLen_.empty())
    {
        // TermReader should be NULL when no doc exists
        BOOST_CHECK(pTermReader.get() == NULL);
        VLOG(2) << "<= IndexerTestFixture::checkNextSkipToImpl(), no doc exists";
        return;
    }

    // regenerate term ids for each doc
    boost::mt19937 randEngine;
    RandGeneratorT docLenRand(randEngine, docLenRand_.distribution());
    RandGeneratorT termIDRand(randEngine, termIDRand_.distribution());

    for(docid_t docID = 1; docID <= maxDocID_; ++docID)
    {
        DTermIdMapT docTermIdMap;

        const unsigned int docLen = docLenRand();
        for(unsigned int j = 0; j < docLen; ++j)
        {
            unsigned int termId = termIDRand();
            docTermIdMap[termId].push_back(j);
        }

        checkNextSkipToDoc(pTermReader.get(), docID, docTermIdMap);
    }

    VLOG(2) << "<= IndexerTestFixture::checkNextSkipToImpl()";
}

void IndexerTestFixture::checkNextSkipToDoc(TermReader* pTermReader, docid_t docID, const DTermIdMapT& docTermIdMap)
{
    bool isDocRemoved = (mapDocIdLen_.find(docID) == mapDocIdLen_.end());

#ifdef LOG_QUERY_OPERATION
    BOOST_TEST_MESSAGE("check TermDocFreqs for doc id: " << docID
            << ", unique terms: " << docTermIdMap.size()
            << ", doc is removed: " << isDocRemoved);
#endif

    Term term(INVERTED_FIELD);
    for(DTermIdMapT::const_iterator termIt = docTermIdMap.begin();
            termIt != docTermIdMap.end();
            ++termIt)
    {
#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAGE("check term id: " << termIt->first);
#endif
        term.setValue(termIt->first);

        // this term is already removed from the whole collection
        if(mapCTermId_.find(term.getValue()) == mapCTermId_.end())
        {
            // TermReader::seek() returns false in 2 cases:
            // 1. merge finished
            // 2. barrel count is 0
            if(pTermReader->seek(&term))
            {
                boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());
                if(skipToRand_())
                    BOOST_CHECK_EQUAL(pTermDocFreqs->skipTo(docID), BAD_DOCID);
                else
                    BOOST_CHECK_EQUAL(pTermDocFreqs->next(), false);
            }
        }
        else
        {
            BOOST_CHECK(pTermReader->seek(&term));
            boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());

            nextOrSkipTo(pTermDocFreqs.get(), docID, isDocRemoved);
            if(! isDocRemoved)
                BOOST_CHECK_EQUAL(pTermDocFreqs->freq(), termIt->second.size());

            boost::scoped_ptr<TermPositions> pTermPositions(pTermReader->termPositions());
            nextOrSkipTo(pTermPositions.get(), docID, isDocRemoved);
            if(! isDocRemoved)
            {
                BOOST_CHECK_EQUAL(pTermPositions->freq(), termIt->second.size());
                for(LocListT::const_iterator locIter = termIt->second.begin();
                        locIter != termIt->second.end();
                        ++locIter)
                {
#ifdef LOG_TERM_ID
                    BOOST_TEST_MESSAGE("check term position: " << *locIter);
#endif
                    BOOST_CHECK_EQUAL(pTermPositions->nextPosition(), *locIter);
                }
                BOOST_CHECK_EQUAL(pTermPositions->nextPosition(), BAD_POSITION);
            }
        }
    }
}

void IndexerTestFixture::nextOrSkipTo(TermDocFreqs* pTermDocFreqs, docid_t docID, bool isDocRemoved)
{
    if(isDocRemoved)
    {
        if(skipToRand_())
        {
            docid_t skipToResult = pTermDocFreqs->skipTo(docID);
            BOOST_CHECK(skipToResult > docID || skipToResult == BAD_DOCID);
        }
        else
        {
            while(pTermDocFreqs->next())
            {
                if(pTermDocFreqs->doc() >= docID)
                {
                    BOOST_CHECK(pTermDocFreqs->doc() > docID);
                    break;
                }
            }
        }
    }
    else
    {
        if(skipToRand_())
            BOOST_CHECK_EQUAL(pTermDocFreqs->skipTo(docID), docID);
        else
        {
            while(pTermDocFreqs->next())
            {
                if(pTermDocFreqs->doc() >= docID)
                    break;
            }
        }

        BOOST_CHECK_EQUAL(pTermDocFreqs->doc(), docID);
    }
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
#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAGE("term id: " << unit.termid_);
#endif
        document.add_to_property(unit);

        docTermIdMap[unit.termid_].push_back(i);
    }
#ifdef LOG_TERM_ID
    BOOST_TEST_MESSAG("");
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
