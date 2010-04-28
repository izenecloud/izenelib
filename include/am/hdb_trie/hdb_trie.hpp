#ifndef _HDB_TRIE_H_
#define _HDB_TRIE_H_

#include <string>

#include <am/concept/DataType.h>
#include <hdb/HugeDB.h>

#include <am/db_trie/common_trie.h>

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename NodeIDType,
          typename LockType>
class HdbTrieEdgeTable : public CommonEdgeTable<CharType, NodeIDType, LockType>
{
    typedef CommonEdgeTable<CharType, NodeIDType, LockType> CommonType;
    typedef HdbTrieEdgeTable<CharType, NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename CommonType::EdgeTableValueType EdgeTableValueType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<EdgeTableKeyType,
        EdgeTableValueType , LockType> DBType;

    HdbTrieEdgeTable(const std::string& name)
    : CommonType(name), db_(name) { }

    virtual ~HdbTrieEdgeTable() { }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void open()
    {
        db_.setDegree(128);
        db_.setPageSize(8192);
        db_.setCachedRecordsNumber(2500000);
        db_.setMergeFactor(2);
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
        db_.optimize();
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
        return db_.getValueBetween(results, start, end);
    }

    DBType& getDB()
    {
        return db_;
    }

private:

    DBType db_;
};


template <typename NodeIDType,
          typename UserDataType,
          typename LockType>
class HdbTrieDataTable : public CommonDataTable<NodeIDType, UserDataType, LockType>
{
    typedef CommonDataTable<NodeIDType, UserDataType, LockType> CommonType;
    typedef HdbTrieDataTable<NodeIDType, UserDataType, LockType> ThisType;

public:

    typedef typename CommonType::DataTableKeyType DataTableKeyType;
    typedef typename CommonType::DataTableValueType DataTableValueType;

    typedef izenelib::hdb::ordered_hdb_no_delta<
        DataTableKeyType,
        DataTableValueType,
        LockType> DBType;

    HdbTrieDataTable(const std::string& name)
    : CommonType(name), db_(name) { }

    virtual ~HdbTrieDataTable() { }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void open()
    {
        db_.setDegree(32);
        db_.setPageSize(1024);
        db_.setCachedRecordsNumber(2500000);
        db_.setMergeFactor(2);
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
        db_.optimize();
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
class HDBTrie
    : public CommonTrie<StringType,
                        UserDataType,
                        NodeIDType,
                        HdbTrieEdgeTable      <typename StringType::value_type,
                                               NodeIDType,
                                               LockType>,
                        HdbTrieDataTable      <NodeIDType,
                                               UserDataType,
                                               LockType> >
{
public:
    typedef HdbTrieEdgeTable      <typename StringType::value_type,
                                   NodeIDType,
                                   LockType>
            EdgeTableType;

    typedef HdbTrieDataTable      <NodeIDType, UserDataType, LockType>
            DataTableType;

    typedef CommonTrie <StringType, UserDataType, NodeIDType,
                EdgeTableType, DataTableType> CommonType;

    HDBTrie(const std::string name)
    :   CommonType(name) {}

    void openForWrite()
    {
        CommonType::open();
    }

    void openForRead()
    {
        CommonType::open();
    }

    virtual ~HDBTrie() {}
};


#define HDBTrie2 HDBTrie


NS_IZENELIB_AM_END

#endif
