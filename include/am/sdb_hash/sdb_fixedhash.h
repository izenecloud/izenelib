/**
 * @file sdb_fixedhash.h
 * @brief The header file of sdb_fixedhash.
 * @author peisheng wang
 *
 * @history
 * ==========================
 * 1. 2009-02-16 first version.
 *
 *
 * This file defines class sdb_fixedhash.
 */
#ifndef sdb_fixedhash_H
#define sdb_fixedhash_H

#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <types.h>
#include <sys/stat.h>
#include <util/ClockTimer.h>

//#include <util/log.h>
#include <stdio.h>

#include "sdb_hash.h"

using namespace std;

NS_IZENELIB_AM_BEGIN

template<typename KeyType,typename ValueType=NullType,typename LockType=NullLock>
class sdb_fixedhash
            : public sdb_hash<KeyType, ValueType, LockType, true>
{
public:
    sdb_fixedhash(const std::string& fileName = "sdb_fixedhash.dat")
            :sdb_hash<KeyType, ValueType, LockType, true>(fileName)
    {

    }

};


NS_IZENELIB_AM_END

#endif
