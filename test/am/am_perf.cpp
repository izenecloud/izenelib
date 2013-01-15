#include <am/am_test/am_types.h>
#include <am/am_test/am_test.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/shared_ptr.hpp>

using namespace izenelib::am;
using namespace izenelib::am_test;

typedef boost::mpl::list<
AmTest<int, string, LinearHashTable<int, string> >, 
//AmTest<int, string, dynamic_perfect_hash<int, string> >, 
AmTest<int, string, ext_hash_map<int, string> >, 
AmTest<int, string, rde_hash<int, string> >, 
AmTest<int, string, stx_btree<int, string> > 

//AmTest<int, string, tc_hash<int, string>, true >,  
//AmTest<int, string, tc_btree<int, string>, true >,
> int_string_am_types;


typedef boost::mpl::list<
AmTest<string, int, LinearHashTable<string, int> >, 
//AmTest<string, int, dynamic_perfect_hash<string, int> >, 
AmTest<string, int, ext_hash_map<string, int> >, 
AmTest<string, int, rde_hash<string, int> >, 
AmTest<string, int, stx_btree<string, int> > 

//AmTest<string, int, tc_hash<string, int>, true >,  
//AmTest<string, int, tc_btree<string, int>, true >,
> string_int_am_types;



BOOST_AUTO_TEST_SUITE(IntString_test)



BOOST_AUTO_TEST_CASE_TEMPLATE(run_am_int_string_rnd, T, int_string_am_types)
{
	{
      T am;
      am.setRandom(false);
      run_crud(am);  
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(run_am_int_string, T, int_string_am_types)
{
	{
      T am;
      am.setRandom(true);
      run_crud(am);  
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(run_am_string_int_rnd, T, string_int_am_types)
{
	{
      T am;
      am.setRandom(false);
      run_crud(am);  
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(run_am_string_int, T, string_int_am_types)
{
	{
      T am;
      am.setRandom(true);
      run_crud(am);  
	}
}





BOOST_AUTO_TEST_SUITE_END()
