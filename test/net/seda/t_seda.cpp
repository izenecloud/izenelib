/*
 * =====================================================================================
 *
 *       Filename:  t_seda.cpp
 *
 *    Description:  SEDA framework test.
 *
 *        Version:  1.0
 *        Created:  12/31/2012 10:13:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */
#include "net/seda/queue.hpp"
#include "net/seda/stage.hpp"
#include "util/hashFunction.h"
#include <cstdlib>
#include <ostream>
#include <boost/test/unit_test.hpp>

using namespace izenelib;
using namespace izenelib::util;
BOOST_AUTO_TEST_SUITE(seda_test)
class RandStringGen
{
    std::string _str;
    uint64_t _hash;
public:
    RandStringGen(const std::string& s="", uint64_t has=-1)
        :_str(s),_hash(has)
    {}

    bool doit(const RandStringGen& e, RandStringGen& o, std::ostringstream& log)
    {
        std::string str;
        uint32_t len = rand()%1000+1;
        for ( uint32_t i=0; i<len; ++i)
            str += 'A' + rand()%26;
        o = RandStringGen(str, HashFunction<std::string>::generateHash64(str));
        return true;
    }
    bool check()const
    {
        return HashFunction<std::string>::generateHash64(_str) == _hash;
    }
};


class CheckString
{
public :
    bool doit(const RandStringGen& e, RandStringGen& o, std::ostringstream& log)
    {
        if (!e.check())
        {
            std::cout<<"  ERRORRRR!\n";
            return false;
        }
        return true;
    }
};

BOOST_AUTO_TEST_CASE(rand_string_test)
{
    EventQueue<RandStringGen> qu;
    RandStringGen gen;
    CheckString che;

    Stage<RandStringGen, EventQueue<RandStringGen>, EventQueue<RandStringGen> > stage1(&gen, NULL, &qu, 5);
    Stage<CheckString, EventQueue<RandStringGen>, EventQueue<RandStringGen> > stage2(&che, qu.add_ref(), NULL, 6);

    stage1.start();
    stage2.start();

    sleep(100);
    stage1.stop();
    stage2.stop();
}

BOOST_AUTO_TEST_SUITE_END()

