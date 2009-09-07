#ifndef _SDB_TRIE_H_
#define _SDB_TRIE_H_

#include <iostream>

#include <am/concept/DataType.h>
#include <sdb/SequentialDB.h>
#include "compoundkey.h"
#include "sdb_hash2btree.h"
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
    typedef CompoundKey<NodeIDType, CharType> EdgeTableKeyType;
    typedef NodeIDType EdgeTableValueType;
    typedef DataType<EdgeTableKeyType, EdgeTableValueType> EdgeTableRecordType;
    typedef izenelib::sdb::unordered_sdb_fixed<EdgeTableKeyType, EdgeTableValueType , LockType> EdgeTableType;
    typedef izenelib::sdb::ordered_sdb_fixed<EdgeTableKeyType, EdgeTableValueType , LockType> OptEdgeTableType;
    typedef Conv_Sdb_Hash2Btree<EdgeTableType, OptEdgeTableType> EdgeTableConverter;

    typedef NodeIDType DataTableKeyType;
    typedef UserDataType DataTableValueType;
    typedef DataType<DataTableKeyType, DataTableValueType> DataTableRecordType;
    typedef izenelib::sdb::ordered_sdb_fixed<DataTableKeyType, DataTableValueType , LockType> DataTableType;

public:

    SDBTrie(const std::string& dbname)
    :   closed_(true), dbname_(dbname),
        edgeTable_(dbname_ + ".sdbtrie.edgetable.sdb"),
        optEdgeTable_(dbname_ + ".sdbtrie.optedgetable.sdb"),
        dataTable_(dbname_ + ".sdbtrie.datatable.sdb")
    {
        nextNID_ = NodeIDTraits<NodeIDType>::MaxValue;
    }

    virtual ~SDBTrie()
    {
        if(!closed_)
            close();
    }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void openForRead()
    {
        if(closed_)
        {
            read_ = true;
            closed_ = false;

            optEdgeTable_.setDegree(32);
            optEdgeTable_.setPageSize(2048);

            dataTable_.setDegree(32);
            dataTable_.setPageSize(1024);

            optEdgeTable_.setCacheSize(128*1024);
            optEdgeTable_.open();

            dataTable_.setCacheSize(64*1024);
            dataTable_.open();

        } else if( !read_ )
            throw std::runtime_error("close before open read mode");
    }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void openForWrite()
    {
        if(closed_)
        {
            read_ = false;
            closed_ = false;

            optEdgeTable_.setDegree(32);
            optEdgeTable_.setPageSize(2048);

            dataTable_.setDegree(32);
            dataTable_.setPageSize(1024);

            edgeTable_.setDegree(16);
            edgeTable_.setBucketSize(512);

            edgeTable_.setCacheSize(256*1024);
            edgeTable_.open();

            optEdgeTable_.setCacheSize(32*1024);
            optEdgeTable_.open();

            dataTable_.setCacheSize(64*1024);
            dataTable_.open();

            nextNID_ = edgeTable_.numItems() + NodeIDTraits<NodeIDType>::MinValue;
        } else if(read_)
            throw std::runtime_error("close before open write mode");
    }

    void flush()
    {
        if(!read_) edgeTable_.flush();
        optEdgeTable_.flush();
        dataTable_.flush();
    }

    /**
     * @brief Write back informations back to db.
     */
    void close()
    {
        closed_ = true;
        if(!read_) edgeTable_.close();
        optEdgeTable_.close();
        dataTable_.close();
    }

    /**
     * @brief Optimize read performance, e.g. get() and findPrefix()
     */
    void optimize()
    {
        if(read_) return;

        EdgeTableConverter converter(edgeTable_,
            optEdgeTable_, dbname_ + ".sdbtrie");
        converter.startThread();
        converter.joinThread();

        edgeTable_.flush();
        optEdgeTable_.flush();
    }

    /**
     * @brief Insert key value pairs into Trie.
     * @return false if key exists already or key is empty
     *         true  success
     */
    bool insert(const std::vector<CharType>& key, const UserDataType& value)
    {
        if(read_ || key.size() == 0) return false;

        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i< key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            addEdge(key[i], nid, tmp);
            nid = tmp;
        }
        return putData(nid, value);
    }

    /**
     * @brief Insert key value pairs into Trie.
     * @return false if key exists already or key is empty
     *         true  success
     */
    bool insert(const DataType<std::vector<CharType>, UserDataType>& data)
    {
        return insert(data.key, data.value);
    }

    /**
     * @brief Update value for a given key, if key doesn't exist, do insertion instead.
     * @return false if key is empty
     *         true  otherwise
     */
    bool update(const std::vector<CharType>& key, const UserDataType& value)
    {
        if(read_ || key.size() == 0) return false;

        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i< key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            addEdge(key[i], nid, tmp);
            nid = tmp;
        }
        updateData(nid, value);
        return true;
    }

    /**
     * @brief Update value for a given key, if key doesn't exist, do insertion instead.
     * @return false if key is empty
     *         true  otherwise
     */
    bool update(const DataType<std::vector<CharType>, UserDataType>& data)
    {
        return update(data.key, data.value);
    }

    /**
     * @brief Get value for a given key.
     * @return false if key doesn't exist
     *         true otherwise
     */
    bool get(const std::vector<CharType>& key, UserDataType& value)
    {
        if(key.size() == 0) return false;

        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge2(key[i], nid, tmp) )
                return false;
            nid = tmp;
        }
        UserDataType tmp;
        if(! getData(nid,tmp) )
            return false;
        value = tmp;
        return true;
    }

    /**
     * @brief Have the interface just to be correspondent with am. Use get() instead.
     * @return NULL always
     */
    UserDataType* find(const std::vector<CharType>& key)
    {
        throw std::runtime_error("unsupported AM function, use get() instead");
    }

    /**
     * @brief Get a list of keys which begin with the same prefix.
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findPrefix(const std::vector<CharType>& prefix,
        std::vector<std::vector<CharType> >& keyList)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<prefix.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge2(prefix[i], nid, tmp) )
                return false;
            nid = tmp;
        }
        keyList.clear();
        std::vector<CharType> begin = prefix;
        findPrefix_(nid, begin, keyList);
        return keyList.size() ? true : false;
    }

    /**
     * @brief Get a list of values whose keys begin with the same prefix.
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findPrefix(const std::vector<CharType>& prefix,
        std::vector<UserDataType>& valueList)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<prefix.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge2(prefix[i], nid, tmp) )
                return false;
            nid = tmp;
        }
        valueList.clear();
        findPrefix_(nid, valueList);
        return valueList.size() ? true : false;
    }

    /**
     * @brief Get a list of keys which begin with the same prefix.
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findPrefix(const std::vector<CharType>& prefix,
        std::vector<std::vector<CharType> >& keyList,
        std::vector<UserDataType>& valueList)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<prefix.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge2(prefix[i], nid, tmp) )
                return false;
            nid = tmp;
        }
        keyList.clear();
        std::vector<CharType> begin = prefix;
        findPrefix_(nid, begin, keyList, valueList);
        return keyList.size() ? true : false;
    }

    /**
     * @brief Get a list of values whose keys match the wildcard query. Only "*" and
     * "?" are supported currently, legal input looks like "ea?th", "her*", or "*ear?h".
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findRegExp(const std::vector<CharType>& regexp,
        std::vector<UserDataType>& valueList)
    {
        valueList.clear();
        findRegExp_(regexp, 0, NodeIDTraits<NodeIDType>::RootValue, valueList);
        return valueList.size() ? true : false;
    }

    /**
     * @return number of key value pairs inserted.
     */
    unsigned int num_items()
    {
        return dataTable_.numItems();
    }

    /**
     * @brief Display debug information.
     */
    void display()
    {
        edgeTable_.display();
        dataTable_.display();
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
        EdgeTableKeyType etKey(parentNID, ch);

        // if NID is found, just return
        if(edgeTable_.getValue(etKey, childNID))
            return false;

        childNID = nextNID_;
        ++nextNID_;
        if(nextNID_ > NodeIDTraits<NodeIDType>::MaxValue)
            throw std::runtime_error("NID allocation out of range");

        edgeTable_.insertValue(etKey, childNID);
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
    bool getEdge(const CharType& ch, const NodeIDType& parentNID, NodeIDType& childNID)
    {
        EdgeTableKeyType etKey(parentNID, ch);
        NodeIDType value = NodeIDType();

        // Return false if edge doesn't exist
        if(!edgeTable_.getValue(etKey, value))
            return false;

        childNID = value;
        return true;
    }

    /**
     * Given the NID of parent node and the edge's character property,
     * find the child node's NID.
     * The difference with getEdge is, it looks up edge info from OptEdgeTable.
     *
     * @param   ch          edge's character property
     *          parentNID   NID of parent node
     *          childNID    NID of child node
     * @return  true    if successfully
     *          false   if child does not exist yet
     */
    bool getEdge2(const CharType& ch, const NodeIDType& parentNID, NodeIDType& childNID)
    {
        EdgeTableKeyType etKey(parentNID, ch);
        NodeIDType value = NodeIDType();

        // Return false if edge doesn't exist
        if(!optEdgeTable_.getValue(etKey, value))
            return false;

        childNID = value;
        return true;
    }

    /**
     * Store key's nid and userdata into SDB.
     * @return true     successfully
     *         false    key exists already
     */
    bool putData( const NodeIDType& nid, const UserDataType& userData)
    {
        return dataTable_.insertValue(nid,userData);
    }

    /**
     * Update userdata in SDB for a given key, insert if key doesnot exist.
     */
    void updateData( const NodeIDType& nid, const UserDataType& userData)
    {
        dataTable_.update(nid, userData);
    }

    /**
     * Retrieve userdata stored in SDB by given key.
     * @return true     successfully
     *         false    given key does not exist
     */
    bool getData( const NodeIDType& nid, UserDataType& userData)
    {
        return dataTable_.getValue(nid, userData);
    }

    void findPrefix_( const NodeIDType& nid,
        std::vector<CharType>& prefix,
        std::vector<std::vector<CharType> >& keyList)
    {
        UserDataType tmp = UserDataType();
        if(getData(nid, tmp))
            keyList.push_back(prefix);

        EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
        EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

        std::vector<EdgeTableRecordType> result;
        optEdgeTable_.getValueBetween(result, minKey, maxKey);

        for(size_t i = 0; i <result.size(); i++ )
        {
            prefix.push_back(result[i].key.key2);
            findPrefix_(result[i].value, prefix, keyList);
            prefix.pop_back();
        }
    }

    void findPrefix_( const NodeIDType& nid,
        std::vector<UserDataType>& valueList)
    {
        UserDataType tmp = UserDataType();
        if(getData(nid, tmp))
            valueList.push_back(tmp);

        EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
        EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

        std::vector<EdgeTableRecordType> result;
        optEdgeTable_.getValueBetween(result, minKey, maxKey);

        for(size_t i = 0; i <result.size(); i++ )
            findPrefix_(result[i].value, valueList);
    }

    void findPrefix_( const NodeIDType& nid,
        std::vector<CharType>& prefix,
        std::vector<std::vector<CharType> >& keyList,
        std::vector<UserDataType>& valueList)
    {
        UserDataType tmp = UserDataType();
        if(getData(nid, tmp))
        {
            keyList.push_back(prefix);
            valueList.push_back(tmp);
        }

        EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
        EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

        std::vector<EdgeTableRecordType> result;
        optEdgeTable_.getValueBetween(result, minKey, maxKey);

        for(size_t i = 0; i <result.size(); i++ )
        {
            prefix.push_back(result[i].key.key2);
            findPrefix_(result[i].value, prefix, keyList, valueList);
            prefix.pop_back();
        }
    }

    void findRegExp_(const std::vector<CharType>& regexp,
        const size_t& startPos, const NodeIDType& nid,
        std::vector<UserDataType>& valueList)
    {
        if(startPos == regexp.size())
        {
            UserDataType tmp = UserDataType();
            if(getData(nid, tmp))
                valueList.push_back(tmp);
            return;
        }

        switch( regexp[startPos] )
        {
            case 42:    //"*"
            {
                EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
                EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<EdgeTableRecordType> result;
                optEdgeTable_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ )
                    findRegExp_(regexp, startPos, result[i].value, valueList);

                findRegExp_(regexp, startPos+1, nid, valueList);
                break;
            }
            case 63:    //"?"
            {
                EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
                EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<EdgeTableRecordType> result;
                optEdgeTable_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ )
                    findRegExp_(regexp, startPos+1, result[i].value, valueList);
                break;
            }
            default:
            {
                EdgeTableKeyType key(nid, regexp[startPos]);
                NodeIDType nxtNode = NodeIDType();
                if( optEdgeTable_.getValue(key, nxtNode) )
                    findRegExp_(regexp, startPos+1, nxtNode, valueList);
                break;
            }
        }
    }

private:

    bool closed_;

    bool read_;

    std::string dbname_;

    EdgeTableType edgeTable_;

    OptEdgeTableType optEdgeTable_;

    DataTableType dataTable_;

    NodeIDType nextNID_;

};

/**
 * @see SDBTrie
 */
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

//    void config(int degree, int cacheSize, int pageSize)
//    {
//        trie_.config(degree, cacheSize, pageSize);
//    }

    void openForRead(){ trie_.openForRead(); }
    void openForWrite(){ trie_.openForWrite(); }
    void flush(){ trie_.flush(); }
    void close(){ trie_.close(); }
    void optimize(){ trie_.optimize(); }

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

    bool findPrefix(const StringType& prefix,
        std::vector<UserDataType>& valueList)
    {
        CharType* chArray = (CharType*)prefix.c_str();
        size_t chCount = prefix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.findPrefix(chVector, valueList);
    }

    bool findPrefix(const StringType& prefix,
        std::vector<StringType>& keyList)
    {
        CharType* chArray = (CharType*)prefix.c_str();
        size_t chCount = prefix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);

        std::vector< std::vector<CharType> > resultList;
        if( false == trie_.findPrefix(chVector, resultList) )
            return false;

        for(size_t i = 0; i< resultList.size(); i++ )
        {
            StringType tmp(resultList[i].begin(), resultList[i].end() );
            keyList.push_back(tmp);
        }
        return true;
    }

    bool findPrefix(const StringType& prefix,
        std::vector<StringType>& keyList,
        std::vector<UserDataType>& valueList)
    {
        CharType* chArray = (CharType*)prefix.c_str();
        size_t chCount = prefix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);

        std::vector< std::vector<CharType> > resultList;
        if( false == trie_.findPrefix(chVector, resultList, valueList) )
            return false;

        for(size_t i = 0; i< resultList.size(); i++ )
        {
            StringType tmp(resultList[i].begin(), resultList[i].end() );
            keyList.push_back(tmp);
        }
        return true;
    }

    bool findRegExp(const StringType& regexp,
        std::vector<UserDataType>& valueList)
    {
        CharType* chArray = (CharType*)regexp.c_str();
        size_t chCount = regexp.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.findRegExp(chVector, valueList);
    }

    unsigned int num_items() { return trie_.num_items(); }

    void display() { trie_.display(); }


private:
    SDBTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
