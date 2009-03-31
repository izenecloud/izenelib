#ifndef SLF_TYPES_H_
#define SLF_TYPES_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
#include <am/util/Wrapper.h>
#include <am/util/MemMap.h>
#include <am/util/SdbUtil.h>
#include <util/RefCount.h>

#include <boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>

//#include "SkipListFileException.h"

using namespace std;

using namespace izenelib::am::util;

NS_IZENELIB_AM_BEGIN

typedef int NodeID;

const int MAX_LEVEL = 32;

/**
 *   flip a coin until a tail come up. Tail come up with a probability 1/b.
 *   \param b probability is 1/b/
 *   \return the least times that the tail come up. 
 */
inline int coin_flip(int b){
	int t = 1;
	while( rand()%b == 0 ){
		t++;
	}
	return t;	
}


vector<MemBlock* > mbList;

NS_IZENELIB_AM_END


#endif /*SLF_TYPES_H_*/
