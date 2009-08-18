#ifndef _EDGE_TABLE_H_
#define _EDGE_TABLE_H_

#include <sdb/SequentialDB.h>

NS_IZENELIB_AM_BEGIN

/**
 * Key for SDB data structure used inside EdgeTable.
 * @see EdgeTable
 */
template <typename CharType, typename NodeIDType>
class EdgeTableKeyType
{
    typedef EdgeTableKeyType<CharType, NodeIDType> ThisType;

    EdgeTable(CharType c, NodeIDType p)
    :   ch(c), parentNID(p) {}

    int compare(const ThisType& other)
    {
        if( parentID < other.parentNID )
            return -1;
        if( parentID > other.parentNID )
            return 1;
        if( ch < other.ch )
            return -1;
        if( ch > other.ch )
            return 1;
        return 0;
    }

public:

    CharType ch;
    NodeIDType parentNID;
};

MAKE_MEMCPY_TYPE(EdgeTableKeyType)

/**
 * Maintain all edges' information in a Trie data structure with the form of
 * database table.
 * As all kinds of graph in computer science, Trie's graph representation also
 * contains two types of basic elements, node and edge.
 * Be different from other graphs, Each edge in Trie has a character propery.
 * So Given a node, the edges from the root node to the given node form a path,
 * and all the characters on the path form a string. That is to say, each node in
 * Trie can represent a string.
 * Node may contains pointers to numerous child nodes, A parent node P has pointer
 * to a child node C means, the string represented by P is the prefix of the string
 * represented by C. For example, node "ca" is the parent of node "cat".
 * Node can be an leaf node means, the string it represents is a user input string.
 * What is more, a node is a leaf node DOES NOT indicate it does not have children.
 * For example, user insert "cat" into trie, so node "cat" is a leaf node, however,
 * if user insert "catch" too, then leaf node "cat" could become the parent of
 * node "catc".
 */
template <typename CharType,
          typename NodeIDType,
          typename LockType = izenelib::util::NullLock>
class EdgeTable
{
    typedef EdgeTableKeyType<CharType, NodeIDType> KeyType;
    typedef NodeIDType ValueType;
    typedef izenelib::sdb::ordered_sdb<KeyType, ValueType , LockType> DBType;

public:

    EdgeTable(const std::string& dbname)
    :   dbname_(dbname),
        db_(dbname_ + ".sdb")
    {
        db_.open();
        nextNID_ = NodeIDTraits<NodeIDType>::MinValue;
    }

    virtual ~EdgeTable(){}

    /**
     * Given the NID of parent node and the edge's character property,
     * generate the child node's NID.
     * @param   ch          edge's character property
     *          parentNID   NID of parent node
     *          childNID    NID of child node
     * @return  true    if child node is generated successfully
     *          false   if key exists already
     */
    bool put(const CharType ch, const NodeIDType parentNID, NodeIDType& childNID)
    {
        EdgeTableKeyType key(ch,parentNID);

        // if NID is found, just return
        if(db_.getValue(key, childNID))
            return false;

        childNID = nextNID_;
        ++nextNID_;
        if(nextNID_ > NodeIDTraits<NodeIDType>::MaxValue)
            throw std::runtime_error("NID allocation out of range");

        db_.insertValue(key, childNID);
        return true;
    }

    /**
     * Given the NID of parent node and the edge's character property,
     * find the child node's NID.
     * @param   ch          edge's character property
     *          parentNID   NID of parent node
     *          childNID    NID of child node
     * @return  true    if successfully
     *          false   if child does not exist yet
     */
    bool get(const CharType ch, const NodeIDType parentNID, NodeIDType& childNID)
    {
        EdgeTableKeyType key(ch,parentNID);

        if(db_.getValue(key, childNID))
            return true;
        return false;
    }

private:

    std::string dbname_;

    DBType db_;

    NodeIDType nextNID_;

};

NS_IZENELIB_AM_END

#endif
