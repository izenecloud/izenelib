/**
* @file       BoostTestThreadSafety.h
* @author     Jun
* @version    SF1 v5.0
* @brief as Boost.Test is not thread safe, define thread safe versions of macro's.
*
*/

#ifndef BOOST_TEST_THREAD_SAFETY_H
#define BOOST_TEST_THREAD_SAFETY_H

#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>

namespace izenelib
{
    namespace test
    {
        extern boost::mutex boost_test_lock_;
    }
}

template<typename S, typename T>
inline void boostCheckEqualTS(const S& a, const T& b, const char* expA, const char* expB)
{
    if(static_cast<T>(a) != b)
    {
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_);
        BOOST_ERROR("check " << expA << " == " << expB << " failed"
                    << " [" << a << " != " << b << "]");
    }
}

template<typename S, typename T>
inline void boostCheckNETS(const S& a, const T& b, const char* expA, const char* expB)
{
    if(static_cast<T>(a) == b)
    {
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_);
        BOOST_ERROR("check " << expA << " != " << expB << " failed"
                    << " [" << a << " == " << b << "]");
    }
}

template<typename S, typename T>
inline void boostRequireEqualTS(const S& a, const T& b, const char* expA, const char* expB)
{
    if(static_cast<T>(a) != b)
    {
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_);
        BOOST_FAIL("check " << expA << " == " << expB << " failed"
                    << " [" << a << " != " << b << "]");
    }
}

template<typename S, typename T>
inline void boostRequireNETS(const S& a, const T& b, const char* expA, const char* expB)
{
    if(static_cast<T>(a) == b)
    {
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_);
        BOOST_FAIL("check " << expA << " != " << expB << " failed"
                    << " [" << a << " == " << b << "]");
    }
}

#define BOOST_CHECK_EQUAL_TS(a,b) \
do { \
    boostCheckEqualTS((a), (b), #a, #b); \
} while(false)

#define BOOST_CHECK_NE_TS(a,b) \
do { \
    boostCheckNETS((a), (b), #a, #b); \
} while(false)

#define BOOST_CHECK_TS(exp) \
do { \
    if(! (exp)) \
    { \
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_); \
        BOOST_ERROR("check " << #exp << " failed"); \
    } \
} while(false)

#define BOOST_REQUIRE_EQUAL_TS(a,b) \
do { \
    boostRequireEqualTS((a), (b), #a, #b); \
} while(false)

#define BOOST_REQUIRE_NE_TS(a,b) \
do { \
    boostRequireNETS((a), (b), #a, #b); \
} while(false)

#define BOOST_REQUIRE_TS(exp) \
do { \
    if(! (exp)) \
    { \
        boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_); \
        BOOST_FAIL("check " << #exp << " failed"); \
    } \
} while(false)

#define BOOST_TEST_MESSAGE_TS(msg) \
do { \
    boost::mutex::scoped_lock lock(izenelib::test::boost_test_lock_); \
    BOOST_TEST_MESSAGE(msg); \
} while(false)

#endif
