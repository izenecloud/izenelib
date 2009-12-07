#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <util/ProcMemInfo.h>
#include <am/trie/alphabet.hpp>
#include <am/trie/alphabet_en.hpp>

#include <am/mt_trie/mt_trie.hpp>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::util;

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

int main()
{
    std::cout << std::endl << "Performance Test for HDBTrie" << std::endl;

    displayMemInfo();

    clock_t start, finish;
    boost::posix_time::ptime start_real = boost::posix_time::microsec_clock::local_time();

    {

        MtTrie<wiselib::UString> mtTrie("mt", 16);
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
			wiselib::UString term(std::string(buffer, buffersize) );
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
		std::cout << "Input term" << std::endl;
		std::cin >> term;
/**
		wiselib::UString uterm("",wiselib::UString::UTF_8);
		for(size_t i=0; i<term.size(); i++) {
			uterm += term[i];
		}
*/
		wiselib::UString uterm(term,wiselib::UString::UTF_8);
		for(size_t i=0; i<uterm.size(); i++) {
			std::cout <<(short) ((char*) uterm.c_str())[i] << ",";
		}
		std::cout << std::endl;
		std::vector<wiselib::UString> terms;
		mtTrie.findRegExp(uterm, terms);
		for(size_t i =0; i<terms.size(); i++ )
			std::cout << terms[i] << std::endl;
	}

	mtTrie.close();
	return 0;
    }
}

