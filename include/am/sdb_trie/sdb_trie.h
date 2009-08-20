#ifndef _SDB_TRIE_H_
#define _SDB_TRIE_H_

#include <string>
#include <wiselib/ustring/UString.h>

#include "edge_table.h"
#include "leafnode_table.h"
#include "traits.h"

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie
{
    typedef LeafNodeTable<NodeIDType, UserDataType, LockType> LeafNodeTableType;
    typedef EdgeTable<CharType, NodeIDType, LockType> EdgeTableType;

public:

    SDBTrie(const std::string name)
    :   triename_(name),
        edgeTable_(triename_ + ".edge.table"),
        leafnodeTable_(triename_ + ".leafnode.table")
    {
    }

    virtual ~SDBTrie(){}

    void insert(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<word.size()-1; i++ )
        {
            NodeIDType childNID;
            edgeTable_.put(word[i], parentNID, childNID);
            parentNID = childNID;
        }
        NodeIDType leafNID;
        if( edgeTable_.put(word[word.size()-1], parentNID, leafNID) )
        {
            if( false == leafnodeTable_.put(leafNID, userData) )
                throw std::runtime_error("SDBTrie insert err: incoherence between \
                    EdgeTable and LeafNodeTable");
        }
    }

    void update(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<word.size()-1; i++ )
        {
            NodeIDType childNID;
            edgeTable_.put(word[i], parentNID, childNID);
            parentNID = childNID;
        }
        NodeIDType leafNID;
        if( edgeTable_.put(word[word.size()-1], parentNID, leafNID) )
        {
            if( false == leafnodeTable_.put(leafNID, userData) )
                throw std::runtime_error("SDBTrie update err: incoherence between \
                    EdgeTable and LeafNodeTable");
        }
        else
        {
            leafnodeTable_.update(leafNID, userData);
        }
    }

    bool find(const std::vector<CharType>& word, UserDataType& userData)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<word.size(); i++ )
        {
            NodeIDType childNID;
            if( false == edgeTable_.get(word[i], parentNID, childNID) )
                return false;
            parentNID = childNID;
        }
        if( false == leafnodeTable_.get(parentNID, userData) )
            return false;
        return true;
    }

    /**
     *
     */
    bool prefixIterate(const std::vector<CharType>& prefix,
        std::vector<UserDataType>& userDataList)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<prefix.size(); i++ )
        {
            NodeIDType childNID;
            if( false == edgeTable_.get(prefix[i], parentNID, childNID) )
                return false;
            parentNID = childNID;
        }

        UserDataType userdata;
        if(leafnodeTable_.get(parentNID, userdata))
            userDataList.push_back(userdata);

        std::vector<NodeIDType> childNIDList;
        edgeTable_.iterate(parentNID, childNIDList) ;
        for( size_t i = 0; i < childNIDList.size(); i++ )
        {
            if(leafnodeTable_.get(childNIDList[i], userdata))
                userDataList.push_back(userdata);
        }

        return true;
    }

	/** Returns the number of words in the trie.
	 *
	 * @return The number of words.
	 */
    unsigned int num_items()
    {
        return leafnodeTable_.num_items();
    }

    void display()
    {
        edgeTable_.display();
    }

protected:

private:

    std::string triename_;

    EdgeTableType edgeTable_;

    LeafNodeTableType leafnodeTable_;

};

template <typename StringType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie2
{
    typedef typename StringType::value_type CharType;
    typedef SDBTrie<CharType, UserDataType, NodeIDType, LockType> SDBTrieType;

public:

    SDBTrie2(const std::string name)
    :   trie_(name) {}

    virtual ~SDBTrie2(){}

    void insert(const StringType& word, const UserDataType userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.insert(chVector, userData);
    }

    void update(const StringType& word, const UserDataType userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.update(chVector, userData);
    }

    bool find(const StringType& word, UserDataType& userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.find(chVector, userData);
    }

    bool prefixIterate(const StringType& prefix,
        std::vector<UserDataType>& userDataList)
    {
        CharType* chArray = (CharType*)prefix.c_str();
        size_t chCount = prefix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.prefixIterate(chVector, userDataList);
    }

    unsigned int num_items() { return trie_.num_items(); }

    void display() { trie_.display(); }


private:
    SDBTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
