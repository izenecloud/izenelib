#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <util/ProcMemInfo.h>
#include <am/trie/alphabet.hpp>
#include <am/trie/alphabet_en.hpp>
#include <am/sdb_trie/sdb_trie.hpp>
#include <am/map/map.hpp>
#include <am/btree/BTreeFile.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_hash.h>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::util;

BOOST_AUTO_TEST_SUITE( sdb_trie_suite )

typedef string string_type;
#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

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
      v.push_back(str);
      // str.displayStringValue(ENCODE_TYPE, cout);

      start = i+1;
      i++;

    }
  }

  delete[] buffer;

}

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}


BOOST_AUTO_TEST_CASE(SDBTrie_am)
{
    std::cout << std::endl << "Performance Test for SDBTrie" << std::endl;

    vector<string_type> vstr;
    readDict("./input", vstr);

    displayMemInfo();

    clock_t start, finish;

    {
        SDBTrie2<std::string, uint32_t> sdbTrie("SDBTrie_am");
        sdbTrie.openForWrite();
        start = clock();
        uint32_t id = 0;
        for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end();i++, id++)
        {
            if(id%100000 == 0) {
                std::cout << "insert" << id << std::endl;
                displayMemInfo();
            }
            sdbTrie.insert(*i, id);
        }
        sdbTrie.optimize();
        finish = clock();
        printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());
    }

    {

        SDBTrie2<std::string, uint32_t> sdbTrie("SDBTrie_am");
        sdbTrie.openForRead();
        start = clock();
        uint32_t id;
        for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end();i++)
        {
            if( false == sdbTrie.get(*i, id) )
                std::cerr << "error " << *i << std::endl;
        }
        finish = clock();
        printf( "\nIt takes %f seconds to find %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());


        std::vector<uint32_t> results;

        start = clock();
        sdbTrie.findRegExp("*abcd", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d *abcd!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());
        displayMemInfo();

        start = clock();
        sdbTrie.findRegExp("a*bcd", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d a*bcd!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());

        start = clock();
        sdbTrie.findRegExp("ab*cd", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d ab*cd!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());

        start = clock();
        sdbTrie.findRegExp("abc*d", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d abc*d!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());

        start = clock();
        sdbTrie.findRegExp("abcd*", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d abc*d!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());

        start = clock();
        sdbTrie.findRegExp("gqnkhw?pa", results);
        finish = clock();
        printf( "\nIt takes %f seconds to find %d gqnkhw?pa!\n", (double)(finish - start) / CLOCKS_PER_SEC, results.size());
    }
}

//BOOST_AUTO_TEST_CASE(SdbBtree_am)
//{
//    std::cout << std::endl << "Performance Test for SdbBtree" << std::endl;
//
//    vector<string_type> vstr;
//    readDict("./input", vstr);
//
//    displayMemInfo();
//
//    clock_t start, finish;
//
//    {
//        SDBTrie2<std::string, uint32_t> sdbTrie("SdbBtree_am");
//        sdbTrie.openForWrite();
//        start = clock();
//        uint32_t id = 0;
//        for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end();i++, id++)
//        {
//            if(id%100000 == 0) {
//                std::cout << "insert" << id << std::endl;
//                displayMemInfo();
//            }
//            sdbTrie.insert(*i, id);
//        }
//        sdbTrie.optimize();
//        finish = clock();
//        printf( "\nIt takes %f seconds to insert %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());
//    }
//
//    {
//        SDBTrie2<std::string, uint32_t> sdbTrie("SdbBtree_am");
//        sdbTrie.openForRead();
//        start = clock();
//        uint32_t id  = 0;
//        for (vector<string_type>::iterator i=vstr.begin(); i!=vstr.end();i++, id++)
//        {
//            if(id%100000 == 0)
//                std::cout << "find" << id << std::endl;
//            uint32_t tmp;
//            sdbTrie.get(*i, tmp);
//            if(tmp!=id) std::cerr << "error " << *i << " should be " << id << ", but is " << tmp << std::endl;
//        }
//        finish = clock();
//        printf( "\nIt takes %f seconds to find %d random data!\n", (double)(finish - start) / CLOCKS_PER_SEC, vstr.size());
//    }
//}

BOOST_AUTO_TEST_SUITE_END()
