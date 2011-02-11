#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>

#include <util/ProcMemInfo.h>
#include <am/trie/alphabet.hpp>
#include <am/trie/alphabet_en.hpp>

#include <am/mt_trie/mt_trie.hpp>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::util;


BOOST_AUTO_TEST_SUITE( mt_trie_t_string )

BOOST_AUTO_TEST_CASE(t_regular)
{

    clock_t start, finish;
    boost::posix_time::ptime start_real = boost::posix_time::microsec_clock::local_time();

    {

        MtTrie<std::string> mtTrie("mtl1trie", 64);
	mtTrie.open();
        start = clock();

	{
	std::ifstream input;
	input.open("input-string", std::ifstream::binary|std::ifstream::in);
	int buffersize;
	char buffer[256];
	while(!input.eof()) {
		input.read((char*)&buffersize, sizeof(int));
		if(buffersize > 256) {
			input.seekg(buffersize, std::ifstream::cur);
		} else {
			input.read(buffer, buffersize);
			std::string term(buffer, buffersize);
			mtTrie.insert(term);
		}
	}
	for(int i=0; i<100; i++) {
		mtTrie.insert("hello");
		mtTrie.insert("case");
		mtTrie.insert("house");
		mtTrie.insert("hi");
	}
	}


	mtTrie.executeTask(2);
        finish = clock();
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start_real;
        printf( "\nIt takes %f seconds (CPU time) and %f seconds (real time)\n",
            (double)(finish - start) / CLOCKS_PER_SEC, (double) (td.total_microseconds())/1000000);
	mtTrie.flush();

	std::cout << "h*" << std::endl;

	std::vector<std::string> results;
	mtTrie.findRegExp("h*", results, 2);
	std::sort(results.begin(), results.end());
	BOOST_CHECK_EQUAL(results[0], "hello");
	BOOST_CHECK_EQUAL(results[1], "hi");

	std::cout << "*se" << std::endl;
	results.clear();

	mtTrie.findRegExp("*se", results, 2);
	std::sort(results.begin(), results.end());
	BOOST_CHECK_EQUAL(results[0], "case");
	BOOST_CHECK_EQUAL(results[1], "house");



	std::cout << "*" << std::endl;
	results.clear();
	mtTrie.findRegExp("*", results, 100);


	mtTrie.close();
    }
}

BOOST_AUTO_TEST_SUITE_END()
