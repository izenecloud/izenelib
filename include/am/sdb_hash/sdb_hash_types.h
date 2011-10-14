#ifndef SDB_HASH_TYPES_H_
#define SDB_HASH_TYPES_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
//#include <am/util/Wrapper.h>
#include <am/util/SdbUtil.h>

#include <util/RefCount.h>
#include <util/hashFunction.h>
#include <util/izene_serialization.h>
#include <util/ProcMemInfo.h>

//#include <boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>

using namespace std;

using namespace izenelib::am::util;

NS_IZENELIB_AM_BEGIN

inline uint32_t rotate(uint32_t a)
{

    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;

}


/**
 *  \brief hashing function object used for sdb_hash
 */
class sdb_hashing
{
public:
    static uint32_t hash_fun(const void* kbuf, const size_t ksize)
    {
        uint32_t convkey = 0;
        const char* str = (const char*)kbuf;
        for (size_t i = 0; i < ksize; i++)
            convkey = 37*convkey + (uint8_t)*str++;
        return convkey;
        //return rotate(convkey);
    }
};


NS_IZENELIB_AM_END

#endif /*SDB_HASH_TYPES_H_*/
