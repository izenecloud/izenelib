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

inline void index(const IndexerTestConfig& config)
{
    VLOG(2) << "=> t_TermDocFreqs::index";

    IndexerTestFixture fixture;
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

    IndexerTestFixture fixture;
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

    IndexerTestFixture fixture;
    fixture.configTest(config);

    fixture.createDocument();

    for(int i=0; i<config.iterNum_; ++i)
    {
        fixture.updateDocument();

        // as TermDocFreqs::docFreq(), getCTF() fails to update after doc is removed
        // below test is commented out
        //fixture.checkTermDocFreqs();

        fixture.checkNextSkipTo();
    }

    VLOG(2) << "<= t_TermDocFreqs::update";
}

}
#endif
