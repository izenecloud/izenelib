/*
 * =====================================================================================
 *
 *       Filename:  t_line_reader.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2013年05月22日 10时00分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */

#include "am/util/line_reader.h"

#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <sys/time.h>

#include <string>
#include <time.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include<stdio.h>

using namespace izenelib::am::util;
using namespace std;

BOOST_AUTO_TEST_SUITE(line_reader_test)

BOOST_AUTO_TEST_CASE(line_reader_test)
{
    const uint32_t SCALE = 100;
    {
        ofstream of("tmp.line_reader");
        for ( uint32_t i=0; i<SCALE; ++i)
            of << i << std::endl;
    }

    LineReader lr("tmp.line_reader");
    char * line = NULL;
    int32_t tt = 0;

    while((line = lr.line(line))!=NULL)
    {
        //std::cout<<line<<"=="<<atoi(line)<<std::endl;
        BOOST_CHECK(tt == atoi(line));
        tt++;
    }
    BOOST_CHECK(SCALE == (uint32_t)tt);
}

BOOST_AUTO_TEST_SUITE_END()
