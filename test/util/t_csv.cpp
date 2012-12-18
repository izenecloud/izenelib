#include <boost/test/unit_test.hpp>

#include <util/csv.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace izenelib::util;

template <size_t A, size_t B>
vector<vector<string> > to_vect(const char *dat[A][B])
{
    vector<vector<string> > ret;
    for (size_t i=0; i<A; i++)
    {
        ret.push_back(vector<string>());
        for (size_t j=0; j<B; j++)
        {
            ret[i].push_back(dat[i][j]);
        }
    }
    return ret;
}

BOOST_AUTO_TEST_SUITE(t_csv)

BOOST_AUTO_TEST_CASE(csv_null)
{
    vector<vector<string> > dat;
    parse_csv("", dat);
    BOOST_CHECK(dat.empty());
}

BOOST_AUTO_TEST_CASE(csv_normal)
{
    vector<vector<string> > dat;
    parse_csv("abc,def,ghc\nijk,lmn,opq\n", dat);
    const char *ans[2][3]=
    {
        { "abc", "def", "ghc", },
        { "ijk", "lmn", "opq", },
    };
    BOOST_CHECK(dat==(to_vect<2,3>(ans)));
}

BOOST_AUTO_TEST_CASE(csv_end)
{
    vector<vector<string> > dat;
    parse_csv("abc,def,ghc\nijk,lmn,opq", dat);
    const char *ans[2][3]=
    {
        { "abc", "def", "ghc", },
        { "ijk", "lmn", "opq", },
    };
    BOOST_CHECK(dat==(to_vect<2,3>(ans)));
}

BOOST_AUTO_TEST_CASE(csv_crlf)
{
    vector<vector<string> > dat;
    parse_csv("abc,def,ghc\r\nijk,lmn,opq\r\n", dat);
    const char *ans[2][3]=
    {
        { "abc", "def", "ghc", },
        { "ijk", "lmn", "opq", },
    };
    BOOST_CHECK(dat==(to_vect<2,3>(ans)));
}

BOOST_AUTO_TEST_CASE(csv_dquote)
{
    vector<vector<string> > dat;
    parse_csv("\"ab\"\"c\",\"def\"\"\",\"ghc\"\n\"\"\"ijk\",\"lmn\",\"opq\"", dat);
    const char *ans[2][3]=
    {
        { "ab\"c", "def\"", "ghc", },
        { "\"ijk", "lmn", "opq", },
    };
    BOOST_CHECK(dat==(to_vect<2,3>(ans)));
}

BOOST_AUTO_TEST_CASE(csv_big)
{
    std::string fn("./test.csv");
    std::ifstream ifs(fn.c_str());
    if (!ifs)
    {
        std::cerr << "cannot open " << fn << std::endl;
      }

    csv_parser c(ifs);
    for (csv_iterator p(c), q; p!=q; ++p)
    {
        std::vector<std::string> w(p->begin(), p->end());
        std::cout<<"start: ";
        for(std::vector<std::string>::iterator wit = w.begin(); wit != w.end(); ++wit)
        {
        std::cout<<*wit<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"finish"<<std::endl;	
}


BOOST_AUTO_TEST_SUITE_END()
