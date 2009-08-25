#ifndef _ONE2ONE_MAPPING_TABLE_H_
#define _ONE2ONE_MAPPING_TABLE_H_

#include <am/concept/DataType.h>
#include <am/util/DbObj.h>
#include <sdb/SequentialDB.h>

NS_IZENELIB_AM_BEGIN

/**
 * Maintain one to one mapping in SDB.
 * Used to help implement some tables in both SDBTrie and SDBSuffixTrie,
 * such as LeafNodeTable.
 */
template <typename KeyType,
          typename ValueType = NullType,
          typename LockType = izenelib::util::NullLock>
class One2OneMappingTable
{
    typedef izenelib::sdb::ordered_sdb<KeyType, ValueType , LockType> DBType;

public:

    One2OneMappingTable(const std::string& dbname)
    :   dbname_(dbname),
        db_(dbname_ + ".sdb")
    {
        db_.open();
    }

    virtual ~One2OneMappingTable(){}

    /**
     * Store a pair of key and value into SDB.
     * @return true     successfully
     *         false    key exists already
     */
    bool put( const KeyType& k, const ValueType& v)
    {
        return db_.insertValue(k,v);
    }

    /**
     * Update value in SDB for a given key, insert if key doesnot exist.
     */
    void update( const KeyType& k, const ValueType& v)
    {
        db_.update(k, v);
    }

    /**
     * Retrieve value stored in SDB by given key.
     * @return true     successfully
     *         false    given key does not exist
     */
    bool get( const KeyType& k, ValueType& v)
    {
        return db_.getValue(k, v);
    }

    /**
     * @return number of stored KV pairs.
     */
    unsigned int num_items()
    {
        return db_.numItems();
    }

    /******************************************************************
     * Following APIs are valid only when ValueType is NullType
     ******************************************************************/

    /**
     * Store a key into SDB.
     * @return true     successfully
     *         false    key exists already
     */
    bool put( const KeyType& k)
    {
        NullType null;
        return db_.insertValue(k,null);
    }

    /**
     * To see if a key exists in SDB.
     * @return true     successfully
     *         false    given key does not exist
     */
    bool get( const KeyType& k)
    {
        NullType null;
        return db_.getValue(k, null);
    }

private:

    std::string dbname_;

    DBType db_;
};

NS_IZENELIB_AM_END

#endif
