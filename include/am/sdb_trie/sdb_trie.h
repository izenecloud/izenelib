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

public:

    SDBTrie(const std::string name)
    :   triename_(name),
        trieImpl_(triename_)
    {
    }

    virtual ~SDBTrie(){}

    inline void insert(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType nid;
        trieImpl_.insert(word, userData, nid);
    }

    inline void update(const std::vector<CharType>& word, const UserDataType userData)
    {
        NodeIDType nid;
        trieImpl_.update(word, userData, nid);
    }

    inline bool find(const std::vector<CharType>& word, UserDataType& userData)
    {
        NodeIDType nid;
        return trieImpl_.find(word, userData, nid);
    }

    inline bool prefixIterate(const std::vector<CharType>& prefix,
        std::vector<UserDataType>& userDataList)
    {
        std::vector<NodeIDType> nidList;
        return trieImpl_.prefixIterate(prefix, userDataList, nidList);
    }

	/** Returns the number of words in the trie.
	 *
	 * @return The number of words.
	 */
    inline unsigned int num_items() { return trieImpl_.num_items(); }

    inline void display() { trieImpl_.display(); }

protected:

private:

    std::string triename_;

    SDBTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

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
