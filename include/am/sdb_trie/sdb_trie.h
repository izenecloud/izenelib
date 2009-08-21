#ifndef _SDB_TRIE_H_
#define _SDB_TRIE_H_

#include <string>
#include <wiselib/ustring/UString.h>

#include "sdb_trie_impl.h"

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrie
{
    /**
     * Maintain all leaf node's information in a Trie data structure with the form
     * of database table.
     * In Trie's graph representation, @see EdgeTable, each data node contains a
     * input string user has inserted into trie.
     * Data Node Table contains <NodeID, UserData> pairs for all data nodes,
     * UserData is the data user inserted into trie together with an input string,
     * the default type is NullType.
     */
    typedef One2OneMappingTable<NodeIDType, UserDataType, LockType> DataNodeTableType;

public:

    SDBTrie(const std::string name)
    :   triename_(name),
        trieImpl_(triename_),
        datanodeTable_(triename_ + ".leafnode.table")
    {
    }

    virtual ~SDBTrie(){}

    inline void insert(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        trieImpl_.insert(word, nid);
        datanodeTable_.put(nid, userData);
    }

    inline void update(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        trieImpl_.insert(word, nid);
        datanodeTable_.update(nid, userData);
    }

    inline bool find(const std::vector<CharType>& word, UserDataType& userData)
    {
        NodeIDType nid = NodeIDTraits<NodeIDType>::RootValue;
        if( !trieImpl_.find(word, nid) )
            return false;
        return datanodeTable_.get(nid, userData);
    }

    inline bool prefixIterate(const std::vector<CharType>& prefix,
        std::vector<UserDataType>& userDataList)
    {
        std::vector<NodeIDType> nidList;
        if( !trieImpl_.prefixIterate(prefix, nidList) )
            return false;

        UserDataType tmp;
        for( size_t i = 0; i < nidList.size(); i++ )
        {
            if(datanodeTable_.get(nidList[i], tmp))
                userDataList.push_back(tmp);
        }
        return true;
    }

	/** Returns the number of words in the trie.
	 *
	 * @return The number of words.
	 */
    inline unsigned int num_items() { return datanodeTable_.num_items(); }

    inline void display() { trieImpl_.display(); }

protected:

private:

    std::string triename_;

    SDBTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

    DataNodeTableType datanodeTable_;

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

    inline void insert(const StringType& word, const UserDataType userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.insert(chVector, userData);
    }

    inline void update(const StringType& word, const UserDataType userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        trie_.update(chVector, userData);
    }

    inline bool find(const StringType& word, UserDataType& userData)
    {
        CharType* chArray = (CharType*)word.c_str();
        size_t chCount = word.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.find(chVector, userData);
    }

    inline bool prefixIterate(const StringType& prefix,
        std::vector<UserDataType>& userDataList)
    {
        CharType* chArray = (CharType*)prefix.c_str();
        size_t chCount = prefix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.prefixIterate(chVector, userDataList);
    }

    inline unsigned int num_items() { return trie_.num_items(); }

    inline void display() { trie_.display(); }


private:
    SDBTrieType trie_;
};

NS_IZENELIB_AM_END

#endif
