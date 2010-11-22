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
    typedef boost::variate_generator<mt19937, uniform_int<> > RandGeneratorT;
    RandGeneratorT docLenRand_; ///< decide how many docs are created in @c prepareDocument()
    RandGeneratorT termIDRand_; ///< decide the term ids in creating docs in @c prepareDocument()
    RandGeneratorT docNumRand_; ///< decide how many docs are updated in @c updateDocument() and how many docs are removed in @c removeDocument()
    RandGeneratorT skipToRand_; ///< in @c nextOrSkipTo(), 1 to use @c TermDocFreqs::skipTo(), 0 to use @c TermDocFreqs::next()

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

    /** Only create \e newDocNum_ documents. */
    void createDocument();

    /** Update random number of documents. */
    void updateDocument();

    /** Remove random number of documents, and also remove documents exceed max doc id. */
    void removeDocument();

    Indexer* getIndexer() { return indexer_; }
    docid_t getMaxDocID() const { return maxDocID_; }
    unsigned int getDocCount() const { return mapDocIdLen_.size(); }

protected:
    /**
     * This function denotes the doc ids in @p removeDocList have been removed,
     * it is called in @c updateDocument() and @c removeDocument(),
     * it is implemented as empty method in this class,
     * while those classes inheriting from it could override this method.
     * @p removeDocument the doc ids which have been removed
     * @pre @p removeDocList should be sorted by docid increasingly
     */
    virtual void removeFixtureDocs(const std::list<docid_t>& removeDocList) {}

    /**
     * This function denotes the doc containing terms in @p docTermIdMap has been added,
     * it is called in @c prepareDocument(),
     * it is implemented as empty method in this class,
     * while those classes inheriting from it could override this method.
     * @p docTermIdMap the map of term id and its position list in the doc
     */
    virtual void addFixtureDoc(const DTermIdMapT& docTermIdMap) {}

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
