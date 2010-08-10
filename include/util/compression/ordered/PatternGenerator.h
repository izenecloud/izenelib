#ifndef PatternGenerator_H_
#define PatternGenerator_H_

#include "TreeAlgorithm.h"

namespace izenelib {
namespace util{
namespace compression {

/**
 *     \brief the responsibility is to generate  BitPattern from char frequency info. 
 */
class PatternGenerator {
	typedef unsigned int Weight;
public:
	PatternGenerator(){}	
	//if NOFILE is TRUE, it doesn't encode '\n', defaut is false for compressing file.   
	PatternGenerator(const string& fileName, bool NOFILE=false);
	PatternGenerator(const map<char, unsigned int>& input) :
		charFrequency_(input) {
	}
	map<char, unsigned int>& getCharFrequency() {
		return charFrequency_;
	}
	Tree *makeTree(BitPattern& bp)
	{
		return ta_.makeTree(bp);
	}
	BitPattern& getBitPattern() {
		ta_.generaterPattern(charFrequency_, bitPattern_);
		return bitPattern_;
	}
private:
	TreeAlgorithm ta_; //it is easy to be extended to strategy design pattern.
	map<char, Weight> charFrequency_;
	BitPattern bitPattern_;
};

}}}

#endif /*PatternGenerator_H_*/
