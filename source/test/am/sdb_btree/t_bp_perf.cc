#include <am/am_test/am_test.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_btree/sdb_bptree.h>
#include <am/sdb_hash/sdb_hash.h>

#include <boost/test/unit_test.hpp>

using namespace izenelib::am;
using namespace izenelib::am_test;

static int num = 1000000;
static bool rnd = 1;
static int loop = 1;
static bool trace = 0;

BOOST_AUTO_TEST_SUITE( bptree_perf_suite )

BOOST_AUTO_TEST_CASE(bptree_test) {
	num = 10000000;	
	
	/*{
			cout<<"\n\nsdb_btree<int, string >"<<endl;
			typedef sdb_btree<int, string> AM_TYPE;
			AmTest<int, string, AM_TYPE, true> am;
			am.setTrace(trace);
			am.setRandom(rnd);
			am.setNum(num);
			run_am(am);
	}*/

	
	{
		cout<<"\n\nsdb_bptree<int, string >"<<endl;
		typedef sdb_btree<int, string> AM_TYPE;
		AmTest<int, string, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}


	
	/*{
		cout<<"\n\nsdb_bptree<string, int >"<<endl;
		typedef sdb_bptree<string, int> AM_TYPE;
		AmTest<string, int, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}*/

}

#if 0
BOOST_AUTO_TEST_CASE(bptree_perf_test)
{
	{
		cout<<"\n\nsdb_bptree<int, string >"<<endl;
		typedef sdb_bptree<int, string> AM_TYPE;
		AmTest<int, string, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n\nsdb_btree<int, string >"<<endl;
		typedef sdb_btree<int, string> AM_TYPE;
		AmTest<int, string, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n\nsdb_hash<int, string >"<<endl;
		typedef sdb_hash<int, string> AM_TYPE;
		AmTest<int, string, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n\nsdb_bptree<string, int >"<<endl;
		typedef sdb_bptree<string, int> AM_TYPE;
		AmTest<string, int, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n\nsdb_btree<string, int >"<<endl;
		typedef sdb_btree<string, int> AM_TYPE;
		AmTest<string, int, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n\nsdb_hash<string, int>"<<endl;
		typedef sdb_hash<string, int> AM_TYPE;
		AmTest<string, int, AM_TYPE, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

}
#endif

BOOST_AUTO_TEST_SUITE_END()
