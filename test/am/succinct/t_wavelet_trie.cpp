#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include "wavelet_trie.hpp"
#include <glibmm.h>
#include <boost/date_time/posix_time/posix_time.hpp>
/*
ustring is 50% slower than string in access/rank/select/rank_prefix/select_prefix
and 70% in build.

*/


BOOST_AUTO_TEST_SUITE (stringtest) // name of the test suite is stringtest

//typedef std::string string_type;
typedef Glib::ustring string_type;

string_type itos(size_t i, size_t N) {
    string_type ustr = "";
    std::string str = "";
    size_t p = i;
    size_t j = 1;
    while (N / j > 9) j *= 10;
    while (j) {
        str += (p / j + 48);
        p = p % j;
        j = j / 10;
    }
    while ('0' == str[0] && str.length() > 1) str.erase(0,1);
    ustr = str;
    return ustr;
}

string_type ch = "零一二三四五六七八九";
string_type itoc(size_t i, size_t N) {

    string_type str = "";
    size_t p = i;
    size_t j = 1;
    while (N / j > 9) j *= 10;
    while (j) {
        str += ch[p / j];
        p = p % j;
        j = j / 10;
    }
    return str;
}

string_type rand(size_t S, size_t L, size_t x) {
    string_type str = "";
    for (size_t i = 0; i < L; ++i) {
        str += static_cast<gunichar> (x % S +1);
//		str += (x % S % 255 +1);
    }
    return str;
}

BOOST_AUTO_TEST_CASE( my_test ) {
    clock_t time0, time1, time2, time3, time4, time5, time6;


    std::vector<string_type> A;
    /*
        A.push_back("a一三");
        A.push_back("一a二");
        A.push_back("一a二三");
        A.push_back("a一二");
        A.push_back("一a三");
        A.push_back("a一四");
    */
    size_t N = 100000;
    size_t M = N / 10;
    size_t L = 10;
    size_t S = 10000;
    std::cout<<"N="<<N<<'\n';
    for(size_t i = 0; i < N; ++i) {
//	string_type str = itos(i, N);
        string_type str = rand(S, L, i);
        A.push_back(str);
    }

    time0 = clock();
    wavelet_trie::wavelet_trie<string_type> T;

    T.build(A);

//	boost::posix_time::ptime start_real = boost::posix_time::microsec_clock::local_time();
    time1 = clock();
    std::cout<<"Build cost "<<(double)(time1 - time0) / CLOCKS_PER_SEC<<"seconds.\n";

    for(size_t i = 0; i < M; ++i) {
//	string_type str = itos(i, N);

        T.access(i);
//	BOOST_CHECK_EQUAL(str, T.access(i));
    }

    time2 = clock();
    std::cout<<M<<" access cost "<<(double)(time2 - time1) / CLOCKS_PER_SEC<<"seconds.\n";

    for(size_t i = 0; i < M; ++i) {
//	string_type str = itos(i, N);
        string_type str = rand(S, L, i);
        T.rank(str, i + 1);
//	BOOST_CHECK_EQUAL(1, T.rank(str, i + 1));
    }

    time3 = clock();
    std::cout<<M<<" rank cost "<<(double)(time3 - time2) / CLOCKS_PER_SEC<<"seconds.\n";

    for(size_t i = 0; i < M; ++i) {
//	string_type str = itos(i, N);
        string_type str = rand(S, L, i);
        T.select(str, 1);
//	BOOST_CHECK_EQUAL(i, T.select(str, 1));
    }

    time4 = clock();
    std::cout<<M<<" select cost "<<(double)(time4 - time3) / CLOCKS_PER_SEC<<"seconds.\n";

    for(size_t i = 0; i < M; ++i) {
//	string_type str = itos(i, N);
        string_type str = rand(S, L, i);
        T.rank_prefix(str, i);
//	BOOST_CHECK_EQUAL(1, T.rank_prefix(str, i + 1));
    }

    time5 = clock();
    std::cout<<M<<" rank_prefix cost "<<(double)(time5 - time4) / CLOCKS_PER_SEC<<"seconds.\n";

    for(size_t i = 0; i < M; ++i) {
//	string_type str = itos(i, N);
        string_type str = rand(S, L, i);
        T.select_prefix(str, 1);
//	BOOST_CHECK_EQUAL(i, T.select_prefix(str, 1));
    }

    time6 = clock();
    std::cout<<M<<" select_prefix cost "<<(double)(time6 - time5) / CLOCKS_PER_SEC<<"seconds.\n";

//		boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start_real;
//        printf( "\nIt takes %f seconds (CPU time) and %f seconds (real time)\n",
//            (double)(finish - start) / CLOCKS_PER_SEC, (double) (td.total_microseconds())/1000000);


}

BOOST_AUTO_TEST_SUITE_END( )
















