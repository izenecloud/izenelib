/**
 *
 * @file cm_basics.h
 * @brief It provides besical definition of DATA TYPE used in namespace sf1lib. 
 *
 * This file defines basical Data Type and HashFun for Hash_Map.
 */

#ifndef CM_BASICS_H
#define CM_BASICS_H

#include <iostream>

#include <am/fromylib/LinearHashTable.h>
#include <am/cccr_string_hash_table/cccr_str_hash_table.hpp>
#include <am/sdb_hash/sdb_hash.h>
#include <am/cccr_hash/cccr_hash.h>
#include <util/ProcMemInfo.h>

using namespace izenelib::util;


//#include <types.h>
//#include <am/am.h>
//#include <am/concept/DataType.h>

//#include <am/util/DbObj.h>
//#include <am/util/Wrapper.h>
//#include <am/util/SdbUtil.h>
//#include <util/RefCount.h>
//#include <boost/static_assert.hpp>

//#include <am/ExtendibleHash.h>
//#include <am/ExtendibleHashFile.h>
//#include <am/LinearHashTable.h>
//#include <am/LinearHashFile.h>


using namespace std;

/**
 *	\brief namespace for Search Formula 1(product SF-1) library.
 */

namespace izenelib{
namespace cache{

typedef enum {NOT_FOUND=0, FOUND_IN_MEM, FOUND_IN_FILE} FOUND_RESULT;
typedef enum {BOTH_FULL=0, FILE_FULL, MEM_FULL, BOTH_NOT_FULL} HASH_STATUS;
typedef enum {DUMP_FAILED=-1, NO_DUMP,DUMP_M2F,DUMP_F2M} DUMP_RESULT;
typedef enum {NONE=0, ONLY_DUMP, DUMP_EVICT, ONLY_EVICT, DUMP_LATER}
		DUMP_OPTION;

/*
 *	Hash function object for hash_map. For different KeyType,
 *	should have different hash function object. It is used in Class CacheDB, MCache, and MFCache.  	
 *	
 */

template<class T> struct HashFun {
	size_t operator()(const T& key) const {	
		return izenelib::util::HashFunction<T>::convert_key(key) % HashFunction<T>::PRIME;
	}
};

}

}

/*
namespace boost {
namespace serialization {
template<typename Archive> void serialize(Archive & ar, YString & t,
		const unsigned int) {
	string temp;
	temp = t;
	ar & temp;
	t = temp;

}
}
}*/


#endif
