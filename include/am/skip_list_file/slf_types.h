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


vector<MemBlock* > mbList;

NS_IZENELIB_AM_END


#endif /*SLF_TYPES_H_*/
