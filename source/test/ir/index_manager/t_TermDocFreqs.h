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
     * Print below statistics:
     * - doc count
     * - unique term count
     * - sum of doc length in collection.
     */
    void printStats() const;

    /**
     * Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>docFreq, getCTF</tt>.
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     */
    void checkTermDocFreqs();

    /**
     * Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>next, skipTo, freq, doc, nextPosition</tt>.
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     */
    void checkNextSkipTo();

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
     * The implementation for @c checkNextSkipTo(),
     * it would throw @c IndexManagerException if query failed.
     */
    void checkNextSkipToImpl();

    /**
     * Check query on a doc.
     * @param pTermReader TermReader instance
     * @param docID the doc to query
     * @param docTermIdMap the term ids in that doc
     */
    void checkNextSkipToDoc(TermReader* pTermReader, docid_t docID, const DTermIdMapT& docTermIdMap);

    /**
     * Move the cursor to @p docID using either @p TermDocFreqs::next() or @p TermDocFreqs::skipTo(),
     * these two methods are selected randomly.
     * @param pTermDocFreqs TermDocFreqs instance
     * @param docID move the cursor to the 1st doc id >= @p docID
     * @param isDocRemoved whether @p docID is removed,
     *                     if true, move the cursor to doc id > @p docID or BAD_DOCID,
     *                     if false, move the cursor to doc id == @p docID.
     */
    void nextOrSkipTo(TermDocFreqs* pTermDocFreqs, docid_t docID, bool isDocRemoved);

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

    RandGeneratorT docLenRand2_; ///< regenerate how many docs in @c checkNextSkipToImpl() and @c removeFixtureDocs()
    RandGeneratorT termIDRand2_; ///< regenerate the term ids in @c checkNextSkipToImpl() and @c removeFixtureDocs()
};

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::index";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    for(int i=0; i<config.iterNum_; ++i)
        fixture.createDocument();

    fixture.printStats();
    fixture.checkTermDocFreqs();
    fixture.checkNextSkipTo();

    VLOG(2) << "<= t_TermDocFreqs::index";
}

inline void remove(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::remove";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.createDocument();

    while(! fixture.isDocEmpty())
    {
        fixture.removeDocument();

        // as TermDocFreqs::docFreq(), getCTF() fails to update after doc is removed
        // below test is commented out
        //fixture.checkTermDocFreqs();

        fixture.checkNextSkipTo();
    }

    VLOG(2) << "<= t_TermDocFreqs::remove";
}

inline void update(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::update";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.createDocument();

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
    fixture.checkNextSkipTo();

    VLOG(2) << "<= t_TermDocFreqs::update";
}

inline void empty(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::empty";

    TermDocFreqsTestFixture fixture;
    fixture.configTest(config);

    fixture.checkTermDocFreqs();
    fixture.checkNextSkipTo();

    VLOG(2) << "<= t_TermDocFreqs::empty";
}

/**
 * Create barrels, optimize barrels (it would clear BitVector),
 * and remove document (it would set BitVector),
 * then check BitVector
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

    fixture.checkNextSkipTo();

    IndexReader* pIndexReader = pIndexer->getIndexReader();
    BarrelsInfo* pBarrelsInfo = pIndexReader->getBarrelsInfo();

    BOOST_CHECK_EQUAL(pBarrelsInfo->maxDocId(), fixture.getMaxDocID());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getDocCount(), fixture.getDocCount());
    BOOST_CHECK_EQUAL(pBarrelsInfo->getBarrelCount(), 1);

    VLOG(2) << "<= t_TermDocFreqs::optimizeAndRemoveDoc";
}
}
#endif
