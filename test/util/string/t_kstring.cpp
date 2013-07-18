/*
 * =====================================================================================
 *
 *       Filename:  t_kstring.cpp
 *
 *    Description:  Test cases of KString.
 *
 *        Version:  1.0
 *        Created:  2013年05月20日 17时43分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */

#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <sys/time.h>

#include <string>
#include <time.h>
#include <math.h>
#include <vector>
#include <ostream>
#include <iostream>

#include "util/string/kstring.hpp"
using namespace izenelib;
using namespace izenelib::util;

static const uint32_t SCALE = 1000;
BOOST_AUTO_TEST_SUITE(kstring_test)

BOOST_AUTO_TEST_CASE(empty_construct_copy_test)
{
    std::vector<KString> v;
    for ( uint32_t i=0; i<SCALE; ++i)
    {
        KString s;
        v.push_back(s);
    }
    for ( uint32_t i=0; i<SCALE; ++i)
    {
        BOOST_CHECK(v[i].length() == 0);
    }
}

BOOST_AUTO_TEST_CASE(copy_test)
{
    std::vector<KString> v;
    for ( uint32_t i=0; i<SCALE; ++i)
    {
        KString s = KString::value_of(i);
        KString ss = s;
        v.push_back(s);
    }
    for ( uint32_t i=0; i<SCALE; ++i)
    {
        KString s = KString::value_of(i);
        BOOST_CHECK(v[i] == s);
        v[i] = KString::value_of(i+1);
    }
    for ( uint32_t i=0; i<SCALE; ++i)
    {
        BOOST_CHECK(v[i] == KString::value_of(i+1));
    }
}

BOOST_AUTO_TEST_CASE(concat_test)
{
    KString kss;
    {
        KString ks;
        for ( uint32_t i = 0; i<SCALE; ++i)
            ks += 'A' + i%26;
        kss = ks;
    }

    //std::cout<<kss<<std::endl;
    for ( uint32_t i = 0; i<SCALE; ++i)
    {
        //std::cout<<(char)kss[i]<<":"<<(char)('A' + i%26)<<std::endl;
        BOOST_CHECK(kss[i] == ('A' + i%26));
    }

    KString ksss = kss;
    for ( uint32_t i = 0; i<SCALE; ++i)
        ksss[i] = 'a' + i%26;

    for ( uint32_t i = 0; i<SCALE; ++i)
        BOOST_CHECK(kss[i] == ('A' + i%26));

    for ( uint32_t i = 0; i<SCALE; ++i)
        BOOST_CHECK(ksss[i] == ('a' + i%26));

    kss += ksss;
    //std::cout<<kss<<std::endl;
    for ( uint32_t i = 0; i<SCALE; ++i)
        BOOST_CHECK(kss[i] == ('A' + i%26));
    for ( uint32_t i = SCALE; i<2*SCALE; ++i)
        BOOST_CHECK(kss[i] == ('a' + (i-SCALE)%26));
}

BOOST_AUTO_TEST_CASE(feature_test)
{
    std::string str = "abcccdddsssdvdrertrer";
    KString ks(str);
    BOOST_CHECK(ks.end_with("rertrer"));
    BOOST_CHECK(ks.end_with("eertrer") == false);
    BOOST_CHECK(strcmp(ks.get_bytes("utf8").c_str(), str.c_str()) == 0);
    BOOST_CHECK(ks.index_of('d') == 5);
    BOOST_CHECK(ks.index_of('k') == (uint32_t)-1);
    BOOST_CHECK(ks.find(KString("ccdddsss")) == 3);
    BOOST_CHECK(ks.find(KString("acdddsss")) == (uint32_t)-1);

    KString kss = ks;
    kss.replace('c', 't');
    BOOST_CHECK(kss.equals("abtttdddsssdvdrertrer"));
    BOOST_CHECK(strcmp(ks.get_bytes("utf8").c_str(), str.c_str()) == 0);

    std::vector<KString> v = kss.split('d');
    //for ( uint32_t i=0; i<v.size(); ++i)
    //std::cout<<v[i]<<std::endl;
    BOOST_CHECK(v.size() == 6);
    BOOST_CHECK(v[0] == std::string("abttt"));
    BOOST_CHECK(v[1].length() == 0 && v[2].length() == 0 );
    BOOST_CHECK(v[3] == std::string("sss"));
    BOOST_CHECK(v[4] == std::string("v"));
    BOOST_CHECK(v[5] == std::string("rertrer"));

    kss.trim('d');
    BOOST_CHECK(kss.equals("abtttsssvrertrer"));
    kss.replace("ttt", "tttt");
    //std::cout<<kss<<std::endl;
    kss.replace("trer", "trerr");
    //std::cout<<kss<<std::endl;
    kss.replace("trer", "tre");
    //std::cout<<kss<<std::endl;
    BOOST_CHECK(kss.equals("abttttsssvrertrer"));

    ks = KString("aAbBcC");
    kss = ks;
    kss.to_lower_case();
    BOOST_CHECK(kss.equals("aabbcc"));
    BOOST_CHECK(ks.equals("aAbBcC"));
    ks.to_upper_case();
    BOOST_CHECK(ks.equals("AABBCC"));

    kss = KString("＋－＊／＝？");
    kss.to_dbc();
    BOOST_CHECK(kss.equals("+-*/=?"));

    kss = KString(" sdfg  sdffgfghs  sdf   ");
    kss.trim_into_1();
    BOOST_CHECK(kss.equals(" sdfg sdffgfghs sdf "));

    kss = KString("   sdfg   sdffgfghs  sdf ");
    kss.trim_into_1();
    BOOST_CHECK(kss.equals(" sdfg sdffgfghs sdf "));

    kss = KString(" sdfg ");
    kss.trim_into_1();
    BOOST_CHECK(kss.equals(" sdfg "));

    kss = KString("sdfg ss");
    kss.trim_into_1();
    BOOST_CHECK(kss.equals("sdfg ss"));

    kss = KString("8 0");
    kss.trim_into_1();
    BOOST_CHECK(kss.equals("8 0"));

    kss = KString(" sdfg ss  ");
    kss.trim_head_tail();
    BOOST_CHECK(kss.equals("sdfg ss"));

    kss = KString("  sdfgss  ");
    kss.trim_head_tail();
    BOOST_CHECK(kss.equals("sdfgss"));

    kss = KString("sdfh   ");
    kss.trim_head_tail();
    BOOST_CHECK(kss.equals("sdfh"));

    kss = KString("   ");
    kss.trim_head_tail();
    BOOST_CHECK(kss.length() == 0);

    kss = KString(" ");
    kss.trim_head_tail();
    BOOST_CHECK(kss.length() == 0);
}

BOOST_AUTO_TEST_SUITE_END()

