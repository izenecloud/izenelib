#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <util/ProcMemInfo.h>
#include <am/trie/alphabet.hpp>
#include <am/trie/alphabet_en.hpp>

#include <am/mt_trie/mt_trie.hpp>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::util;

BOOST_AUTO_TEST_SUITE( mt_trie_suite )

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

BOOST_AUTO_TEST_CASE(HDBTrie_am)
{
    std::cout << std::endl << "Performance Test for HDBTrie" << std::endl;

    displayMemInfo();

    clock_t start, finish;
    boost::posix_time::ptime start_real = boost::posix_time::microsec_clock::local_time();

    {

        MtTrie<std::string> mtTrie("mt", 16);
	mtTrie.open();
        start = clock();

	std::ifstream input;
	input.open("input", std::ifstream::binary|std::ifstream::in);
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


	mtTrie.executeTask(2);
        finish = clock();
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start_real;
        printf( "\nIt takes %f seconds (CPU time) and %f seconds (real time)\n",
            (double)(finish - start) / CLOCKS_PER_SEC, (double) (td.total_microseconds())/1000000);
	mtTrie.flush();
	while(true) {
		std::string term;
		std::vector<std::string> terms;
		std::cout << "Input term" << std::endl;
		std::cin >> term;
		mtTrie.findRegExp(term, terms);
		for(size_t i =0; i<terms.size(); i++ )
			std::cout << terms[i] << std::endl;
	}

	mtTrie.close();
    }
}

BOOST_AUTO_TEST_SUITE_END()
