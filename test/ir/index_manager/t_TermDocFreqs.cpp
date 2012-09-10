#include "t_TermDocFreqs.h"

#include <sstream> // std::ostringstream
#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/AbsTermIterator.h>
#include <util/test/BoostTestThreadSafety.h>

//#define LOG_TERM_ID
//#define LOG_QUERY_OPERATION
//#define LOG_TERM_ITERATE

namespace
{
/**
 * run @p func until no @c IndexManagerException exception is thrown.
 * @param func function object to run
 */
template<class Function> void runToSuccess(Function func) {
    bool isSuccess = false;
    while(! isSuccess)
    {
        try
        {
            func();
            isSuccess = true;
        }
        catch(std::exception& e)
        {
            LOG(ERROR) << "start query again as exception found: " << e.what();
        }
    }
}
}

namespace t_TermDocFreqs
{
TermDocFreqsTestFixture::TermDocFreqsTestFixture()
    : isSkipToRand_(randEngine_, boost::bernoulli_distribution<>())
    , docLenRand2_(docLenRand_)
    , termIDRand2_(termIDRand_)
{
}

void TermDocFreqsTestFixture::resetRand2()
{
    boost::mt19937 newEngine;

    docLenRand2_ = docLenRand_;
    docLenRand2_.engine() = newEngine;
    termIDRand2_ = termIDRand_;
    termIDRand2_.engine() = newEngine;
}

void TermDocFreqsTestFixture::removeFixtureDocs(const std::list<docid_t>& removeDocList)
{
    if(removeDocList.empty())
        return;

    // regenerate term ids for each doc
    resetRand2();
    list<docid_t>::const_iterator removeIt = removeDocList.begin();
    for(docid_t docID = 1; docID <= maxDocID_; ++docID)
    {
        DTermIdMapT docTermIdMap;

        bool isDocRemoved = (docID == *removeIt);
#ifdef LOG_TERM_ID
        if(isDocRemoved)
            BOOST_TEST_MESSAGE("remove term id for doc id: " << docID);
#endif

        const unsigned int docLen = docLenRand2_();
        for(unsigned int j = 0; j < docLen; ++j)
        {
            unsigned int termId = termIDRand2_();
#ifdef LOG_TERM_ID
            if(isDocRemoved)
                BOOST_TEST_MESSAGE("remove term id: " << termId);
#endif

            if(isDocRemoved)
                docTermIdMap[termId].push_back(j);
        }
#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAGE("");
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

void TermDocFreqsTestFixture::addFixtureDoc(const DTermIdMapT& docTermIdMap)
{
    for(DTermIdMapT::const_iterator it=docTermIdMap.begin();
            it!=docTermIdMap.end(); ++it)
    {
#ifdef LOG_TERM_ID
        std::ostringstream oss;
        oss << "term id: " << it->first << ", position: ";
        for(LocListT::const_iterator posIt = it->second.begin();
                posIt != it->second.end();
                ++posIt)
            oss << *posIt << ", ";

        BOOST_TEST_MESSAGE(oss.str());
#endif
        ++mapCTermId_[it->first].first;
        mapCTermId_[it->first].second += it->second.size();
    }
#ifdef LOG_TERM_ID
    BOOST_TEST_MESSAGE("");
#endif
}

void TermDocFreqsTestFixture::checkUpdateDocs(const std::list<docid_t>& updateDocList)
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkUpdateDocs()";

    runToSuccess(boost::bind(&TermDocFreqsTestFixture::checkUpdateDocsImpl,
                             this,
                             boost::cref(updateDocList)));

    VLOG(2) << "<= TermDocFreqsTestFixture::checkUpdateDocs()";
}

void TermDocFreqsTestFixture::checkUpdateDocsImpl(const std::list<docid_t>& updateDocList)
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkUpdateDocsImpl()";

    if(updateDocList.empty())
    {
        VLOG(2) << "<= TermDocFreqsTestFixture::checkUpdateDocsImpl(), updateDocList is empty";
        return;
    }

    IndexReader* pIndexReader = indexer_->getIndexReader();
    if(!indexer_->isRealTime())
    {
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

    // regenerate term ids for each doc
       resetRand2();
        list<docid_t>::const_iterator updateIt = updateDocList.begin();
        for(docid_t docID = 1; docID <= maxDocID_; ++docID)
        {
            DTermIdMapT docTermIdMap;

            bool isDocUpdated = (docID == *updateIt);
            const unsigned int docLen = docLenRand2_();
            for(unsigned int j = 0; j < docLen; ++j)
            {
                unsigned int termId = termIDRand2_();

                if(isDocUpdated)
                    docTermIdMap[termId].push_back(j);
            }

            if(isDocUpdated)
            {
                queryOneDoc(pTermReader.get(), docID, docTermIdMap, isSkipToRand_, isCheckRand_);

                if(++updateIt == updateDocList.end())
                    break;
            }
        }
    }
    VLOG(2) << "<= TermDocFreqsTestFixture::checkUpdateDocsImpl()";
}

void TermDocFreqsTestFixture::checkStats()
{
    BOOST_TEST_MESSAGE("\nchecking statistics...");

    const int docCount = getDocCount();
    // doc count
    BOOST_TEST_MESSAGE("doc count: " << docCount);
    IndexReader* pIndexReader = indexer_->getIndexReader();
    BOOST_CHECK_EQUAL(pIndexReader->numDocs(), docCount);

    // total term count
    int64_t totalTermNum = 0;
    for(CTermIdMapT::const_iterator it = mapCTermId_.begin();
            it != mapCTermId_.end(); ++it)
    {
        totalTermNum += it->second.second;
    }
    BOOST_TEST_MESSAGE("total term count: " << totalTermNum);
    int64_t docLenSum = 0;
    for(DocIdLenMapT::const_iterator lenMapIt = mapDocIdLen_.begin();
            lenMapIt != mapDocIdLen_.end(); ++lenMapIt)
    {
        docLenSum += pIndexReader->docLength(lenMapIt->first, indexer_->getPropertyIDByName(COLLECTION_ID, INVERTED_FIELD));
    }
    BOOST_CHECK_EQUAL(docLenSum, totalTermNum);

    // average doc length
    if(docCount > 0)
    {
        double avgDocLen = (double)docLenSum / docCount;
        BOOST_TEST_MESSAGE("average doc length: " << avgDocLen);
        BOOST_CHECK_CLOSE(pIndexReader->getAveragePropertyLength(indexer_->getPropertyIDByName(COLLECTION_ID, INVERTED_FIELD)),
                avgDocLen, 0.0001);
    }
    else
    {
        // average doc length is 1 if the collection is empty
        BOOST_CHECK_EQUAL(pIndexReader->getAveragePropertyLength(indexer_->getPropertyIDByName(COLLECTION_ID, INVERTED_FIELD)), 1);
    }

    // unique term count
    BOOST_TEST_MESSAGE("unique term count: " << mapCTermId_.size());
    // wait for merge finish
    indexer_->optimizeIndex();
    indexer_->waitForMergeFinish();
    // reopen index reader
    pIndexReader = indexer_->getIndexReader();
    BOOST_CHECK_EQUAL(pIndexReader->getDistinctNumTerms(COLLECTION_ID, INVERTED_FIELD), mapCTermId_.size());
}

void TermDocFreqsTestFixture::checkTermDocFreqs()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkTermDocFreqs()";

    runToSuccess(boost::bind(&TermDocFreqsTestFixture::checkTermDocFreqsImpl, this));

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermDocFreqs()";
}

void TermDocFreqsTestFixture::checkTermDocFreqsImpl()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkTermDocFreqsImpl()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    if(!indexer_->isRealTime())
    {
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

        if(mapDocIdLen_.empty())
        {
            // TermReader should be NULL when no doc exists
            BOOST_CHECK(pTermReader.get() == NULL);
            VLOG(2) << "<= TermDocFreqsTestFixture::checkTermDocFreqsImpl(), no doc exists";
            return;
        }

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
            if(!pTermDocFreqs) throw std::runtime_error("index dirty");
            BOOST_CHECK_EQUAL(pTermDocFreqs->docFreq(), termIt->second.first);
            BOOST_CHECK_EQUAL(pTermDocFreqs->getCTF(), termIt->second.second);
        }
    }

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermDocFreqsImpl()";
}

void TermDocFreqsTestFixture::checkTermIterator()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkTermIterator()";

    runToSuccess(boost::bind(&TermDocFreqsTestFixture::checkTermIteratorImpl, this));

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermIterator()";
}

void TermDocFreqsTestFixture::checkTermIteratorImpl()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkTermIteratorImpl()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    if(!indexer_->isRealTime())
    {
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

        if(mapDocIdLen_.empty())
        {
        // TermReader should be NULL when no doc exists
            BOOST_CHECK(pTermReader.get() == NULL);
            VLOG(2) << "<= TermDocFreqsTestFixture::checkTermIteratorImpl(), no doc exists";
           return;
        }

#ifdef LOG_TERM_ITERATE
        BOOST_TEST_MESSAGE("check TermIterator on " << mapCTermId_.size() << " terms.");
#endif

        boost::scoped_ptr<TermIterator> pTermIterator(pTermReader->termIterator(INVERTED_FIELD));//xxx
        Term term(INVERTED_FIELD);
        for(CTermIdMapT::const_iterator termIt = mapCTermId_.begin();
            termIt != mapCTermId_.end(); ++termIt)
        {
            BOOST_CHECK(pTermIterator->next());

            const Term* pTerm = pTermIterator->term();
            BOOST_REQUIRE(pTerm);
            const TermInfo* pTermInfo = pTermIterator->termInfo();
            BOOST_REQUIRE(pTermInfo);

#ifdef LOG_TERM_ITERATE
            BOOST_TEST_MESSAGE("term id: " << pTerm->value
                           << ", df: " << pTermInfo->docFreq_
                           << ", ctf: " << pTermInfo->ctf_);
#endif
            BOOST_CHECK_EQUAL(pTerm->value, termIt->first);
            BOOST_CHECK_EQUAL(pTerm->field, INVERTED_FIELD);
            BOOST_CHECK_EQUAL(pTermInfo->docFreq_, termIt->second.first);
            BOOST_CHECK_EQUAL(pTermInfo->ctf_, termIt->second.second);

            term.setValue(termIt->first);
            BOOST_CHECK_EQUAL(pTermReader->docFreq(&term), termIt->second.first);

            pTermInfo = pTermReader->termInfo(&term);
            BOOST_REQUIRE(pTermInfo);

            BOOST_CHECK_EQUAL(pTermInfo->docFreq_, termIt->second.first);
            BOOST_CHECK_EQUAL(pTermInfo->ctf_, termIt->second.second);
        }
    }

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermIteratorImpl()";
}

void TermDocFreqsTestFixture::queryCollection(int threadNum)
{
    assert(threadNum > 0);

    VLOG(2) << "=> TermDocFreqsTestFixture::queryCollection(), threadNum: " << threadNum;

    if(maxDocID_ == 0)
    {
        BOOST_CHECK(isDocEmpty());
        return;
    }

    if(threadNum == 1)
    {
        queryDocs(1, maxDocID_);
        return;
    }

    boost::thread_group threads;
    const docid_t avgDocNum = static_cast<docid_t>(ceil(static_cast<double>(maxDocID_) / threadNum));

    for(int i=0; i<threadNum; ++i)
    {
        docid_t startDocID = avgDocNum * i + 1;
        docid_t endDocID = avgDocNum * (i+1);

        if(startDocID > maxDocID_)
            break;
        else if(endDocID > maxDocID_)
            endDocID = maxDocID_; // limit the doc range for the last thread

        threads.create_thread(boost::bind(&TermDocFreqsTestFixture::queryDocs,
                                          this, startDocID, endDocID));
    }

    threads.join_all();

#ifdef LOG_QUERY_OPERATION
    BOOST_TEST_MESSAGE("TermDocFreqsTestFixture::queryCollection() created " << threads.size() << " threads in collection query.");
#endif

    VLOG(2) << "<= TermDocFreqsTestFixture::queryCollection()";
}

void TermDocFreqsTestFixture::queryDocs(docid_t startDocID, docid_t endDocID)
{
    VLOG(2) << "=> TermDocFreqsTestFixture::queryDocs()";

    runToSuccess(boost::bind(&TermDocFreqsTestFixture::queryDocsImpl, this, startDocID, endDocID));

    VLOG(2) << "<= TermDocFreqsTestFixture::queryDocs()";
}

void TermDocFreqsTestFixture::queryInputDocID()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::queryInputDocID()";

    docid_t targetID = 0;
    while(true)
    {
        cout << "please input doc id to query:" << endl;
        cin >> targetID;

        runToSuccess(boost::bind(&TermDocFreqsTestFixture::queryDocsImpl, this, targetID, targetID));
    }

    VLOG(2) << "<= TermDocFreqsTestFixture::queryInputDocID()";
}

void TermDocFreqsTestFixture::queryDocsImpl(docid_t startDocID, docid_t endDocID)
{
    VLOG(2) << "=> TermDocFreqsTestFixture::queryDocsImpl()"
            << ", startDocID: " << startDocID
            << ", endDocID: " << endDocID;

    if(startDocID < 1 || startDocID > endDocID || endDocID > maxDocID_)
    {
        cerr << "invalid doc range to query: [" << startDocID << ", " << endDocID << "]" << endl;
        VLOG(2) << "<= TermDocFreqsTestFixture::queryDocsImpl(), invalid doc range to query";
        return;
    }

    IndexReader* pIndexReader = indexer_->getIndexReader();
    if (!indexer_->isRealTime())
    {
        boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

        if(pTermReader.get() == NULL)
        {
            BOOST_CHECK_TS(isDocEmpty());
            VLOG(2) << "<= TermDocFreqsTestFixture::queryDocsImpl(), no doc exists and TermReader is NULL";
            return;
        }

    // regenerate term ids for each doc
        boost::mt19937 newEngine;

        IntRandGeneratorT docLenRand2(docLenRand_);
        docLenRand2.engine() = newEngine;

        IntRandGeneratorT termIDRand2(termIDRand_);
        termIDRand2.engine() = newEngine;

        BoolRandGeneratorT isSkipToRand2(isSkipToRand_);
        isSkipToRand2.engine() = newEngine;

        BoolRandGeneratorT isCheckRand2(isCheckRand_);
        isCheckRand2.engine() = newEngine;

        // skip random term ids of docs before the range
        for(docid_t docID = 1; docID < startDocID; ++docID)
        {
            const unsigned int docLen = docLenRand2();
            for(unsigned int j = 0; j < docLen; ++j)
                termIDRand2();
        }

    // query the range
        for(docid_t docID = startDocID; docID <= endDocID; ++docID)
        {
            const unsigned int docLen = docLenRand2();
            DTermIdMapT docTermIdMap;
            for(unsigned int j = 0; j < docLen; ++j)
                docTermIdMap[termIDRand2()].push_back(j);
            queryOneDoc(pTermReader.get(), docID, docTermIdMap, isSkipToRand2, isCheckRand2);
        }
    }
    VLOG(2) << "<= TermDocFreqsTestFixture::queryDocsImpl()";
}

void TermDocFreqsTestFixture::queryOneDoc(TermReader* pTermReader, docid_t docID, const DTermIdMapT& docTermIdMap,
                                          BoolRandGeneratorT& isSkipToRand, BoolRandGeneratorT& isCheckRand) const
{
    bool isDocRemoved = (mapDocIdLen_.find(docID) == mapDocIdLen_.end());

#ifdef LOG_QUERY_OPERATION
    BOOST_TEST_MESSAGE_TS("check TermDocFreqs for doc id: " << docID
            << ", unique terms: " << docTermIdMap.size()
            << ", doc is removed: " << isDocRemoved
            << ", thread id: " << boost::this_thread::get_id());
#endif

    VLOG(5) << "=> TermDocFreqsTestFixture::queryOneDoc(), docID: " << docID
            << ", unique terms: " << docTermIdMap.size()
            << ", doc is removed: " << isDocRemoved;

    Term term(INVERTED_FIELD);
    for(DTermIdMapT::const_iterator termIt = docTermIdMap.begin();
            termIt != docTermIdMap.end();
            ++termIt)
    {
        if(! isCheckRand())
            continue;

#ifdef LOG_TERM_ID
        BOOST_TEST_MESSAGE_TS("check term id: " << termIt->first);
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
                if(!pTermDocFreqs) throw std::runtime_error("index dirty");
                if(isSkipToRand())
                    BOOST_CHECK_EQUAL_TS(pTermDocFreqs->skipTo(docID), BAD_DOCID);
                else
                    BOOST_CHECK_EQUAL_TS(pTermDocFreqs->next(), false);
            }
        }
        else
        {
            BOOST_CHECK_TS(pTermReader->seek(&term));
            boost::scoped_ptr<TermDocFreqs> pTermDocFreqs(pTermReader->termDocFreqs());
            if(!pTermDocFreqs) throw std::runtime_error("index dirty");
            //BOOST_REQUIRE_TS(pTermDocFreqs.get());

            moveToDoc(pTermDocFreqs.get(), docID, isDocRemoved, isSkipToRand());
            if(! isDocRemoved)
               BOOST_CHECK_EQUAL_TS(pTermDocFreqs->freq(), termIt->second.size());

            boost::scoped_ptr<TermPositions> pTermPositions(pTermReader->termPositions());
            if(!pTermPositions) throw std::runtime_error("index dirty");
            //BOOST_REQUIRE_TS(pTermPositions.get());

            moveToDoc(pTermPositions.get(), docID, isDocRemoved, isSkipToRand());
            if(! isDocRemoved)
            {
                BOOST_CHECK_EQUAL_TS(pTermPositions->freq(), termIt->second.size());
                if (testConfig_.indexLevel_ == WORDLEVEL)
                {
                    for(LocListT::const_iterator locIter = termIt->second.begin();
                            locIter != termIt->second.end();
                            ++locIter)
                    {
        #ifdef LOG_TERM_ID
                        BOOST_TEST_MESSAGE_TS("check term position: " << *locIter);
        #endif
                        BOOST_CHECK_EQUAL_TS(pTermPositions->nextPosition(), *locIter);
                    }
                    BOOST_CHECK_EQUAL_TS(pTermPositions->nextPosition(), BAD_POSITION);
                }
            }
        }
    }

    VLOG(5) << "<= TermDocFreqsTestFixture::queryOneDoc()";
}

void TermDocFreqsTestFixture::moveToDoc(TermDocFreqs* pTermDocFreqs, docid_t docID, bool isDocRemoved, bool isSkipTo) const
{
    VLOG(5) << "=> TermDocFreqsTestFixture::moveToDoc(), docID: " << docID
            << ", isDocRemoved: " << isDocRemoved
            << ", isSkipTo: " << isSkipTo;

    if(isDocRemoved)
    {
        if(isSkipTo)
        {
            docid_t skipToResult = pTermDocFreqs->skipTo(docID);
            BOOST_CHECK_TS(skipToResult > docID || skipToResult == BAD_DOCID);
        }
        else
        {
            while(pTermDocFreqs->next())
            {
                if(pTermDocFreqs->doc() >= docID)
                {
                    BOOST_CHECK_NE_TS(pTermDocFreqs->doc(), docID);
                    break;
                }
            }
        }
    }
    else
    {
        if(isSkipTo)
            BOOST_CHECK_EQUAL_TS(pTermDocFreqs->skipTo(docID), docID);
        else
        {
            while(pTermDocFreqs->next())
            {
                if(pTermDocFreqs->doc() >= docID)
                    break;
            }
        }

        BOOST_CHECK_EQUAL_TS(pTermDocFreqs->doc(), docID);
    }

    VLOG(5) << "<= TermDocFreqsTestFixture::moveToDoc()";
}

}
