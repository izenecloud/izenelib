#ifndef BITPATTERN_H_
#define BITPATTERN_H_

#include <map>
#include <string>
#include <iostream>
#include <iomanip>

#include "IntTypes.h"
#include "CompressorException.h"
using namespace std;
using namespace izenelib::util::compression;

namespace izenelib {
namespace util{
namespace compression {

/**
 *    \brief binary code for a char
 */
struct Pattern
{
	ub1 nbits;  //not exceed 128
	ub4 bits;	//it is assumed a char's bit would not exceed 32.
};

/**
 *   \brief wrapper for bit Patternchar->binary code.
 */
class BitPattern{
public:
	BitPattern(){}	
	BitPattern(const map<char, Pattern>& bitPattern): bitPattern_(bitPattern){}
	void fromFile(FILE* fp);
	void toFile(FILE* fp);
	inline Pattern& getPattern(char ch){
		return bitPattern_[ch];
	}
	void display(){
		map<char, Pattern>::iterator it;
		it =  bitPattern_.begin();
		for(; it != bitPattern_.end(); it++)
		{
			cout<<it->first<<" -> "<<(unsigned int)it->second.nbits<<"  "<<setbase(16)<<it->second.bits<<endl;
		}
	}
	map<char, Pattern>& getMap(){
		return bitPattern_;
	}
	
private:
	map<char, Pattern> bitPattern_;			
};

}
}
}
#endif /*BITPATTERN_H_*/
