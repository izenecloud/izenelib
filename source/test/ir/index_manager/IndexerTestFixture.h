/**
* @file       IndexerTestFixture.h
* @author     Jun
* @version    SF1 v5.0
* @brief Fixture to test Indexer module.
*
*/

#ifndef INDEXER_TEST_FIXTURE_H
#define INDEXER_TEST_FIXTURE_H

#include <string>
#include <map>
#include <list>
#include <utility> // std::pair
#include <sstream> // std::ostringstream
#include <boost/random.hpp>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/utility/system.h>

using namespace izenelib::ir::indexmanager;

/**
 * The parameters in testing Indexer module.
 */
struct IndexerTestConfig
{
    int configNum_; ///< the sequence number of current config

    unsigned int docNum_; ///< the number of documents created in @c IndexerTestFixture::createDocument()

    int iterNum_; ///< the iteration number used in test case, such as iterate calling @c IndexerTestFixture::createDocument()

    std::string indexMode_; ///< "default" (for offline), "realtime", "default:block", "default:chunk"

    bool isMerge_; ///< true for merge index in stand-alone thread, false for not to merge at all

    /**
     * Get string which outputs each parameters.
     * @return output string
     */
    std::string str() const {
        std::ostringstream oss;
        oss << "config " << configNum_ << " (docNum_: " << docNum_
            << ", iterNum_: " << iterNum_ << ", indexMode_: " << indexMode_
            << ", isMerge_: " << isMerge_ << ")";

        return oss.str();
    }
};

/**
 * The fixture used in cases testing Indexer module,
 * it would set up Indexer config at construction,
 * and delete Indexer at destruction.
 */
class IndexerTestFixture
{
protected:
    Indexer* indexer_;

    IndexerTestConfig testConfig_;

    std::map<std::string, unsigned int> propertyMap_;

    unsigned int newDocNum_; ///< the max number of documents created in createDocument()
    boost::mt19937 randEngine_;
    typedef boost::variate_generator<mt19937, uniform_int<> > RandGeneratorT;
    RandGeneratorT docLenRand_;
    RandGeneratorT termIDRand_;
    RandGeneratorT skipToRand_; ///< in @p nextOrSkipTo(), 1 to use @p TermDocFreqs::skipTo(), 0 to use @p TermDocFreqs::next()

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
    /**
     * Default Constructor.
     * @note in each test case, you must also call @c configTest() to configure test parameter.
     */
    IndexerTestFixture();

    /** Destroy Indexer resource. */
    ~IndexerTestFixture();

    /**
     * Configure test parameters.
     * including:
     * the number of documents to create,
     * term id random number generator,
     * Indexer parameters such as index mode, merge strategy, etc
     * @param testConfig the test parameters to configure
     * @note this function must be called to configure test parameter in each test case.
     */
    void configTest(const IndexerTestConfig& testConfig);

    /**
     * Get the test parameters used in previous call of @c configTest().
     * @return the test parameters
     */
    const IndexerTestConfig& getTestConfig() const { return testConfig_; }

    /**
     * Destroy the Indexer instance, and create a new one.
     * This function is used to simulate restarting Indexer.
     */
    void renewIndexer();

    /**
     * Whether documents exist in current test status.
     * @return true for no document exists, false for there are some documents.
     */
    bool isDocEmpty() const { return mapDocIdLen_.empty(); }

    /**
     * Print below statistics:
     * - doc count
     * - unique term count
     * - sum of doc length in collection.
     */
    void printStats() const;

    /** Only create \e newDocNum_ documents. */
    void createDocument();

    /** Update random number of documents. */
    void updateDocument();

    /** Remove random number of documents, and also remove documents exceed max doc id. */
    void removeDocument();

    /** Check document length. */
    void checkDocLength();

    /** Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>docFreq, getCTF</tt>. */
    void checkTermDocFreqs();

    /**
     * Check @c TermReader::seek() and @c TermDocFreqs interfaces, such as <tt>next, skipTo, freq, doc, nextPosition</tt>.
     * If @c IndexManagerException is thrown during query, it would catch it and query again.
     */
    void checkNextSkipTo();

    /**
     * Create barrels and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void checkBarrel(int barrelNum);

    /**
     * Create barrels, optimize barrels (merge them into one), and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void optimizeBarrel(int barrelNum);

    /**
     * Create barrels, optimize barrels (merge them into one), and check barrel status.
     * @param barrelNum the number of barrels to create
     */
    void createAfterOptimizeBarrel(int barrelNum);

    /**
     * Pause and resume the merge.
     * @param barrelNum the number of barrels to create
     */
    void pauseResumeMerge(int barrelNum);

private:
    /**
     * For each doc in @p removeDocList, remove all its terms in @p mapCTermId_.
     * @pre @p removeDocList should be sorted by docid increasingly
     */
    void removeDocTerms(const list<docid_t>& removeDocList);

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

    void removeIndexFiles();

    void prepareDocument(IndexerDocument& document, unsigned int docId, bool filter = true);
};

#endif
