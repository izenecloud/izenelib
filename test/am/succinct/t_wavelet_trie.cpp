#include <boost/test/unit_test.hpp>
#include <am/succinct/wavelet_trie/wavelet_trie.hpp>
#include <util/ustring/UString.h>



BOOST_AUTO_TEST_SUITE (t_wavelet_trie) // name of the test suite is t_wavelet_trie

//typedef std::string string_type;
typedef izenelib::util::UString string_type;

string_type itos(size_t i, size_t N)
{
    string_type str;

    size_t p = i;
    size_t j = 1;
    while (N / j > 9) j *= 10;
    while (j)
    {
        str += static_cast<izenelib::util::UCS2Char>(p / j + 48);
//		str.assign(tmp);
        p = p % j;
        j = j / 10;
    }
    while (static_cast<izenelib::util::UCS2Char>('0') == str[0] && str.length() > 1) str.erase(0,1);
    return str;
}




//string_type ch = "零一二三四五六七八九";

string_type create(size_t S, size_t L)
{
    string_type str;
    for (size_t i = 0; i < L; ++i)
    {
        str += static_cast<izenelib::util::UCS2Char> (rand() % S +1);
//		str += (x % S % 255 +1);
    }
    return str;
}

BOOST_AUTO_TEST_CASE( wavelet_trie_1 )
{
//    clock_t time0, time1, time2, time3, time4, time5, time6;
    std::vector<std::string> B;
    std::vector<string_type> A;
    B.push_back("a一三");
    B.push_back("一a二");
    B.push_back("一a二三");
    B.push_back("a一二");
    B.push_back("一a三");
    B.push_back("a一四");
    A.resize(B.size());
    for (size_t i = 0; i < B.size(); ++i)
        for (size_t j = 0; j < B[i].length(); ++j)
            A[i]+=B[i][j];
//for(size_t i = 0 ; i < A[1].length() ; ++ i) std::cout<<static_cast<int>(A[1][i])<<' '<<static_cast<int>(B[1][i])<<"\n";
    izenelib::am::succinct::wavelet_trie::wavelet_trie<string_type> T;

    T.build(A);

    for(size_t i = 0; i < A.size(); ++i)
    {
        BOOST_CHECK_EQUAL(A[i],T.access(i));
    }
    /*
        size_t N = 100;
        size_t M = N / 10;

        size_t L = 10;
    	size_t S = 10000;
        std::cout<<"N="<<N<<'\n';
    string_type str = A[1];
    std::vector<size_t> C;
    for(size_t i = 0; i <= str.length(); ++i)C.push_back(str[i]);
        for(size_t i = 0; i < N; ++i) {
    		string_type str = create(S, L);
    //std::cout<<str<<"!\n";
    //        string_type str = rand(S, L, i);
            A.push_back(str);
        }

        time0 = clock();
        izenelib::am::succinct::wavelet_trie::wavelet_trie<string_type> T;

        T.build(A);

        time1 = clock();
        std::cout<<"Build cost "<<(double)(time1 - time0) / CLOCKS_PER_SEC<<"seconds.\n";

        for(size_t i = 0; i < M; ++i) {
    //		string_type str = itos(i, N);
            T.access(i);
    //		BOOST_CHECK_EQUAL(str,T.access(i));
        }

        time2 = clock();
        std::cout<<M<<" access cost "<<(double)(time2 - time1) / CLOCKS_PER_SEC<<"seconds.\n";

        for(size_t i = 0; i < M; ++i) {
    //    		string_type str = itos(i, N);
    //        string_type str = rand(S, L, i);
            T.rank(C, i + 1);
    //		BOOST_CHECK_EQUAL(1, T.rank(C, i + 1));
        }

        time3 = clock();
        std::cout<<M<<" rank cost "<<(double)(time3 - time2) / CLOCKS_PER_SEC<<"seconds.\n";

        for(size_t i = 0; i < M; ++i) {
    //    		string_type str = itos(i, N);
    //        string_type str = rand(S, L, i);
            T.select(C, 1);
    //		BOOST_CHECK_EQUAL(i, T.select(C, 1));
        }

        time4 = clock();
        std::cout<<M<<" select cost "<<(double)(time4 - time3) / CLOCKS_PER_SEC<<"seconds.\n";

        for(size_t i = 0; i < M; ++i) {
    //    		string_type str = itos(i, N);
    //        string_type str = rand(S, L, i);
            T.rank_prefix(C, i);
    //	BOOST_CHECK_EQUAL(1, T.rank_prefix(C, i + 1));
        }

        time5 = clock();
        std::cout<<M<<" rank_prefix cost "<<(double)(time5 - time4) / CLOCKS_PER_SEC<<"seconds.\n";

        for(size_t i = 0; i < M; ++i) {
    //    		string_type str = itos(i, N);
    //        string_type str = rand(S, L, i);
            T.select_prefix(C, 1);
    //	BOOST_CHECK_EQUAL(i, T.select_prefix(C, 1));
        }

        time6 = clock();
        std::cout<<M<<" select_prefix cost "<<(double)(time6 - time5) / CLOCKS_PER_SEC<<"seconds.\n";
    */
}

BOOST_AUTO_TEST_SUITE_END( )
