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

    IndexLevel indexLevel_;

    std::string indexMode_; ///< "default" (for offline), "realtime", "default:block", "default:chunk"

    bool isMerge_; ///< true for merge index in stand-alone thread, false for not to merge at all

    /**
     * skip interval.
     * for chunk type, it is fixed to CHUNK_SIZE (128) docs,
     * for block type, it is fixed to BLOCK_SIZE (8192) bytes.
     */
    int skipInterval_; ///<

    /**
     * max skip level.
     * for vint and chunk type, 0 for no skip data generated, positive value for the number of skip levels,
     * for block type, it always uses one level skip list.
     */
    int maxSkipLevel_; ///< max skip level

    /**
     * the percentage of check statements executed, such as @c BOOST_CHECK_* etc.
     * it is a real value in [0..1], used to sample the check statements to save testing time on large data size.
     * example:
     * value 1.0 for all check statements would be executed,
     * value 0.5 for half of check statments would be chosen randomly,
     */
    double checkPercent_;

    /**
     * Get string which outputs each parameters.
     * @return output string
     */
    std::string str() const {
        std::ostringstream oss;
        oss << "IndexerTestConfig " << configNum_ << " (docNum_: " << docNum_
            << ", iterNum_: " << iterNum_ << ", indexLevel_: "<< indexLevel_<<", indexMode_: " << indexMode_
            << ", isMerge_: " << isMerge_
            << ", skipInterval_: " << skipInterval_
            << ", maxSkipLevel_: " << maxSkipLevel_
            << ", checkPercent_: " << checkPercent_
            << ")";

        return oss.str();
    }

    /**
     * Get index working mode
     * @return true for realtime mode
     */
    bool isRealTimeMode() const{
        return indexMode_ == "realtime";
    }
    /**
     * Whether @c docNum_ is large,
     * so that we need set an upper bound for doc length, sample check statements, etc.
     * @return true for large, false for small.
     */
    bool isDocNumLarge() const {
        return docNum_ >= 1000;
    }
};

/**
 * The fixture used in cases testing Indexer module,
 * it would set up Indexer config at construction,
 * and delete Indexer at destruction.
 */
class IndexerTestFixture
{
public:
    static const char* INDEX_FILE_DIR;
    static const unsigned int COLLECTION_ID;
    static const char* INDEX_MODE_REALTIME;
    static const char* INVERTED_FIELD;

protected:
    Indexer* indexer_;

    IndexerTestConfig testConfig_;

    std::map<std::string, unsigned int> propertyMap_;

    unsigned int newDocNum_; ///< the max number of documents created in createDocument()
    boost::mt19937 randEngine_;
    typedef boost::variate_generator<boost::mt19937, boost::uniform_int<> > IntRandGeneratorT;
    IntRandGeneratorT docLenRand_; ///< decide how many terms are created in each doc of @c prepareDocument()
    IntRandGeneratorT termIDRand_; ///< decide the term ids in each doc of @c prepareDocument()
    IntRandGeneratorT docNumRand_; ///< decide how many docs are updated in @c updateDocument() and how many docs are removed in @c removeDocument()

    typedef boost::variate_generator<boost::mt19937, boost::bernoulli_distribution<> > BoolRandGeneratorT;
    BoolRandGeneratorT isCheckRand_; ///< decide whether to execute check statements for large data size, true for execute, false for not to execute

    /**
     * as the max doc id in indexer is the max doc id ever indexed,
     * no matter whether that doc is deleted,
     * we have to maintain that max doc id here.
     */
    docid_t maxDocID_;

    ///< docid => doc length
    typedef map<docid_t, size_t> DocIdLenMapT;
    DocIdLenMapT mapDocIdLen_;

    ///< term position list
    typedef vector<loc_t> LocListT;
    ///< termid => term position list of one doc
    typedef map<termid_t, LocListT> DTermIdMapT;

    /**
     * True to clear index files in @c configTest() and create true index files,
     * false to keep original index files in and not create true index files,
     * it just re-generates the random numbers to check.
     */
    bool isRealIndex_;

public:
    /**
     * Default Constructor.
     * @note in each test case, you must also call @c configTest() to configure test parameter.
     */
    IndexerTestFixture();

    /** Destroy Indexer resource. */
    virtual ~IndexerTestFixture();

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
     * Set the flag of @c isRealIndex_.
     * @param isReal true for real index, false for not real index.
     */
    void setRealIndex(bool isReal) { isRealIndex_ = isReal; }

    /**
     * Get the flag of @c isRealIndex_.
     * @return true for real index, false for not real index.
     */
    bool isRealIndex() const { return isRealIndex_; }

    /**
     * Whether documents exist in current test status.
     * @return true for no document exists, false for there are some documents.
     */
    bool isDocEmpty() const { return mapDocIdLen_.empty(); }

    /**
     * Create @p docNum documents.
     * @param docNum the number of docs to create
     * @param manualFlush whether call flush operation of indexer. It's not necessary when indexer works under realtime mode
     * @note if @p docNum is 0, @c IndexerTestConfig::docNum_ would be used as @p docNum instead
     */
    void createDocument(unsigned int docNum = 0, bool manualFlush = true);

    /** Update random number of documents. */
    void updateDocument(bool manualFlush = true);

    /** Remove random number of documents, and also remove documents exceed max doc id. */
    void removeDocument();

    Indexer* getIndexer() { return indexer_; }
    docid_t getMaxDocID() const { return maxDocID_; }
    int getDocCount() const { return mapDocIdLen_.size(); }

protected:
    /**
     * This function denotes the doc ids in @p removeDocList have been removed,
     * it is called in @c updateDocument() and @c removeDocument(),
     * it is implemented as empty method in this class,
     * while those classes inheriting from it could override this method.
     * @param removeDocList the doc ids which have been removed
     * @pre @p removeDocList should be sorted by docid increasingly
     */
    virtual void removeFixtureDocs(const std::list<docid_t>& removeDocList) {}

    /**
     * This function denotes the doc containing terms in @p docTermIdMap has been added,
     * it is called in @c prepareDocument(),
     * it is implemented as empty method in this class,
     * while those classes inheriting from it could override this method.
     * @param docTermIdMap the map of term id and its position list in the doc
     */
    virtual void addFixtureDoc(const DTermIdMapT& docTermIdMap) {}

    /**
     * This function is used to check the doc ids in @p updateDocList have been removed or added,
     * It is called in @c updateDocument(),
     * it is implemented as empty method in this class,
     * while those classes inheriting from it could override this method.
     * @param updateDocList the doc ids which have been removed or added
     * @pre @p updateDocList should be sorted by docid increasingly
     */
    virtual void checkUpdateDocs(const std::list<docid_t>& updateDocList) {}

private:
    /**
     * Get a random number in the range of [1, current doc size].
     * @return the random doc number
     */
    int randDocNum();

    void removeIndexFiles();

    void prepareDocument(IndexerDocument& document, unsigned int docId, bool filter = true);
};

#endif
