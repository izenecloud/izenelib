#include <sstream> // std::ostringstream
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermReader.h>

#include "t_TermDocFreqs.h"

//#define LOG_TERM_ID
//#define LOG_QUERY_OPERATION
//#define QUERY_ON_STDIN_DOCID

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
        catch(IndexManagerException& e)
        {
            LOG(ERROR) << "start query again as exception found: " << e.what();
        }
    }
}
}

namespace t_TermDocFreqs
{
TermDocFreqsTestFixture::TermDocFreqsTestFixture()
    :docLenRand2_(docLenRand_)
    ,termIDRand2_(termIDRand_)
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
                             cref(updateDocList)));

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
            checkNextSkipToDoc(pTermReader.get(), docID, docTermIdMap);

            if(++updateIt == updateDocList.end())
                break;
        }
    }

    VLOG(2) << "<= TermDocFreqsTestFixture::checkUpdateDocsImpl()";
}

void TermDocFreqsTestFixture::printStats() const
{
    BOOST_TEST_MESSAGE("\nprinting statistics...");
    BOOST_TEST_MESSAGE("doc count: " << getDocCount());
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
        BOOST_CHECK_EQUAL(pTermDocFreqs->docFreq(), termIt->second.first);
        BOOST_CHECK_EQUAL(pTermDocFreqs->getCTF(), termIt->second.second);
    }

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermDocFreqsImpl()";
}

void TermDocFreqsTestFixture::checkNextSkipTo()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkNextSkipTo()";

    runToSuccess(boost::bind(&TermDocFreqsTestFixture::checkNextSkipToImpl, this));

    VLOG(2) << "<= TermDocFreqsTestFixture::checkNextSkipTo()";
}

void TermDocFreqsTestFixture::checkNextSkipToImpl()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkNextSkipToImpl()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

    if(pTermReader.get() == NULL)
    {
        BOOST_CHECK(isDocEmpty());
        VLOG(2) << "<= TermDocFreqsTestFixture::checkNextSkipToImpl(), no doc exists and TermReader is NULL";
        return;
    }

#ifdef QUERY_ON_STDIN_DOCID
    docid_t targetID = 0;
    while(true)
    {
        cout << "please input doc id to query:" << endl;
        cin >> targetID;
        // regenerate term ids for each doc
        resetRand2();
        for(docid_t docID = 1; docID <= maxDocID_; ++docID)
        {
            DTermIdMapT docTermIdMap;

            const unsigned int docLen = docLenRand2_();
            for(unsigned int j = 0; j < docLen; ++j)
            {
                unsigned int termId = termIDRand2_();
                docTermIdMap[termId].push_back(j);
            }

            if(docID == targetID)
            {
                checkNextSkipToDoc(pTermReader.get(), docID, docTermIdMap);
                break;
            }
        }
    }
#else
    // regenerate term ids for each doc
    resetRand2();
    for(docid_t docID = 1; docID <= maxDocID_; ++docID)
    {
        DTermIdMapT docTermIdMap;

        const unsigned int docLen = docLenRand2_();
        for(unsigned int j = 0; j < docLen; ++j)
        {
            unsigned int termId = termIDRand2_();
            docTermIdMap[termId].push_back(j);
        }

        checkNextSkipToDoc(pTermReader.get(), docID, docTermIdMap);
    }
#endif

    VLOG(2) << "<= TermDocFreqsTestFixture::checkNextSkipToImpl()";
}

void TermDocFreqsTestFixture::checkNextSkipToDoc(TermReader* pTermReader, docid_t docID, const DTermIdMapT& docTermIdMap)
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

void TermDocFreqsTestFixture::nextOrSkipTo(TermDocFreqs* pTermDocFreqs, docid_t docID, bool isDocRemoved)
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

}
