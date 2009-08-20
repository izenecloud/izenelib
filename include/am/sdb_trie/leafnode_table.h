#ifndef _LEAFNODE_TABLE_H_
#define _LEAFNODE_TABLE_H_

#include <am/concept/DataType.h>
#include <am/util/DbObj.h>
#include <sdb/SequentialDB.h>

NS_IZENELIB_AM_BEGIN

/**
 * Maintain all leaf node's information in a Trie data structure with the form
 * of database table.
 * In Trie's graph representation, @see EdgeTable, each leaf node contains a
 * input string user has inserted into trie.
 * Leaf Node Table contains <NodeID, UserData> pairs for all leaf nodes,
 * UserData is the data user inserted into trie together with an input string,
 * the default type is NullType.
 */
template <typename NodeIDType,
          typename UserDataType = NullType,
          typename LockType = izenelib::util::NullLock>
class LeafNodeTable
{
    typedef NodeIDType KeyType;
    typedef UserDataType ValueType;
    typedef izenelib::sdb::ordered_sdb<KeyType, ValueType , LockType> DBType;

public:

    LeafNodeTable(const std::string& dbname)
    :   dbname_(dbname),
        db_(dbname_ + ".sdb")
    {
        db_.open();
    }

    virtual ~LeafNodeTable(){}

    /**
     * Store a leaf node (both NID and user data) into LeafNodeTable.
     * @return true     successfully
     *         false    NID exists already
     */
    bool put( const NodeIDType nid, const UserDataType& data)
    {
        return db_.insertValue(nid, data);
    }

    /**
     * Update user data in a LeafNodeTable, insert if NID doesnot exist.
     */
    void update( const NodeIDType nid, const UserDataType& data)
    {
        db_.updateValue(nid, data);
    }

    /**
     * Retrieve the user data stored in some leaf node by given NID.
     * @return true     successfully
     *         false    given NID does not exist
     */
    bool get( const NodeIDType nid, UserDataType& data)
    {
        return db_.getValue(nid, data);
    }

    /**
     * @return number of user input strings
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
