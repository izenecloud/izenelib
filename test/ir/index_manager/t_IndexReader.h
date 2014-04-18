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
       if(!indexer_->isRealTime())
       {
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
        }

        VLOG(2) << "<= IndexReaderTestFixture::checkDocLength()";
    }

    /** Check @c Bitset, it is used to record which doc is removed. */
    void checkDocFilter() {
        VLOG(2) << "=> IndexReaderTestFixture::checkDocFilter()";

        IndexReader* pIndexReader = indexer_->getIndexReader();
        Bitset* pDocFilter = pIndexReader->getDocFilter();
        Directory* pDirectory = indexer_->getDirectory();

        // no doc is deleted
        if(getMaxDocID() == static_cast<unsigned int>(getDocCount()))
        {
            // file "docs.del" should not exist
            BOOST_CHECK(! pDirectory->fileExists(DELETED_DOCS));
            // Bitset instance should not be created
            BOOST_CHECK(pDocFilter == NULL);
        }
        else
        {
            // file "docs.del" should exist
            BOOST_CHECK(pDirectory->fileExists(DELETED_DOCS));
            // Bitset instance should be created
            BOOST_CHECK(pDocFilter != NULL);
            // Bitset size should be <= (maxDocID + 4), in case of Bitset::grow()
            BOOST_CHECK_GT(pDocFilter->size(), 0U);
            BOOST_CHECK_LE(pDocFilter->size(), getMaxDocID() + 4);
        }

        VLOG(2) << "<= IndexReaderTestFixture::checkDocFilter()";
    }
};

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::index";

    {
        IndexReaderTestFixture fixture;
        fixture.configTest(config);

        // create barrels
        for(int i=0; i<config.iterNum_; ++i)
        {
            fixture.createDocument();
            fixture.checkDocLength();
            fixture.checkDocFilter();
        }
    }

    {
        // new Indexer instance,
        // while keep original index files
        IndexReaderTestFixture fixture;
        fixture.setRealIndex(false);

        IndexerTestConfig newConfig = config;

        // change from offline mode to realtime mode,
        // only VInt type is supported for realtime mode
        if(config.indexMode_ == "default")
            newConfig.indexMode_ = "realtime";
        fixture.configTest(newConfig);

        // re-generate random numbers to check
        for(int i=0; i<config.iterNum_; ++i)
            fixture.createDocument();
        fixture.checkDocLength();
        fixture.checkDocFilter();

        // create new index files
        fixture.setRealIndex(true);
        for(int i=0; i<config.iterNum_; ++i)
        {
            fixture.createDocument();
            fixture.checkDocLength();
            fixture.checkDocFilter();
        }
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
        fixture.checkDocFilter();
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
        fixture.checkDocFilter();
    }

    VLOG(2) << "<= t_IndexReader::remove";
}

inline void empty(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::empty";

    IndexReaderTestFixture fixture;
    fixture.configTest(config);

    fixture.checkDocLength();
    fixture.checkDocFilter();

    VLOG(2) << "<= t_IndexReader::empty";
}

}
#endif
