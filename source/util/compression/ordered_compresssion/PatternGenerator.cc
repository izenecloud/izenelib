#include "PatternGenerator.h"
#include <algorithm>

using namespace std;

namespace izenelib {
namespace util{
namespace compression {

PatternGenerator::PatternGenerator(const string& fileName, bool NOFILE) {

	FILE *fp;
	fp = fopen(fileName.c_str(), "r");

	if ( !fp) {
		throw CompressorException("PatternGenerator:file not exits\n");
	}

	while ( !feof(fp) ) {
		char ch;
		ch = fgetc(fp);
		if( feof(fp) )break;
		if(!NOFILE || ch != '\n')
			charFrequency_[ch]++;
	}

}

}}}
