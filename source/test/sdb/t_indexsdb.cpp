#include <sdb/IndexSDB.h>
#include <sdb/TrieIndexSDB.h>

#include <boost/test/unit_test.hpp>

using namespace izenelib::sdb;


BOOST_AUTO_TEST_SUITE( bptree_suite )
BOOST_AUTO_TEST_CASE(bptree_open_close_test)
{
	IndexSDB<string, string> isdb("i1.dat");
	isdb.initialize();	
}


BOOST_AUTO_TEST_CASE(bptree_test)
{
	
}

BOOST_AUTO_TEST_SUITE_END()

