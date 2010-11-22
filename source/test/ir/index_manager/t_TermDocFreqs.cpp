#include <sstream> // std::ostringstream
#include <boost/test/unit_test.hpp>

#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermReader.h>

#include "t_TermDocFreqs.h"

//#define LOG_TERM_ID
//#define LOG_QUERY_OPERATION

namespace t_TermDocFreqs
{
void TermDocFreqsTestFixture::removeFixtureDocs(const std::list<docid_t>& removeDocList)
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

    VLOG(2) << "<= TermDocFreqsTestFixture::checkTermDocFreqs()";
}

void TermDocFreqsTestFixture::checkNextSkipTo()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkNextSkipTo()";

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

    VLOG(2) << "<= TermDocFreqsTestFixture::checkNextSkipTo()";
}

void TermDocFreqsTestFixture::checkNextSkipToImpl()
{
    VLOG(2) << "=> TermDocFreqsTestFixture::checkNextSkipToImpl()";

    IndexReader* pIndexReader = indexer_->getIndexReader();
    boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

    if(mapDocIdLen_.empty())
    {
        // TermReader should be NULL when no doc exists
        BOOST_CHECK(pTermReader.get() == NULL);
        VLOG(2) << "<= TermDocFreqsTestFixture::checkNextSkipToImpl(), no doc exists";
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
