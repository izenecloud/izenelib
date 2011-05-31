/**
 * Needed to use boost shared library. should define before including unit_test.hpp. 
 * Only one define statement should exist in a whole test
 */
#define BOOST_TEST_DYN_LINK     

/**
 * Needed to use boost shared library. should define before including unit_test.hpp. 
 * This defines the master test suite. So, only one master test suite define statement should exist in a test. 
 * (Or one for each execution file)
 */
#define BOOST_TEST_MAIN         

/** 
 * Can define a test module in the following way. 
 * This creates a master test suite as above, but also assigns the master test suite with a name.
 * e.g. Here the master test Suite name is "boost test example test suite"
 */
#define BOOST_TEST_MODULE boost test example test suite


#include <boost/test/unit_test.hpp>

