/**
* @file       t_IndexReader.h
* @author     Jun
* @version    SF1 v5.0
* @brief Test cases on @c IndexReader.
*
*/

#ifndef T_INDEX_READER_H
#define T_INDEX_READER_H

#include <glog/logging.h>
#include <boost/test/unit_test.hpp>

#include <ir/index_manager/index/IndexReader.h>

#include "IndexerTestFixture.h"

//#define LOG_CHECK_OPERATION

namespace t_IndexReader
{
/**
 * The fixture used in cases testing IndexReader module.
 */
class IndexReaderTestFixture: public IndexerTestFixture
{
public:
    /** Check document length. */
    void checkDocLength() {
        VLOG(2) << "=> IndexReaderTestFixture::checkDocLength()";

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

        VLOG(2) << "<= IndexReaderTestFixture::checkDocLength()";
    }
};

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::index";

    IndexReaderTestFixture fixture;
    fixture.configTest(config);

    // create barrels
    for(int i=0; i<config.iterNum_; ++i)
    {
        fixture.createDocument();
        fixture.checkDocLength();
    }

    // new Indexer instance
    fixture.renewIndexer();
    //using "realtime" mode
    IndexerTestConfig newConfig = config;
    newConfig.indexMode_ = "realtime";
    fixture.configTest(newConfig);

    fixture.checkDocLength();
    for(int i=0; i<config.iterNum_; ++i)
    {
        fixture.createDocument();
        fixture.checkDocLength();
    }
    VLOG(2) << "<= t_IndexReader::index";
}

inline void update(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::update";

    IndexReaderTestFixture fixture;
    fixture.configTest(config);
    fixture.createDocument();

    for(int i=0; i<config.iterNum_; ++i)
    {
        fixture.updateDocument();
        fixture.checkDocLength();
    }

    VLOG(2) << "<= t_IndexReader::update";
}

inline void remove(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::remove";

    IndexReaderTestFixture fixture;
    fixture.configTest(config);
    fixture.createDocument();

    while(! fixture.isDocEmpty())
    {
        fixture.removeDocument();
        fixture.checkDocLength();
    }
    fixture.checkDocLength();

    VLOG(2) << "<= t_IndexReader::remove";
}

inline void empty(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::empty";

    IndexReaderTestFixture fixture;
    fixture.configTest(config);

    fixture.checkDocLength();

    VLOG(2) << "<= t_IndexReader::empty";
}

}
#endif
