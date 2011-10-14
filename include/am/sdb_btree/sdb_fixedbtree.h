#ifndef SDB_FIXEDBTREE_H_
#define SDB_FIXEDBTREE_H_

#include "sdb_btree.h"

NS_IZENELIB_AM_BEGIN

template<typename KeyType,typename ValueType=NullType,typename LockType=NullLock>
class sdb_fixedbtree
            : public sdb_btree<KeyType, ValueType, LockType, true>
{
public:
    sdb_fixedbtree(const std::string& fileName = "sdb_fixedbtree.dat#")
            :sdb_btree<KeyType, ValueType, LockType, true>(fileName)
    {

    }

};

NS_IZENELIB_AM_END

#endif /*SDB_FIXEDBTREE_H_*/
