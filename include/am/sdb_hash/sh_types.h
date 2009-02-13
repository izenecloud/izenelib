#ifndef SH_TYPES_H_
#define SH_TYPES_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
#include <am/util/Wrapper.h>
#include <am/util/MemMap.h>
#include <am/util/SdbUtil.h>

#include <util/RefCount.h>
#include <util/hashFunction.h>

#include <boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>


using namespace std;

using namespace izenelib::am::util;

NS_IZENELIB_AM_BEGIN


class sdb_hashing {
public:
	static uint32_t hash_fun(const void* kbuf, const size_t ksize) {
		uint32_t convkey = 0;
		const char* str = (const char*)kbuf;
		for (size_t i = 0; i < ksize; i++)
		convkey = 37*convkey + (uint8_t)*str++;
		return convkey;
	}
};



NS_IZENELIB_AM_END


#endif /*SLF_TYPES_H_*/
