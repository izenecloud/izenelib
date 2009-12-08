#ifndef _PARTITION_TRIE_H_
#define _PARTITION_TRIE_H_

#include <iostream>

#include <am/concept/DataType.h>
#include <hdb/HugeDB.h>
#include <sdb/SequentialDB.h>
#include "traits.h"

NS_IZENELIB_AM_BEGIN

template<typename NodeIDType>
class PartitionTrieHeader {

public:

    NodeIDType nextNodeID;

    PartitionTrieHeader(const std::string& path)
        : path_(path)
	{
	    ifstream ifs(path_.c_str());
        if( !ifs ) {
            nextNodeID = NodeIDTraits<NodeIDType>::MinValue;
            return;
        }

        ifs.seekg(0, ifstream::end);
        if( 0 == ifs.tellg()) {
            nextNodeID = NodeIDTraits<NodeIDType>::MinValue;
        } else {
            ifs.seekg(0, fstream::beg);
            boost::archive::xml_iarchive xml(ifs);
            xml >> boost::serialization::make_nvp("NextNodeID", nextNodeID);
        }
        ifs.close();
    }

    ~PartitionTrieHeader()
    {
        flush();
    }

    void flush()
    {
        ofstream ofs(path_.c_str());
        boost::archive::xml_oarchive xml(ofs);
        xml << boost::serialization::make_nvp("NextNodeID", nextNodeID);
        ofs.flush();
    }

private:

    std::string path_;
};

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
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class PartitionTrie
{
    // hdb is fast for large number of random insertions
    typedef std::pair<NodeIDType, CharType> EdgeTableKeyType;
    typedef NodeIDType EdgeTableValueType;
    typedef DataType<EdgeTableKeyType, EdgeTableValueType> EdgeTableRecordType;
    typedef izenelib::hdb::ordered_hdb_fixed<EdgeTableKeyType, EdgeTableValueType , LockType> EdgeTableType;

    // sdb is fast for sequential insertions
    typedef NodeIDType DataTableKeyType;
    typedef bool DataTableValueType;
    typedef DataType<DataTableKeyType, DataTableValueType> DataTableRecordType;
    typedef izenelib::hdb::ordered_hdb_fixed<DataTableKeyType, DataTableValueType , LockType> DataTableType;

    typedef PartitionTrie<CharType, NodeIDType, LockType> ThisType;

public:

    PartitionTrie(const std::string& dbname)
    :   closed_(true), dbname_(dbname),
        partitionTrieHeader_(dbname_ + ".header.xml"),
        edgeTable_(dbname_ + ".edgetable"),
        dataTable_(dbname_ + ".datatable.sdb")
    {
        nextNID_ = partitionTrieHeader_.nextNodeID;
    }

    virtual ~PartitionTrie()
    {
        flush();
    }

    /**
     * @brief Open all db files, maps them to memory. Call it
     *        after config() and before any insert() or find() operations.
     */
    void open()
    {
        dataTable_.setDegree(32);
        dataTable_.setPageSize(1024);
        dataTable_.setCachedRecordsNumber(2500000);
        dataTable_.setMergeFactor(2);
        dataTable_.open();

        edgeTable_.setDegree(128);
        edgeTable_.setPageSize(8192);
        edgeTable_.setCachedRecordsNumber(2500000);
        edgeTable_.setMergeFactor(2);
        edgeTable_.open();
    }

    void flush()
    {
        partitionTrieHeader_.nextNodeID = nextNID_;
        partitionTrieHeader_.flush();
        edgeTable_.flush();
        dataTable_.flush();
    }


    /**
     * @brief Write back informations back to db.
     */
    void close()
    {
        edgeTable_.close();
        dataTable_.close();
    }

    /**
     * @brief Optimize read performance, e.g. get() and findPrefix()
     */
    void optimize()
    {
        dataTable_.optimize();
        edgeTable_.optimize();
    }

    /**
     * @brief Insert key value pairs into Trie.
     */
    void insert(const std::vector<CharType>& key)
    {
        if( key.size() == 0) return;

        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i< key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            addEdge(key[i], nid, tmp);
            nid = tmp;
        }
        setLeaf(nid);
    }

    /**
     * @brief Get value for a given key.
     * @return false if key doesn't exist
     *         true otherwise
     */
    bool get(const std::vector<CharType>& key)
    {
        if(key.size() == 0) return false;

        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<key.size(); i++ )
        {
            NodeIDType tmp = NodeIDType();
            if( !getEdge(key[i], nid, tmp) )
                return false;
            nid = tmp;
        }
        return isLeaf(nid);
    }

    /**
     * @brief Get a list of values whose keys match the wildcard query. Only "*" and
     * "?" are supported currently, legal input looks like "ea?th", "her*", or "*ear?h".
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findRegExp(const std::vector<CharType>& regexp,
        std::vector<std::vector<CharType> >& keyList,
        int maximumResultNumber)
    {
        keyList.clear();
        std::vector<CharType> prefix;
        findRegExp_(regexp, 0, NodeIDTraits<NodeIDType>::RootValue,
            prefix, keyList, maximumResultNumber);
        return keyList.size() ? true : false;
    }

    void concatenate(ThisType& other, NodeIDType startNID) {
        EdgeTableRecordType edge;
        typename EdgeTableType::HDBCursor ecur = other.edgeTable_.get_first_Locn();
        while(other.edgeTable_.get(ecur,edge)) {
            if(edge.value >=startNID)
                edgeTable_.insertValue(edge.key, edge.value);
            other.edgeTable_.seq(ecur);
        }

        DataTableRecordType data;
        typename DataTableType::HDBCursor dcur = other.dataTable_.get_first_Locn();
        while(other.dataTable_.get(dcur,data)) {
            dataTable_.insertValue(data.key, data.value);
            other.dataTable_.seq(dcur);
        }

        nextNID_ = (nextNID_ > other.nextNID_) ? nextNID_ : other.nextNID_;
    }

    /**
     * @brief set base NodeID.
     */
    void setbase(const NodeIDType base)
    {
        //std::cout << "nextNID_ changes " << nextNID_;
        nextNID_ += base;
        //std::cout << " ==> " << nextNID_ << std::endl;
    }

    NodeIDType nextNodeID() {
        return nextNID_;
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
     * Store key's nid and userdata into SDB.
     * @return true     successfully
     *         false    key exists already
     */
    void setLeaf( const NodeIDType nid)
    {
        dataTable_.insertValue(nid,true);
    }

    /**
     * Retrieve userdata stored in SDB by given key.
     * @return true     successfully
     *         false    given key does not exist
     */
    bool isLeaf( const NodeIDType nid)
    {
        bool flag = false;
        return dataTable_.getValue(nid, flag);
    }

    void findRegExp_(const std::vector<CharType>& regexp,
        const size_t& startPos, const NodeIDType& nid,
        std::vector<CharType>& prefix,
        std::vector<std::vector<CharType> >& keyList,
        int maximumResultNumber)
    {
        if(keyList.size() >= (size_t)maximumResultNumber)
            return;

        if(startPos == regexp.size())
        {
            if(isLeaf(nid))
                keyList.push_back(prefix);
            return;
        }

        // std::cout << regexp[startPos] << "," << nid << std::endl;

        switch( regexp[startPos] )
        {
            case 42:    //"*"
            {
                EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
                EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<EdgeTableRecordType> result;
                edgeTable_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ ) {
                    prefix.push_back(result[i].key.second);
                    findRegExp_(regexp, startPos, result[i].value, prefix,
                        keyList, maximumResultNumber);
                    prefix.pop_back();
                }

                findRegExp_(regexp, startPos+1, nid, prefix,
                    keyList, maximumResultNumber);
                break;
            }
            case 63:    //"?"
            {
                EdgeTableKeyType minKey(nid, NumericTraits<CharType>::MinValue);
                EdgeTableKeyType maxKey(nid, NumericTraits<CharType>::MaxValue);

                std::vector<EdgeTableRecordType> result;
                edgeTable_.getValueBetween(result, minKey, maxKey);

                for(size_t i = 0; i <result.size(); i++ ) {
                    prefix.push_back(result[i].key.second);
                    findRegExp_(regexp, startPos+1, result[i].value, prefix,
                        keyList, maximumResultNumber);
                    prefix.pop_back();
                }
                break;
            }
            default:
            {
                EdgeTableKeyType key(nid, regexp[startPos]);
                NodeIDType nxtNode = NodeIDType();
                if( edgeTable_.getValue(key, nxtNode) ) {
                    prefix.push_back(regexp[startPos]);
                    findRegExp_(regexp, startPos+1, nxtNode, prefix,
                        keyList, maximumResultNumber);
                    prefix.pop_back();
                }
                break;
            }
        }
    }

private:

    bool closed_;

    std::string dbname_;

    PartitionTrieHeader<NodeIDType> partitionTrieHeader_;

    EdgeTableType edgeTable_;

    DataTableType dataTable_;

    NodeIDType nextNID_;

};

/**
 * @see SDBTrie
 */
template <typename StringType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class PartitionTrie2
{
    typedef typename StringType::value_type CharType;
    typedef PartitionTrie2<StringType, NodeIDType, LockType> ThisType;
    typedef PartitionTrie<CharType, NodeIDType, LockType> PartitionTrieType;

public:

    PartitionTrie2(const std::string name)
    :   trie_(name) {}

    virtual ~PartitionTrie2(){}

    void open(){ trie_.open(); }
    void flush(){ trie_.flush(); }
    void close(){ trie_.close(); }
    void optimize(){ trie_.optimize(); }

    void insert(const StringType& key)
    {
        CharType* chArray = (CharType*)key.c_str();
        size_t chCount = key.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.insert(chVector);
    }

    bool get(const StringType& key)
    {
        CharType* chArray = (CharType*)key.c_str();
        size_t chCount = key.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.get(chVector);
    }

    bool findRegExp(const StringType& regexp,
        std::vector<StringType>& keyList,
        int maximumResultNumber)
    {
        CharType* chArray = (CharType*)regexp.c_str();
        size_t chCount = regexp.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);

        std::vector< std::vector<CharType> > resultList;
        if( false == trie_.findRegExp(chVector, resultList, maximumResultNumber) )
            return false;

        for(size_t i = 0; i< resultList.size(); i++ )
        {
            StringType tmp(resultList[i].begin(), resultList[i].end() );
            keyList.push_back(tmp);
        }
        return true;
    }

    void setbase(const NodeIDType base) {
        trie_.setbase(base);
    }

    void concatenate(ThisType& other, NodeIDType start) {
        trie_.concatenate(other.trie_, start);
    }

    NodeIDType nextNodeID() {
        return trie_.nextNodeID();
    }

    unsigned int num_items() { return trie_.num_items(); }

    void display() { trie_.display(); }

private:
    PartitionTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
