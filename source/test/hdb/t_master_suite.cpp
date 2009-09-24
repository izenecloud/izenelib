/**
 * @file t_master_suite.cpp
 * @brief The master test suite.
 * @details
 *   This file defines the BOOST_TEST_DYN_LINK and BOOST_TEST_MAIN, which should be defined only once for one master test suite.
 *   It can also have test suites and test cases of the main process.
 * @author MyungHyun (Kent)
 * @date 2008-07-09
 */

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

