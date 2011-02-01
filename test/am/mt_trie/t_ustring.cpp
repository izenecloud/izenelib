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

BOOST_AUTO_TEST_SUITE( mt_trie_t_ustring )

BOOST_AUTO_TEST_CASE(regular)
{

    clock_t start, finish;
    boost::posix_time::ptime start_real = boost::posix_time::microsec_clock::local_time();

    {

        MtTrie<izenelib::util::UString> mtTrie("mtustring", 64);
	mtTrie.open();
        start = clock();
	std::ifstream input;
	input.open("input-ustring", std::ifstream::binary|std::ifstream::in);
	int buffersize;
	char buffer[256];
	while(!input.eof()) {
		input.read((char*)&buffersize, sizeof(int));
		if(buffersize > 256) {
			input.seekg(buffersize, std::ifstream::cur);
		} else {
			input.read(buffer, buffersize);
			izenelib::util::UString term(std::string(buffer, buffersize) );
			mtTrie.insert(term);
		}
	}
	mtTrie.executeTask(2);
        finish = clock();
        boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - start_real;
        printf( "\nIt takes %f seconds (CPU time) and %f seconds (real time)\n",
            (double)(finish - start) / CLOCKS_PER_SEC, (double) (td.total_microseconds())/1000000);
	mtTrie.flush();
	{
	std::ifstream input;
	input.open("input-ustring", std::ifstream::binary|std::ifstream::in);
	int buffersize;
	char buffer[256];
	std::vector<izenelib::util::UString> results;
	while(!input.eof()) {
		input.read((char*)&buffersize, sizeof(int));
		if(buffersize > 256) {
			input.seekg(buffersize, std::ifstream::cur);
		} else {
			input.read(buffer, buffersize);
			izenelib::util::UString term(std::string(buffer, buffersize) );
			for(size_t i=0; i<term.length(); i++) {
				izenelib::util::UString wildcard = term;
				wildcard[i] = '?';
				results.clear();
				mtTrie.findRegExp(wildcard, results);
				BOOST_CHECK(results.size()>0);
			}
			for(size_t i=1; i<term.length(); i++) {
				izenelib::util::UString wildcard = term;
				wildcard[i] = '*';
				results.clear();
				mtTrie.findRegExp(wildcard, results);
				BOOST_CHECK(results.size()>0);
			}
		}
	}

	}
	mtTrie.close();
    }
}

BOOST_AUTO_TEST_SUITE_END()
