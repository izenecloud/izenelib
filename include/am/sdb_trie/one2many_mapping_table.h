#ifndef _ONE2MANY_MAPPING_TABLE_H_
#define _ONE2MANY_MAPPING_TABLE_H_

#include <am/concept/DataType.h>
#include <am/util/DbObj.h>
#include <sdb/SequentialDB.h>
#include "compoundkey.h"
#include "traits.h"

NS_IZENELIB_AM_BEGIN

/**
 * Maintain one to many mapping in SDB.
 * Used to help implement some tables in both SDBTrie and SDBSuffixTrie,
 * such as SuffixTable.
 */
template <typename KeyType,
          typename ValueType,
          typename LockType = izenelib::util::NullLock>
class One2ManyMappingTable
{
    typedef CompoundKey<KeyType, ValueType> CompoundKeyType;
    typedef DataType<CompoundKeyType, NullType> CompoundDataType;
    typedef izenelib::sdb::ordered_sdb<CompoundKeyType, NullType, LockType> DBType;

public:

    One2ManyMappingTable(const std::string& dbname)
    :   dbname_(dbname),
        db_(dbname_ + ".sdb")
    {
        db_.open();
    }

    virtual ~One2ManyMappingTable(){}

    /**
     * Store a pair of key and value into SDB.
     * @return true     successfully
     *         false    key exists already
     */
    bool put( const KeyType k, const ValueType& v)
    {
        NullType null;
        return db_.insertValue(CompoundKeyType(k,v),null);
    }

    /**
     * Retrieve value stored in SDB by given key.
     * @return true     successfully
     *         false    given key does not exist
     */
    bool get( const KeyType k, std::vector<ValueType>& v)
    {
        NullType null;
        CompoundKeyType minKey(k, NumericTraits<ValueType>::MinValue);
        CompoundKeyType maxKey(k, NumericTraits<ValueType>::MaxValue);

        std::vector<CompoundDataType> result;
        db_.getValueBetween(result, minKey, maxKey);
        if(result == 0)
            return false;

        v.reserve(result.size());
        for(size_t i = 0; i< result.size(); i++ )
            v.push_back(result[i].key.key2);
        return true;
    }

    /**
     * @return number of stored Keys.
     */
    unsigned int num_items()
    {
        return db_.numItems();
    }

private:

    std::string dbname_;

    DBType db_;
};

NS_IZENELIB_AM_END

#endif
