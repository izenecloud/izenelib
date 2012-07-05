/// @file   example_builder.cc
///
/// @brief  A test unit for checking b_trie's funcationality and memory usage.
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
#include <signal.h>

#include <am/trie/b_trie.hpp>
#include <am/trie/alphabet.hpp>
#include <am/trie/alphabet_en.hpp>
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

void checkInput(const string_type& u)
{
    if (u.length() == 0)
        cout<<"Empty string!\n";

    for (size_t i=0; i<u.length(); i++)
    {
        if (AlphabetNode<string_type::value_type,en,en_size>::getIndexOf(u[i])>=en_size)
        {
            cout<<"Not exist in alphabet: "<<u[i]<<"   ";
            //u.displayStringValue(ENCODE_TYPE, cout);
            cout<<endl;
            break;
        }

    }
}

UString init (const string& str, UString*)
{
    UString t(str.c_str(), ENCODE_TYPE);
    return t;
}

string init (const string& str, string*)
{
    return str;
}

void readDict(const string& dict, vector<string_type>& v)
{
    ifstream f;

    f.open (dict.c_str(), std::ifstream::in);
    if (f.fail())
    {
        cout<<"Can't open the dictinary file! Please check the file name: "<<dict<<endl;
        return;
    }

    // get length of file:
    f.seekg (0, std::ios::end);
    std::size_t length = f.tellg();
    f.seekg (0, std::ios::beg);

    // allocate memory:
    char* buffer = new char [length];

    // read data as a block:
    f.read (buffer,length);
    f.close();

    string bf(buffer, length);
    //UString bf(tmp, ENCODE_TYPE);
    size_t i=0;
    size_t start = 0;

    for (; i<bf.length(); i++)
    {
        if (bf[i]==' '|| bf[i]=='\n')
        {

            if(start+1 == i && (bf[start]==' '||bf[start]=='\n'))
            {
                start = i+1;
                continue;
            }

            string_type str  = init(bf.substr(start, i-start), (string_type*)0);
            //UString tp;
            //bf.subString(tp ,start, i-start);
            checkInput(str);
            v.push_back(str);
            // str.displayStringValue(ENCODE_TYPE, cout);

            start = i+1;
            i++;

        }
    }

    delete[] buffer;

}


int main(int argc, char** argv)
{
    clock_t start, finish;

    // Initialize Google's logging library.
    google::InitGoogleLogging("b_trie");

    remove("./test.buk");
    remove("./test.nod");
    remove("./test.has");


    vector<string_type*> vp;

    {
        vector<string_type> vstr;
        readDict("./input", vstr);
        for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end(); i++)
        {
            string_type* str = new string_type (*i);
            vp.push_back(str);
        }
        cout<<"\nData is ready!\n";
    }

    //------------------------------------------------------------------------------

    {
        BTrie_En trie("./test");

        start = clock();
        int id = 1;
        for (vector<string_type*>::iterator i=vp.begin(); i!=vp.end(); i++,id++)
        {
            //    (*i).displayStringValue(ENCODE_TYPE);
            //    cout<<*(*i)<<endl;
            trie.insert(*(*i),id);

            //    if (t!=trie.find(*(*i)))
            //    {
            //      (*i)->displayStringValue(ENCODE_TYPE);
            //      cout<<endl<<debug_count<<endl<<trie.find(*(*i))<<endl;
            //      break;
            //    }
        }
        trie.flush();
        finish = clock();
        printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vp.size());
        cout<<"Node amount: "<<trie.getNodeAmount()<<endl;

    }

    for (vector<string_type*>::iterator i=vp.begin(); i!=vp.end(); i++)
    {
        delete *i;
    }
    vp.clear();

}
