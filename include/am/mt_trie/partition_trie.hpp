#ifndef _PARTITION_TRIE_H_
#define _PARTITION_TRIE_H_

#include <iostream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

#include <am/concept/DataType.h>
#include <hdb/HugeDB.h>
#include <sdb/SequentialDB.h>

#include <am/db_trie/common_trie.h>

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename NodeIDType,
          typename LockType>
class PartitionTrieEdgeTable : public CommonEdgeTable<CharType, NodeIDType, LockType>
{
    typedef CommonEdgeTable<CharType, NodeIDType, LockType> CommonType;
    typedef PartitionTrieEdgeTable<CharType, NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename CommonType::EdgeTableValueType EdgeTableValueType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<EdgeTableKeyType,
        EdgeTableValueType , LockType> DBType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<EdgeTableValueType,
        EdgeTableKeyType , LockType> RevertDBType;

    PartitionTrieEdgeTable(const std::string& name)
    : CommonType(name), db_(name), rdb_(name + ".revert") { }

    virtual ~PartitionTrieEdgeTable() { }

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

        rdb_.setDegree(128);
        rdb_.setPageSize(8192);
        rdb_.setCachedRecordsNumber(2500000);
        rdb_.setMergeFactor(2);
        rdb_.open();
    }

    void flush()
    {
        db_.flush();
        rdb_.flush();
    }

    void close()
    {
        db_.close();
        rdb_.close();
    }

    void optimize()
    {
        db_.optimize();
    }

    virtual void insertValue(const EdgeTableKeyType& key, const EdgeTableValueType & value)
    {
        db_.insertValue(key, value);
        rdb_.insertValue(value, key);
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

    RevertDBType& getRevertDB()
    {
        return rdb_;
    }

private:

    DBType db_;

    RevertDBType rdb_;
};

template <typename NodeIDType,
          typename LockType>
class PartitionTrieDataTable : public CommonDataTable<NodeIDType, int, LockType>
{
    typedef CommonDataTable<NodeIDType, int, LockType> CommonType;
    typedef PartitionTrieDataTable<NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::DataTableKeyType DataTableKeyType;
    typedef typename CommonType::DataTableValueType DataTableValueType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<
        DataTableKeyType,
        DataTableValueType,
        LockType> DBType;

    PartitionTrieDataTable(const std::string& name)
    : CommonType(name), db_(name) { }

    virtual ~PartitionTrieDataTable() { }

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

template <typename CharType,
          typename NodeIDType,
          typename LockType>
class FinalTrieEdgeTable : public CommonEdgeTable<CharType, NodeIDType, LockType>
{
    typedef CommonEdgeTable<CharType, NodeIDType, LockType> CommonType;
    typedef FinalTrieEdgeTable<CharType, NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename CommonType::EdgeTableValueType EdgeTableValueType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<EdgeTableKeyType,
        EdgeTableValueType , LockType> DBType;

    FinalTrieEdgeTable(const std::string& name)
    : CommonType(name), db_(name) { }

    virtual ~FinalTrieEdgeTable() { }

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
          typename LockType>
class FinalTrieDataTable : public CommonDataTable<NodeIDType, NullType, LockType>
{
    typedef CommonDataTable<NodeIDType, NullType, LockType> CommonType;
    typedef FinalTrieDataTable<NodeIDType, LockType> ThisType;

public:

    typedef typename CommonType::DataTableKeyType DataTableKeyType;
    typedef typename CommonType::DataTableValueType DataTableValueType;

    typedef izenelib::hdb::ordered_hdb_fixed_no_delta<
        DataTableKeyType,
        DataTableValueType,
        LockType> DBType;

    FinalTrieDataTable(const std::string& name)
    : CommonType(name), db_(name) { }

    virtual ~FinalTrieDataTable() { }

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
          typename NodeIDType>
class PartitionTrie
    : public CommonTrie<StringType,
                        int,
                        NodeIDType,
                        PartitionTrieEdgeTable<typename StringType::value_type,
                                               NodeIDType,
                                               NullLock>,
                        PartitionTrieDataTable  <NodeIDType,
                                                 NullLock> >
{
public:
    typedef PartitionTrieEdgeTable<typename StringType::value_type,
                                   NodeIDType,
                                   NullLock>
            EdgeTableType;

    typedef PartitionTrieDataTable  <NodeIDType, NullLock>
            DataTableType;

    PartitionTrie(const std::string name)
    :   CommonTrie <StringType, int, NodeIDType,
                    EdgeTableType, DataTableType> (name) {}

    virtual ~PartitionTrie() {}
};

/**
 * @brief Definitions for the final tries merged by all partitions.
 */
template <typename StringType,
          typename NodeIDType>
class FinalTrie
    : public CommonTrie<StringType,
                        NullType,
                        NodeIDType,
                        FinalTrieEdgeTable    <typename StringType::value_type,
                                               NodeIDType,
                                               izenelib::util::ReadWriteLock>,
                        FinalTrieDataTable  <NodeIDType,
                                               izenelib::util::ReadWriteLock> >
{
public:
    typedef FinalTrieEdgeTable    <typename StringType::value_type,
                                   NodeIDType,
                                   izenelib::util::ReadWriteLock>
            EdgeTableType;

    typedef FinalTrieDataTable  <NodeIDType, izenelib::util::ReadWriteLock>
            DataTableType;

    FinalTrie(const std::string name)
    :   CommonTrie <StringType, NullType, NodeIDType,
                    EdgeTableType, DataTableType> (name) {}
    virtual ~FinalTrie() {}
};



/**
 * @brief Definitions for the final tries merged by all partitions.
 */
template <typename StringType,
          typename NodeIDType>
class FinalL1Trie
    : public CommonTrie<StringType,
                        int,
                        NodeIDType,
                        FinalTrieEdgeTable    <typename StringType::value_type,
                                               NodeIDType,
                                               izenelib::util::ReadWriteLock>,
                        PartitionTrieDataTable  <NodeIDType,
                                               izenelib::util::ReadWriteLock> >
{
public:
    typedef FinalTrieEdgeTable    <typename StringType::value_type,
                                   NodeIDType,
                                   izenelib::util::ReadWriteLock>
            EdgeTableType;

    typedef PartitionTrieDataTable  <NodeIDType, izenelib::util::ReadWriteLock>
            DataTableType;

    FinalL1Trie(const std::string name)
    :   CommonTrie <StringType, int, NodeIDType,
                    EdgeTableType, DataTableType> (name) {}
    virtual ~FinalL1Trie() {}
};

/**
 * @brief Algorithm: merge a PartitionTrie to a FinalTrie
 */
template<typename FinalTrieType, typename PartitionTrieType, typename NodeIDType>
void mergeToFinal(FinalTrieType& finalTrie, PartitionTrieType& partitionTrie, const NodeIDType& fromNID)
{
    typedef typename PartitionTrieType::EdgeTableType PTEdgeTableType;
    typedef typename PTEdgeTableType::RevertDBType PTEdgeTableRevertDBType;

    typedef typename FinalTrieType::EdgeTableType FTEdgeTableType;
    typedef typename FTEdgeTableType::DBType FTEdgeTableDBType;

    typedef typename PartitionTrieType::DataTableType PTDataTableType;
    typedef typename PTDataTableType::DBType PTDataTableDBType;

    typedef typename FinalTrieType::DataTableType FTDataTableType;
    typedef typename FTDataTableType::DBType FTDataTableDBType;

    typedef typename PTEdgeTableType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename PTEdgeTableType::EdgeTableValueType EdgeTableValueType;

    typedef typename PTDataTableType::DataTableKeyType DataTableKeyType;
    typedef typename PTDataTableType::DataTableValueType DataTableValueType;

    PTEdgeTableRevertDBType& e1r = partitionTrie.getEdgeTable().getRevertDB();
    FTEdgeTableDBType& e2 = finalTrie.getEdgeTable().getDB();
    DataType<EdgeTableValueType, EdgeTableKeyType> redge;

    typename PTEdgeTableRevertDBType::HDBCursor ecur = e1r.search(fromNID);
    while(e1r.get(ecur,redge)) {
        e2.insertValue(redge.value, redge.key);
        e1r.seq(ecur);
    }
    e1r.clear();

    PTDataTableDBType& d1 = partitionTrie.getDataTable().getDB();
    FTDataTableDBType& d2 = finalTrie.getDataTable().getDB();
    DataType<DataTableKeyType, DataTableValueType> data;
    typename PTDataTableDBType::HDBCursor dcur = d1.get_first_locn();
    while(d1.get(dcur,data)) {
        d2.insertValue(data.key, NullType());
        d1.seq(dcur);
    }
    d1.clear();
}

template<typename FinalL1TrieType, typename PartitionTrieType, typename NodeIDType>
void mergeToFinalL1(FinalL1TrieType& finalTrie, PartitionTrieType& partitionTrie, const NodeIDType& fromNID)
{
    typedef typename PartitionTrieType::EdgeTableType PTEdgeTableType;
    typedef typename PTEdgeTableType::RevertDBType PTEdgeTableRevertDBType;

    typedef typename FinalL1TrieType::EdgeTableType FTEdgeTableType;
    typedef typename FTEdgeTableType::DBType FTEdgeTableDBType;

    typedef typename PartitionTrieType::DataTableType PTDataTableType;
    typedef typename PTDataTableType::DBType PTDataTableDBType;

    typedef typename FinalL1TrieType::DataTableType FTDataTableType;
    typedef typename FTDataTableType::DBType FTDataTableDBType;

    typedef typename PTEdgeTableType::EdgeTableKeyType EdgeTableKeyType;
    typedef typename PTEdgeTableType::EdgeTableValueType EdgeTableValueType;

    typedef typename PTDataTableType::DataTableKeyType DataTableKeyType;
    typedef typename PTDataTableType::DataTableValueType DataTableValueType;

    PTEdgeTableRevertDBType& e1r = partitionTrie.getEdgeTable().getRevertDB();
    FTEdgeTableDBType& e2 = finalTrie.getEdgeTable().getDB();
    DataType<EdgeTableValueType, EdgeTableKeyType> redge;

    typename PTEdgeTableRevertDBType::HDBCursor ecur = e1r.search(fromNID);
    while(e1r.get(ecur,redge)) {
        e2.insertValue(redge.value, redge.key);
        e1r.seq(ecur);
    }
    e1r.clear();

    PTDataTableDBType& d1 = partitionTrie.getDataTable().getDB();
    FTDataTableDBType& d2 = finalTrie.getDataTable().getDB();
    DataType<DataTableKeyType, DataTableValueType> data;
    typename PTDataTableDBType::HDBCursor dcur = d1.get_first_locn();
    while(d1.get(dcur,data)) {
        d2.insertValue(data.key, data.value );
        d1.seq(dcur);
    }
    d1.clear();
}


NS_IZENELIB_AM_END

#endif
