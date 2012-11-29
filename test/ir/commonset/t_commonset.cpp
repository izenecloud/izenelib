#include <iostream>
#include <vector>

#include <ir/commonset/documentproperties.hpp>
#include "queryprocessormanagersimple.hpp"
#include "queryprocessormanagersimdbitmapsimple.hpp"
#include "indexsimple.hpp"
#include "indexsimdbitmapsimple.hpp"
#include "posixtimer.hpp"


#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE( t_commonset )

BOOST_AUTO_TEST_CASE(index)
{
// index parameters

    unsigned int ntokens_cached = 2;
    unsigned int ntokens_disk = 0;
    std::vector<unsigned int> ndocuments;


//parameter set for testing
    /*
      bool verbose = true;
      unsigned int ndocuments_total = 1280;
      unsigned int nblocks = 5;

      unsigned int ndocuments_common = 2;
      ndocuments.push_back(4);
      ndocuments.push_back(2);
    */

    bool verbose = false;
    unsigned int ndocuments_total = 12800000;
    unsigned int nblocks = 10000;

    unsigned int ndocuments_common = 100000;

    ndocuments.push_back(1000000);
    ndocuments.push_back(1000000);


// query parameters

    unsigned int nresults_start = 0;
    unsigned int nresults_end = 1000000;

    std::vector<unsigned int> tokens;
    std::vector<std::string> operators;

    tokens.push_back(0);
    operators.push_back( "AND" );
    tokens.push_back(1);

// timing parameters

    unsigned int nquery_repeats = 100;

// simd index parameters

    unsigned int nregisters = 128;



    std::cout << "\nindextester.cpp : creating dummy index data.\n\n";

    izenelib::ir::commonset::IndexSIMDBitMapSimple indexsimple( nblocks, nregisters, ndocuments_total, ntokens_cached, ntokens_disk, ndocuments_common, ndocuments, "./", verbose );

    izenelib::ir::commonset::DocumentProperties<> document_properties( ndocuments_total, true );

    izenelib::ir::commonset::Query<> query( tokens, operators, nresults_start, nresults_end );


    izenelib::ir::commonset::QueryProcessorManagerSimple<> queryprocessormanager_simple( indexsimple, document_properties );

    izenelib::ir::commonset::QueryProcessorManagerSIMDBitMapSimple<> queryprocessormanager_simdbitmapsimple( indexsimple, document_properties );

    queryprocessormanager_simple.setQuery( query );

    queryprocessormanager_simdbitmapsimple.setQuery( query );



    std::cout << "indextester.cpp : verifying index result is correct..." << std::flush;

    queryprocessormanager_simple.computeCommonSet();

    bool ok_simple = indexsimple.checkResult( queryprocessormanager_simple.getCommonSet() );

    queryprocessormanager_simdbitmapsimple.computeCommonSet();

    bool ok_simd = indexsimple.checkResult( queryprocessormanager_simdbitmapsimple.getCommonSet() );

    std::cout << "done.\n\n";

    if( ok_simple ) std::cout << "  simple index result ok";
    else std::cout << "  simple index result not correct";

    if(verbose)
    {
        unsigned int ndocs = queryprocessormanager_simple.getCommonSet().getNResults();
        std::cout << ", common set [" << ndocs << "] : " << std::flush;
        for( unsigned int idoc = 0 ; idoc < ndocs ; ++idoc ) std::cout << *(queryprocessormanager_simple.getCommonSet().getDocID(idoc)) << " ";
    }

    if( ok_simd ) std::cout << "\n\n  simple simd bitmap result ok";
    else std::cout << "\n\n  simple simd bitmap result not correct";

    if(verbose)
    {
        unsigned int ndocs = queryprocessormanager_simdbitmapsimple.getCommonSet().getNResults();
        std::cout << ", common set [" << ndocs << "] : " << std::flush;
        for( unsigned int idoc = 0 ; idoc < ndocs ; ++idoc ) std::cout << *(queryprocessormanager_simdbitmapsimple.getCommonSet().getDocID(idoc)) << " ";
    }


    std::cout << "\n\nindextester.cpp : timing simple index query retrieval..." << std::flush;

    POSIXTimer timer;
    double t, dt;

    timer.start();
    for( unsigned int i = 0 ; i < nquery_repeats ; ++i ) queryprocessormanager_simple.computeCommonSet();
    timer.stop();

    std::cout << "done.\n\n";

    t = timer.getElapsedTimeInSeconds();
    dt = t / nquery_repeats;
    std::cout << " took " << t << " s for " << nquery_repeats << " queries\n";
    std::cout << " took " << dt << " s per query\n\n";

    std::cout << "indextester.cpp : timing simple simd bitmap index query retrieval..." << std::flush;

    timer.reset();

    timer.start();
    indexsimple.testCompressed();
    timer.stop();

    std::cout << "done.\n\n";

    t = timer.getElapsedTimeInSeconds();
    dt = t;
    std::cout << " took " << t << " s for " << 1 << " queries\n";

    std::cout << "indextester.cpp : compressedset timing simplequery retrieval..." << std::flush;

    timer.reset();


    timer.start();
    indexsimple.testWavelet(ndocuments_common);
    timer.stop();

    std::cout << "done.\n\n";

    t = timer.getElapsedTimeInSeconds();
    dt = t;
    std::cout << " took " << t << " s for " << 1 << " queries\n";

    std::cout << "indextester.cpp : wavelet-tree timing simplequery retrieval..." << std::flush;

    timer.reset();


    timer.start();
    for( unsigned int i = 0 ; i < nquery_repeats ; ++i ) queryprocessormanager_simdbitmapsimple.computeCommonSet();
    timer.stop();

    std::cout << "done.\n\n";

    t = timer.getElapsedTimeInSeconds();
    dt = t / nquery_repeats;
    std::cout << " took " << t << " s for " << nquery_repeats << " queries\n";
    std::cout << " took " << dt << " s per query\n\n";

//  std::locale new_locale(std::locale(), new commas);
//  std::cout.imbue(new_locale);
    std::cout << "indextester.cpp : these results are for an AND query on two tokens with inverted list lengths " << ndocuments[0] << " and " << ndocuments[1] << ", with a common set of " << ndocuments_common << " documents, and total number of documents = " << ndocuments_total << ".\n\n";

}

BOOST_AUTO_TEST_SUITE_END()

