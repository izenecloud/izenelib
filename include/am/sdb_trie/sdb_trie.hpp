#ifndef _EDGE_TABLE_H_
#define _EDGE_TABLE_H_

#include <iostream>

#include <am/concept/DataType.h>
#include <sdb/SequentialDB.h>
#include "compoundkey.h"
#include "compoundvalue.h"
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
          typename UserDataType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie : public AccessMethod<std::vector<CharType>, UserDataType>
{
    typedef CompoundKey<NodeIDType, CharType> KeyType;
    typedef CompoundValue<NodeIDType, UserDataType> ValueType;
    typedef DataType<KeyType, ValueType> RecordType;
    typedef izenelib::sdb::ordered_sdb<KeyType, ValueType , LockType> DBType;

public:

    SDBTrie(const std::string& dbname)
    :   dbname_(dbname),
        db_(dbname_ + ".sdb")
    {
        numItems_ = 0;
        nextNID_ = NodeIDTraits<NodeIDType>::MaxValue;

        db_.setDegree(90);
        db_.setCacheSize(16*1024);
        db_.setPageSize(8192);
    }

    virtual ~SDBTrie()
    {
        close();
        db_.close();
    }

    void config(int degree, int cacheSize, int pageSize)
    {
        db_.setDegree(degree);
        db_.setCacheSize(cacheSize);
        db_.setPageSize(pageSize);
    }

    void open()
    {
        db_.open();
        KeyType key(NodeIDTraits<NodeIDType>::EmptyValue,
            NumericTraits<CharType>::MinValue);
        ValueType value;
        if( db_.getValue(key, value) )
            numItems_ = (int)value.value1;
        nextNID_ = db_.numItems() + NodeIDTraits<NodeIDType>::MinValue;
    }

    void close()
    {
        KeyType key(NodeIDTraits<NodeIDType>::EmptyValue,
            NumericTraits<CharType>::MinValue);
        ValueType value(numItems_, false, UserDataType());
        db_.update(key, value);
    }

    bool insert(const std::vector<CharType>& key, const UserDataType& value)
    {
        if(key.size() == 0) return false;

        NodeIDType parent = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i< key.size()-1; i++ )
        {
            NodeIDType tmp = NodeIDType();
            addEdge(key[i], parent, tmp);
            parent = tmp;
        }

        NodeIDType nid;
        bool keyExist = false;
        UserDataType previousValue;
        if( getEdge(key[key.size()-1], parent, nid, keyExist, previousValue)
            && keyExist)
            return false;

        numItems_++;
        addEdge(key[key.size()-1], parent, nid);
        setEdgeProperty(key[key.size()-1], parent, true, value);
        return true;
    }

    bool insert(const DataType<std::vector<CharType>, UserDataType>& data)
    {
        return insert(data.key, data.value);
    }

    bool update(const std::vector<CharType>& key, const UserDataType& value)
    {
        if(key.size() == 0) return false;

        NodeIDType parent = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i< key.size()-1; i++ )
        {
            NodeIDType tmp = NodeIDType();
            addEdge(key[i], parent, tmp);
            parent = tmp;
        }

        NodeIDType nid;
        bool keyExist = true;
        UserDataType previousValue;
        if( !getEdge(key[key.size()-1], parent, nid, keyExist, previousValue)
            || !keyExist)
            numItems_++;

        addEdge(key[key.size()-1], parent, nid);
        if( false == setEdgeProperty(key[key.size()-1], parent, true, value) )
            std::cout << "update error" << std::endl;
        return true;
    }

    bool update(const DataType<std::vector<CharType>, UserDataType>& data)
    {
        return update(data.key, data.value);
    }

    bool get(const std::vector<CharType>& key, UserDataType& value)
    {
        if(key.size() == 0) return false;

        NodeIDType parent = NodeIDTraits<NodeIDType>::RootValue;
        bool keyExist;
        UserDataType previousValue;
        for( size_t i=0; i<key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge(key[i], parent, tmp, keyExist, previousValue) )
                return false;
            parent = tmp;
        }
        if(!keyExist) return false;
        value = previousValue;
        return true;
    }

    UserDataType* find(const std::vector<CharType>& key)
    {
        throw std::runtime_error("unsupported AM function, use get() instead");
    }

    bool searchRegexp(const std::vector<CharType>& regexp,
        std::vector<UserDataType>& valueList)
    {
        ValueType root(NodeIDTraits<NodeIDType>::RootValue, UserDataType(), false);
        searchRegexp_(regexp, 0, root, valueList);
        return valueList.size() ? true : false;
    }

    /**
     * @return number of nodes
     */
    unsigned int num_items()
    {
        return numItems_;
    }

    void display()
    {
        db_.display();
    }

protected:

    /**
     * Given the NID of parent node and the edge's character property,
     * generate the child node's NID.
     * @param   ch          edge's character property
     *          parentNID   NID of parent node
     *          childNID    NID of child node
     * @return  true    if child node is generated successfully
     *          false   if key exists already
     */
    bool addEdge(const CharType ch, const NodeIDType parentNID, NodeIDType& childNID)
    {
        KeyType compoundKey(parentNID, ch);
        ValueType compoundValue;

        // if NID is found, just return
        if(db_.getValue(compoundKey, compoundValue))
        {
            childNID = compoundValue.value1;
            return false;
        }

        childNID = nextNID_;
        ++nextNID_;
        if(nextNID_ > NodeIDTraits<NodeIDType>::MaxValue)
            throw std::runtime_error("NID allocation out of range");

        compoundValue.value1 = childNID;
        compoundValue.tag = false;
        compoundValue.value2 = UserDataType();
        db_.insertValue(compoundKey, compoundValue);
        return true;
    }

    bool setEdgeProperty(const CharType ch, const NodeIDType parentNID,
        const bool& keyExist, const UserDataType& data)
    {
        KeyType compoundKey(parentNID, ch);
        ValueType compoundValue;

        // Return false if edge doesn't exist
        if(!db_.getValue(compoundKey, compoundValue))
            return false;

        compoundValue.tag = keyExist;
        compoundValue.value2 = data;
        db_.update(compoundKey, compoundValue);
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
    bool getEdge(const CharType& ch, const NodeIDType& parentNID,
        NodeIDType& childNID, bool& keyExist, UserDataType& data)
    {
        KeyType compoundKey(parentNID, ch);
        ValueType compoundValue;

        // Return false if edge doesn't exist
        if(!db_.getValue(compoundKey, compoundValue))
            return false;

        childNID = compoundValue.value1;
        keyExist = compoundValue.tag;
        data = compoundValue.value2;
        return true;
    }

    void searchRegexp_(const std::vector<CharType>& regexp,
        const size_t& startPos, const ValueType& node,
        std::vector<UserDataType>& valueList)
    {
        if(startPos == regexp.size() && node.tag )
        {
            valueList.push_back(node.value2);
            return;
        }

        NodeIDType nid = node.value1;

        switch( regexp[startPos] )
        {
            case 42:    //"*"
            {
                KeyType minKey(nid, NumericTraits<CharType>::MinValue);
                KeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<RecordType> result;
                db_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ )
                    searchRegexp_(regexp, startPos, result[i].value, valueList);

                searchRegexp_(regexp, startPos+1, node, valueList);
                break;
            }
            case 63:    //"?"
            {
                KeyType minKey(nid, NumericTraits<CharType>::MinValue);
                KeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<RecordType> result;
                db_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ )
                    searchRegexp_(regexp, startPos+1, result[i].value, valueList);
                break;
            }
            default:
            {
                KeyType key(nid, regexp[startPos]);
                ValueType nxtNode = ValueType();
                if( db_.getValue(key, nxtNode) )
                    searchRegexp_(regexp, startPos+1, nxtNode, valueList);
                break;
            }
        }

    }

private:

    std::string dbname_;

    DBType db_;

    int numItems_;

    NodeIDType nextNID_;

};


template <typename StringType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie2 : public AccessMethod<StringType, UserDataType>
{
    typedef typename StringType::value_type CharType;
    typedef SDBTrie<CharType, UserDataType, NodeIDType, LockType> SDBTrieType;

public:

    SDBTrie2(const std::string name)
    :   trie_(name) {}

    virtual ~SDBTrie2(){}

    void config(int degree, int cacheSize, int pageSize)
    {
        trie_.config(degree, cacheSize, pageSize);
    }

    void open(){ trie_.open(); }

    void close(){ trie_.close(); }

    bool insert(const StringType& key, const UserDataType& value)
    {
        CharType* chArray = (CharType*)key.c_str();
        size_t chCount = key.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.insert(chVector, value);
    }

    bool insert(const DataType<StringType, UserDataType>& data)
    {
        return insert(data.key, data.value);
    }

    bool update(const StringType& key, const UserDataType value)
    {
        CharType* chArray = (CharType*)key.c_str();
        size_t chCount = key.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.update(chVector, value);
    }

    bool update(const DataType<StringType, UserDataType>& data)
    {
        return update(data.key, data.value);
    }

    bool get(const StringType& key, UserDataType& value)
    {
        CharType* chArray = (CharType*)key.c_str();
        size_t chCount = key.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.get(chVector, value);
    }

    UserDataType* find(const StringType& key)
    {
        UserDataType value = UserDataType();
        if(!get(key, value))
            return NULL;
        return new UserDataType(value);
    }

    bool searchRegexp(const StringType& regexp,
        std::vector<UserDataType>& valueList)
    {
        CharType* chArray = (CharType*)regexp.c_str();
        size_t chCount = regexp.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.searchRegexp(chVector, valueList);
    }

    unsigned int num_items() { return trie_.num_items(); }

    void display() { trie_.display(); }


private:
    SDBTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
