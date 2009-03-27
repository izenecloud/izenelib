#ifndef SDB_UTIL_H_
#define SDB_UTIL_H_

#include "../../types.h"
#include <cstdlib>
#include <cstddef>
#include <cassert>
using namespace std;

NS_IZENELIB_AM_BEGIN

namespace util {

	enum ESeqPos
	{
		ESP_START = 0, // start iterating through the entire tree
		ESP_KEY, // start from the key provided
		ESP_CONT // continue from the last position
	};

	enum ESeqDirection
	{
		ESD_FORWARD = 0, // iterate forwards through the tree
		ESD_BACKWARD // seek backwards through the tree
	};

	const int BOOST_SERIAZLIZATION_HEAD_LENGTH = 40;

	const size_t BLOCK_SIZE = 16;

	inline int uint_to_bytes(unsigned int val, char* dest) {
		unsigned int i;
		for(i = 0; i < sizeof(int); i++) {
			dest[i] = (char)((val) & 0xFF);
			val >>= 8;
			if( val == 0)
			break;
		}
		return i;
	}

	inline unsigned int bytes_to_uint(int len, char * vals) {
		int dest = 0;
		for(int i = 0; i < len; i++) {
			dest = (dest << 8) | (vals[sizeof(int) - i - 1] & 0xFF);
		}
		return dest;
	}

}

NS_IZENELIB_AM_END

#endif /*SDB_UTIL_H_*/
