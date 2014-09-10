/**
 *
 * @file cm_basics.h
 * @brief It provides besical definition of DATA TYPE used in namespace sf1lib.
 *
 * This file defines basical Data Type and HashFun for Hash_Map.
 */

#ifndef CM_BASICS_H
#define CM_BASICS_H


#include <am/3rdparty/rde_hash.h>

#include <util/ProcMemInfo.h>

using namespace izenelib::am;

/**
 *  \brief namespace for Search Formula 1(product SF-1) library.
 */

namespace izenelib
{
namespace cache
{

enum FOUND_RESULT
{
    NOT_FOUND = 0,
    FOUND_IN_MEM,
    FOUND_IN_FILE
};

enum HASH_STATUS
{
    BOTH_FULL = 0,
    FILE_FULL,
    MEM_FULL,
    BOTH_NOT_FULL
};

enum DUMP_RESULT
{
    DUMP_FAILED = -1,
    NO_DUMP,
    DUMP_M2F,
    DUMP_F2M
};

enum DUMP_OPTION
{
    NONE = 0,
    ONLY_DUMP,
    DUMP_EVICT,
    ONLY_EVICT,
    DUMP_LATER
};

/**
 *  Hash function object for hash_map. For different KeyType,
 *  should have different hash function object. It is used in Class CacheDB, MCache, and MFCache.
 *
 */

template<class T> struct HashFun
{
    size_t operator()(const T& key) const
    {
        return izenelib::util::izene_hashing(key);
    }
};

}
}

#endif
