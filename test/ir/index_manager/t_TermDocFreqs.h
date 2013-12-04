/**
* @file       t_TermDocFreqs.h
* @author     Jun
* @version    SF1 v5.0
* @brief Test cases on @c TermDocFreqs.
*
*/

#ifndef T_TERM_DOC_FREQS_H
#define T_TERM_DOC_FREQS_H

#include "IndexerTestFixture.h"
#include <boost/test/unit_test.hpp>

namespace t_TermDocFreqs
{
/**
 * The fixture used in cases testing TermDocFreqs module.
 */
class TermDocFreqsTestFixture: public IndexerTestFixture
{
public:
    /**
     * Default Constructor.
     */
    TermDocFreqsTestFixture();

    /**
     * Check below statistics.
     * - doc count
     * - total term count
     * - average doc length
     * - unique term count (before checking it, all barrels would be merged into one)
     * @note this function could not be called when any doc is removed or updated,
     * as term stastistics (such as term count and average doc length) are not updated in those situations.
     */
    void checkStats();

    /**
     * Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>docFreq, getCTF</tt>.
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     */
    void checkTermDocFreqs();

    /**
     * Query the whole collection by multiple threads,
     * each thread would query its own doc range.
     * @param threadNum the number of threads
     * @note if @p threadNum is 1, it would query the collection in main thread
     * @note this function waits until the query is finished
     */
    void queryCollection(int threadNum = 10);

    /**
     * It executes in a while loop,
     * in each loop, it gets a doc id from stdin input, and query the terms in that doc id.
     */
    void queryInputDocID();

    /**
     * Check @c TermIterator interfaces, such as <tt>next, term, termInfo</tt>,
     * and also check @c TermReader::termInfo().
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     * @note this function could not be called when any doc is removed or updated,
     * as term stastistics (such as df and ctf) are not updated in those situations.
     */
    void checkTermIterator();

protected:
    /**
     * remove @p removeDocList terms in @c mapCTermId_.
     * @p removeDocument the doc ids which have been removed
     * @pre @p removeDocList should be sorted by docid increasingly
     */
    virtual void removeFixtureDocs(const std::list<docid_t>& removeDocList);

    /**
     * add @p docTermIdMap terms in @c mapCTermId_.
     * @p docTermIdMap the map of term id and its position list in the doc
     */
    virtual void addFixtureDoc(const DTermIdMapT& docTermIdMap);

    /**
     * This function is used to check the doc ids in @p updateDocList have been removed or added.
     * @param updateDocList the doc ids which have been removed or added
     * @pre @p updateDocList should be sorted by docid increasingly
     */
    virtual void checkUpdateDocs(const std::list<docid_t>& updateDocList);

private:
    /** the type of pointer to member function */
    typedef void (TermDocFreqsTestFixture::*PMF_T)();

    /**
     * The implementation for @c checkTermDocFreqs(),
     * it would throw @c IndexManagerException if query failed.
     */
    void checkTermDocFreqsImpl();

    /**
     * The implementation for @c checkTermIterator(),
     * it would throw @c IndexManagerException if query failed.
     */
    void checkTermIteratorImpl();

    /**
     * Query the docs range from @p startDocID to @p endDocID.
     * Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>next, skipTo, freq, doc, nextPosition</tt>.
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     * @param startDocID start of the doc id range to query
     * @param endDocID end of the doc id range to query
     */
    void queryDocs(docid_t startDocID, docid_t endDocID);

    /**
     * The implementation for @c queryDocs(),
     * it would throw @c IndexManagerException if query failed.
     */
    void queryDocsImpl(docid_t startDocID, docid_t endDocID);

    /**
     * Check query on a doc.
     * @param pTermReader TermReader instance
     * @param docID the doc to query
     * @param docTermIdMap the term ids in that doc
     */
    void queryOneDoc(TermReader* pTermReader, docid_t docID, const DTermIdMapT& docTermIdMap,
                     BoolRandGeneratorT& isSkipToRand, BoolRandGeneratorT& isCheckRand) const;

    /**
     * Move the cursor to @p docID using either @p TermDocFreqs::next() or @p TermDocFreqs::skipTo(),
     * these two methods are selected randomly.
     * @param pTermDocFreqs TermDocFreqs instance
     * @param docID move the cursor to the 1st doc id >= @p docID
     * @param isDocRemoved whether @p docID is removed,
     *                     if true, move the cursor to doc id > @p docID or BAD_DOCID,
     *                     if false, move the cursor to doc id == @p docID.
     * @param isSkipTo true to use @c TermDocFreqs::skipTo(), false to use @c TermDocFreqs::next()
     */
    void moveToDoc(TermDocFreqs* pTermDocFreqs, docid_t docID, bool isDocRemoved, bool isSkipTo) const;

    /**
     * Reset random number generator @c docLenRand2_ and @c termIDRand2_.
     */
    void resetRand2();

    /**
     * This implementation for @c checkUpdateDocs(),
     * it would throw @c IndexManagerException if query failed.
     * @param updateDocList the doc ids which have been removed or added
     * @pre @p updateDocList should be sorted by docid increasingly
     */
    void checkUpdateDocsImpl(const std::list<docid_t>& updateDocList);

private:
    ///< termid => (doc freq, collection term freq)
    typedef map<termid_t, pair<freq_t, int64_t> > CTermIdMapT;
    CTermIdMapT mapCTermId_;

    BoolRandGeneratorT isSkipToRand_; ///< in @c moveToDoc(), true to use @c TermDocFreqs::skipTo(), false to use @c TermDocFreqs::next()

    IntRandGeneratorT docLenRand2_; ///< regenerate how many docs in @c queryDocsImpl() and @c removeFixtureDocs()
    IntRandGeneratorT termIDRand2_; ///< regenerate the term ids in @c queryDocsImpl() and @c removeFixtureDocs()
};

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::index";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    for(int i=0; i<config.iterNum_; ++i)
        fixture.createDocument(0,!config.isRealTimeMode());
    fixture.checkTermDocFreqs();
    fixture.checkTermIterator();
    fixture.queryCollection();
    fixture.checkStats();

    VLOG(2) << "<= t_TermDocFreqs::index";
}

inline void changeIndexMode(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::changeIndexMode";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    int iterNum = config.iterNum_;
    int leftNum = 0;

    if(iterNum/2 > 0)
    {
        iterNum = iterNum/2;
        leftNum = config.iterNum_ - iterNum;
    }

    if(leftNum > 0)
    {
        if(!strcasecmp((config.indexMode_).c_str(),"realtime"))
        {
            fixture.getIndexer()->setIndexMode("default");
            fixture.getIndexer()->getIndexManagerConfig()->indexStrategy_.indexMode_ = "default";
        }
        else
        {
            fixture.getIndexer()->setIndexMode("realtime");
            fixture.getIndexer()->getIndexManagerConfig()->indexStrategy_.indexMode_ = "realtime";
        }
    }

    for(int j = 0; j < leftNum; j++)
        fixture.createDocument(0,!config.isRealTimeMode());

    fixture.checkTermDocFreqs();
    fixture.checkTermIterator();
    fixture.queryCollection();
    fixture.checkStats();

    VLOG(2) << "<= t_TermDocFreqs::changeIndexMode";
}

inline void remove(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::remove";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.createDocument();

    for(int i=0; i<config.iterNum_ && !fixture.isDocEmpty(); ++i)
    {
        fixture.removeDocument();

        // as TermDocFreqs::docFreq(), getCTF() fails to update after doc is removed
        // below test is commented out
        //fixture.checkTermDocFreqs();

        fixture.queryCollection();
    }

    VLOG(2) << "<= t_TermDocFreqs::remove";
}

inline void update(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::update";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.createDocument(0,!config.isRealTimeMode());

    for(int i=0; i<config.iterNum_; ++i)
    {
        // each update is checked by TermDocFreqsTestFixture::checkUpdateDocs(),
        // which is called by IndexerTestFixture::updateDocument() below
        fixture.updateDocument();

        // as TermDocFreqs::docFreq(), getCTF() fails to update after doc is removed
        // below test is commented out
        //fixture.checkTermDocFreqs();
    }

    // check whole collection
    fixture.queryCollection();

    VLOG(2) << "<= t_TermDocFreqs::update";
}

inline void empty(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::empty";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.checkTermDocFreqs();
    fixture.queryCollection();
    fixture.checkStats();

    VLOG(2) << "<= t_TermDocFreqs::empty";
}

/**
 * this case is to test:
 * remove some documents,
 * then merge all barrels into one.
 */
inline void removeDocAndOptimize(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::removeDocAndOptimize";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    const int barrelNum = config.iterNum_;
    for(int i=0; i<barrelNum; ++i)
        fixture.createDocument(); // create barrel i

    fixture.removeDocument();

    Indexer* pIndexer = fixture.getIndexer();
    pIndexer->optimizeIndex();

    // wait for merge finish
    pIndexer->waitForMergeFinish();

    fixture.queryCollection();

    IndexReader* pIndexReader = pIndexer->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), fixture.getMaxDocID());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), fixture.getDocCount());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);

    VLOG(2) << "<= t_TermDocFreqs::removeDocAndOptimize";
}
/**
 * Create barrels, optimize barrels (it would clear Bitset),
 * and remove document (it would set Bitset),
 * then check Bitset
 */
inline void optimizeAndRemoveDoc(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::optimizeAndRemoveDoc";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    const int barrelNum = config.iterNum_;
    for(int i=0; i<barrelNum; ++i)
        fixture.createDocument(); // create barrel i

    Indexer* pIndexer = fixture.getIndexer();
    pIndexer->optimizeIndex();

    fixture.removeDocument();

    // wait for merge finish
    pIndexer->waitForMergeFinish();

    fixture.queryCollection();

    IndexReader* pIndexReader = pIndexer->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), fixture.getMaxDocID());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), fixture.getDocCount());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);

    VLOG(2) << "<= t_TermDocFreqs::optimizeAndRemoveDoc";
}

/**
 * this case is to test:
 * create documents containing only one term "1",
 * then remove all of them.
 */
inline void removeOneTerm(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::removeOneTerm, config.docNum_: " << config.docNum_;
    BOOST_TEST_MESSAGE("config.docNum_: " << config.docNum_);

    IndexerTestConfig newConfig(config);
    unsigned int docNum = config.docNum_;
    newConfig.docNum_ = 0; // to make each doc created containing only one term "1"

    TermDocFreqsTestFixture fixture;
    fixture.configTest(newConfig);

    fixture.createDocument(docNum);
    while(! fixture.isDocEmpty())
    {
        fixture.removeDocument();

        fixture.queryCollection();
    }

    VLOG(2) << "<= t_TermDocFreqs::removeOneTerm";
}

/**
 * this case is to test:
 * remove documents,
 * start index manager again,
 * check documents are removed
 */
inline void checkRemoveAtStartUp(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::checkRemoveAtStartUp";

    {
        TermDocFreqsTestFixture fixture;
        fixture.configTest(config);

        fixture.createDocument();
        fixture.removeDocument();

        fixture.queryCollection();
    }

    {
        // new Indexer instance,
        // while keep original index files
        TermDocFreqsTestFixture fixture;
        fixture.setRealIndex(false);
        fixture.configTest(config);

        // re-generate random numbers to check
        fixture.createDocument();
        fixture.removeDocument();

        fixture.queryCollection();

        // create new index files
        fixture.setRealIndex(true);
        // remove again
        fixture.removeDocument();
        fixture.queryCollection();
    }

    VLOG(2) << "<= t_TermDocFreqs::checkRemoveAtStartUp";
}

}
#endif
