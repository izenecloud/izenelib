#ifndef _SDB_TRIE_H_
#define _SDB_TRIE_H_

#include<string>
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
        leafnodeTable_(triename_+"_leafnodetable"),
        edgeTable_(triename_ + "_edgetable"),
        numWords_(0)
    {
    }

    virtual ~SDBTrie(){}

    void insert(const std::vector<CharType>& word, const UserDataType id)
    {
        bool add = false;
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
            add = true;
            if( false == leafnodeTable_.put(leafNID, id) )
                throw std::runtime_error("SDBTrie insertion err: incoherence between \
                    EdgeTable and LeafNodeTable");
        }

        if(add)
            numWords_ ++;
    }

    bool find(const std::vector<CharType>& word, UserDataType& id)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=0; i<word.size(); i++ )
        {
            NodeIDType childNID;
            if( false == edgeTable_.get(word[i], parentNID, childNID) )
                return false;
            parentNID = childNID;
        }
        if( false == leafnodeTable_.get(parentNID, id) )
            return false;
        return true;
    }

    bool findRegExp(const std::vector<CharType> & exp,
        std::vector<UserDataType> & results)
    {
        return false;
    }

	/** Returns the number of words in the trie.
	 *
	 * @return The number of words.
	 */
    unsigned int num_items();

private:

    std::string triename_;

    LeafNodeTableType leafnodeTable_;

    EdgeTableType edgeTable_;

    unsigned int numWords_;
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

    void insert(const StringType& word, const UserDataType id)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.insert(chVector, id);
    }

    bool find(const StringType& word, UserDataType& id)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.find(chVector, id);
    }

    bool findRegExp(const StringType& exp, std::vector<UserDataType> & results)
    {
        CharType* chArray = (CharType*)exp.c_str();
        size_t chCount = exp.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.findRegExp(chVector, results);
    }

    unsigned int num_items() { return trie_.num_items(); }

private:
    SDBTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
