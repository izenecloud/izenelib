/// @file   example_searcher.cc
/// @brief  This example shows how to search words/wildcards in a b-trie
/// @author Wei Cao
/// @date   2000-08-04
///
///
/// @details
///


#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <am/trie/b_trie.hpp>
#include <am/trie/alphabet.hpp>
#include <am/map/map.hpp>

#include <util/ustring/UString.h>

#include <util/izene_log.h>

using namespace izenelib::util;
using namespace izenelib::am;
using namespace izenelib::util;
using namespace std;

#define SIZE 27
#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

typedef string string_type;

UString init (const string& str, UString*)
{
    UString t(str.c_str(), ENCODE_TYPE);
    return t;
}

string init (const string& str, string*)
{
    return str;
}

int main(int argc, char** argv)
{
    clock_t start, finish;

    // Initialize Google's logging library.
    google::InitGoogleLogging("b_trie");

    //------------------------------------------------------------------------------
    {
        BTrie_En trie1("./test");

//    while(true)
        {
            cout << "enter query: ";
            string query;
            cin >> query;

            int count = 0;
            start = clock();
            {
                vector<item_pair<string_type> > ip;
                string_type pattern = init(query, (string_type*)0);
                trie1.findRegExp(pattern, ip);
                std::cout << ip.size() << " results" << endl;
//        for (vector<item_pair<string_type> >::iterator i = ip.begin(); i!= ip.end(); i++)
//          cout<<i->str_<<endl;
            }
            finish = clock();
            if(start == finish)
            {
                start = clock();
                for(int i=0; i<1000; i++)
                {
                    vector<item_pair<string_type> > ip;
                    string_type pattern = init(query, (string_type*)0);
                    trie1.findRegExp(pattern, ip);
                }
                finish = clock();
                cout << "search " << query << " cost " <<
                     (double)(finish-start) / (CLOCKS_PER_SEC) << " ms" << endl;
            }
            else
            {
                cout << "search " << query << " cost "
                     << (double)(finish-start) / CLOCKS_PER_SEC << " second" << endl;
            }

        }
    }


}
