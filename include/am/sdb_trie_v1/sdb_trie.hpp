#ifndef _HDB_TRIE_H_
#define _HDB_TRIE_H_

#include <string>

#include <am/concept/DataType.h>
#include <sdb/SequentialDB.h>

#include <am/db_trie/common_trie.h>
#include "sdb_hash2btree.h"

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename NodeIDType,
          typename LockType>
class SdbTrieEdgeTable : public CommonEdgeTable<CharType, NodeIDType, LockType>
{
    typedef CommonEdgeTable<CharType, NodeIDType, LockType> CommonType;
    typedef SdbTrieEdgeTable<CharType, NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename CommonType::EdgeTableValueType EdgeTableValueType;

    typedef izenelib::sdb::unordered_sdb_fixed<EdgeTableKeyType, EdgeTableValueType , LockType> DBType;
    typedef izenelib::sdb::ordered_sdb_fixed<EdgeTableKeyType, EdgeTableValueType , LockType> OptDBType;
    typedef Conv_Sdb_Hash2Btree<DBType, OptDBType> DBConverter;

    SdbTrieEdgeTable(const std::string& name)
    : CommonType(name),
      db_(name + ".sdb"),
      odb_(name + ".opt.sdb")
    {
    }

    virtual ~SdbTrieEdgeTable() { flush(); }

    void open()
    {
        db_.setDegree(16);
        db_.setBucketSize(512);
        db_.setCacheSize(256*1024);
        db_.open();

        odb_.setDegree(32);
        odb_.setPageSize(2048);
        odb_.setCacheSize(64*1024);
        odb_.open();
    }

    void flush()
    {
        db_.flush();
        odb_.flush();
    }

    void close()
    {
        db_.close();
        odb_.close();
    }

    void optimize()
    {
        odb_.clear();

        DBConverter converter(db_, odb_, CommonType::name_ + ".sdbtrie");
        converter.startThread();
        std :: cout << converter.joinThread() << std::endl;

        odb_.flush();
    }

    virtual void insertValue(const EdgeTableKeyType& key, const EdgeTableValueType & value)
    {
        db_.insertValue(key, value);
    }

    virtual bool getValue(const EdgeTableKeyType& key, EdgeTableValueType & value)
    {
        return db_.getValue(key, value);
    }

    virtual bool getValueBetween( std::vector<DataType<EdgeTableKeyType,EdgeTableValueType> > & results,
        const EdgeTableKeyType& start, const EdgeTableKeyType& end)
    {
        return odb_.getValueBetween(results, start, end);
    }

private:

    DBType db_;

    OptDBType odb_;

};


template <typename NodeIDType,
          typename UserDataType,
          typename LockType>
class SdbTrieDataTable : public CommonDataTable<NodeIDType, UserDataType, LockType>
{
    typedef CommonDataTable<NodeIDType, UserDataType, LockType> CommonType;
    typedef SdbTrieDataTable<NodeIDType, UserDataType, LockType> ThisType;

public:

    typedef typename CommonType::DataTableKeyType DataTableKeyType;
    typedef typename CommonType::DataTableValueType DataTableValueType;

    typedef izenelib::sdb::ordered_sdb_fixed<
        DataTableKeyType,
        DataTableValueType,
        LockType> DBType;

    SdbTrieDataTable(const std::string& name)
    : CommonType(name), db_(name + ".sdb") { }

    virtual ~SdbTrieDataTable() { flush(); }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void open()
    {
        db_.setDegree(32);
        db_.setPageSize(1024);
        db_.setCacheSize(64*1024);
        db_.open();
    }

    void flush()
    {
        db_.flush();
    }

    void close()
    {
        db_.close();
    }

    void optimize()
    {
    }

    unsigned int numItems()
    {
        return db_.numItems();
    }

    virtual void insertValue(const DataTableKeyType& key, const DataTableValueType & value)
    {
        db_.insertValue(key, value);
    }

    virtual void update(const DataTableKeyType& key, const DataTableValueType & value)
    {
        db_.update(key, value);
    }

    virtual bool getValue(const DataTableKeyType& key, DataTableValueType & value)
    {
        return db_.getValue(key, value);
    }

    DBType& getDB()
    {
        return db_;
    }

private:

    DBType db_;
};

/**
 * @brief Definitions for distributed tries on each partition.
 */
template <typename StringType,
          typename UserDataType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie
    : public CommonTrie<StringType,
                        UserDataType,
                        NodeIDType,
                        SdbTrieEdgeTable      <typename StringType::value_type,
                                               NodeIDType,
                                               LockType>,
                        SdbTrieDataTable      <NodeIDType,
                                               UserDataType,
                                               LockType> >
{
public:
    typedef SdbTrieEdgeTable      <typename StringType::value_type,
                                   NodeIDType,
                                   LockType>
            EdgeTableType;

    typedef SdbTrieDataTable      <NodeIDType, UserDataType, LockType>
            DataTableType;

    typedef CommonTrie <StringType, UserDataType, NodeIDType,
                EdgeTableType, DataTableType> CommonType;

    SDBTrie(const std::string name)
    :   CommonType(name) {}

    void openForWrite()
    {
        CommonType::open();
    }

    void openForRead()
    {
        CommonType::open();
    }

    virtual ~SDBTrie() {}
};

#define SDBTrie2 SDBTrie


NS_IZENELIB_AM_END

#endif
