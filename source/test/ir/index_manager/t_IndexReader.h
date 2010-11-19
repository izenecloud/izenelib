/**
* @file       t_IndexReader.h
* @author     Jun
* @version    SF1 v5.0
* @brief Test cases on @c IndexReader.
*
*/

#ifndef T_INDEX_READER_H
#define T_INDEX_READER_H

#include "IndexerTestFixture.h"

namespace t_IndexReader
{

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_IndexReader::index";

    IndexerTestFixture fixture;
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

    IndexerTestFixture fixture;
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

    IndexerTestFixture fixture;
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

}
#endif
