#include <boost/test/unit_test.hpp>

#include <util/url_util.h>


using namespace izenelib::util;


BOOST_AUTO_TEST_SUITE(t_url_util)

BOOST_AUTO_TEST_CASE(normal_test)
{
    std::string url = "http://product.m18.com/p-1112529.htm";
    std::string base_site;
    BOOST_CHECK(izenelib::util::UrlUtil::GetBaseSite(url, base_site));
    BOOST_CHECK(base_site=="m18.com");
    url = "http://product.m18.com.cn/p-1112529.htm/asdsd";
    BOOST_CHECK(izenelib::util::UrlUtil::GetBaseSite(url, base_site));
    BOOST_CHECK(base_site=="m18.com.cn");
}


BOOST_AUTO_TEST_SUITE_END()
