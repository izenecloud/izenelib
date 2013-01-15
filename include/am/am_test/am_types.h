#ifndef AM_TYPES_H_
#define AM_TYPES_H_

#include <cstdio>

//memory based
#include <am/fromylib/LinearHashTable.h>
#include <am/cccr_hash/cccr_hash.h>
#include <am/dynamic_perfect_hash/dynamic_perfect_hash.hpp>
#include <am/3rdparty/ext_hash_map.h>
//#include <am/3rdparty/persist_vector.h>
#include <am/3rdparty/rde_hash.h>
#include <am/3rdparty/stx_btree.h>

//file based
#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <am/sdb_btree/sdb_fixedbtree.h>
#include <am/tokyo_cabinet/tc_hash.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <am/tc/Hash.h>
#include <am/tc/BTree.h>

#endif /*AM_TYPES_H_*/


