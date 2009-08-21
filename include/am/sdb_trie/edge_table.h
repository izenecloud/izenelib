#ifndef _EDGE_TABLE_H_
#define _EDGE_TABLE_H_

#include <iostream>

#include <am/concept/DataType.h>
#include <sdb/SequentialDB.h>
#include "compoundkey.h"
#include "traits.h"

NS_IZENELIB_AM_BEGIN

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
    typedef CompoundKey<NodeIDType, CharType> KeyType;
    typedef NodeIDType ValueType;
    typedef DataType<KeyType, ValueType> EdgeTableDataType;
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
        KeyType key(parentNID, ch);

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
        KeyType key(parentNID, ch);

        if(db_.getValue(key, childNID))
            return true;
        return false;
    }

    /**
     * Iterate all child nodes for a given parent node, inside a given depth.
     * @param   parentNID       NID of parent node
     *          childNIDList    a NID list of all child nodes
     *          depth           a positive value controls how many layers be retrieved,
     *                          set to 0 will return nothing,
     *                          while set to -1 (default) means no limit at all.
     */
    void iterate(const NodeIDType parentNID,
        std::vector<NodeIDType>& childNIDList,
        const int depth = -1 )
    {
        if(depth == 0) return;
        KeyType minKey(parentNID, NumericTraits<CharType>::MinValue);
        KeyType maxKey(parentNID, NumericTraits<CharType>::MaxValue);

        std::vector<EdgeTableDataType> result;
        db_.getValueBetween(result, minKey, maxKey);

        for(size_t i = 0; i <result.size(); i++ )
        {
            childNIDList.push_back(result[i].value);
            iterate(result[i].value, childNIDList, depth-1);
        }
    }

    /**
     * @return number of nodes
     */
    unsigned int num_items()
    {
        return db_.numItems();
    }

    void display()
    {
        db_.display();
    }

private:

    std::string dbname_;

    DBType db_;

    NodeIDType nextNID_;

};

NS_IZENELIB_AM_END

#endif
