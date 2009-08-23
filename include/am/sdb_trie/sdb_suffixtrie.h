#ifndef _SDB_SUFFIX_TRIE_H_
#define _SDB_SUFFIX_TRIE_H_

#include "sdb_suffixtrie_impl.h"
#include "one2one_mapping_table.h"

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBSuffixTrie
{
    typedef One2OneMappingTable<NodeIDType, UserDataType, LockType> DataNodeTableType;

public:

    SDBSuffixTrie(const std::string name)
    :   triename_(name),
        trieImpl_(triename_),
        datanodeTable_(triename_ + ".datanode.table")
    {
    }

    virtual ~SDBSuffixTrie(){}

    inline void insert(const std::vector<CharType>& word,
                       const UserDataType userData)
    {
        NodeIDType nid;
        trieImpl_.insert(word, nid);
        datanodeTable_.put(nid, userData);
    }

    inline void update(const std::vector<CharType>& word,
                       const UserDataType userData)
    {
        NodeIDType nid;
        trieImpl_.insert(word, nid);
        datanodeTable_.update(nid, userData);
    }

    inline bool find(const std::vector<CharType>& word,
                     UserDataType& userData)
    {
        NodeIDType nid;
        if( !trieImpl_.find(word, nid) )
            return false;
        return datanodeTable_.get(nid, userData);
    }

    inline bool prefixIterate(const std::vector<CharType>& prefix,
                              std::vector<UserDataType>& results)
    {
        std::vector<NodeIDType> nidList;
        if(!trieImpl_.prefixIterate(prefix, nidList) )
            return false;
        UserDataType tmp;
        for(size_t i = 0; i<nidList.size(); i++)
        {
            if(datanodeTable_.get(nidList[i],tmp))
                results.push_back(tmp);
        }
        return true;
    }

    inline bool suffixIterate(const std::vector<CharType>& suffix,
                              std::vector<UserDataType>& results)
    {
        std::vector<NodeIDType> nidList;
        if(!trieImpl_.suffixIterate(suffix, nidList) )
            return false;
        UserDataType tmp;
        for(size_t i = 0; i< nidList.size(); i++)
        {
            if(datanodeTable_.get(nidList[i], tmp))
                results.push_back(tmp);
        }
        return true;
    }

    inline bool substringIterate(const std::vector<CharType>& substring,
                              std::vector<UserDataType>& results)
    {
        std::vector<NodeIDType> nidList;
        if(!trieImpl_.substringIterate(substring, nidList) )
            return false;
        UserDataType tmp;
        for(size_t i = 0; i< nidList.size(); i++)
        {
            if(datanodeTable_.get(nidList[i], tmp))
                results.push_back(tmp);
        }
        return true;
    }

    inline unsigned int num_items() { return datanodeTable_.num_items(); }

    inline void display() { trieImpl_.display(); }

private:

    std::string triename_;

    SDBSuffixTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

    DataNodeTableType datanodeTable_;

};


template <typename StringType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBSuffixTrie2
{
    typedef typename StringType::value_type CharType;
    typedef SDBSuffixTrie<CharType, UserDataType, NodeIDType, LockType> SDBSuffixTrieType;

public:

    SDBSuffixTrie2(const std::string name)
    :   trie_(name) {}

    virtual ~SDBSuffixTrie2(){}

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

    inline bool suffixIterate(const StringType& suffix,
        std::vector<UserDataType>& userDataList)
    {
        CharType* chArray = (CharType*)suffix.c_str();
        size_t chCount = suffix.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.suffixIterate(chVector, userDataList);
    }

    inline bool substringIterate(const StringType& substring,
        std::vector<UserDataType>& userDataList)
    {
        CharType* chArray = (CharType*)substring.c_str();
        size_t chCount = substring.length();
        std::vector<CharType> chVector(chArray, chArray+chCount);
        return trie_.substringIterate(chVector, userDataList);
    }

    inline unsigned int num_items() { return trie_.num_items(); }

    inline void display() { trie_.display(); }


private:
    SDBSuffixTrieType trie_;
};


NS_IZENELIB_AM_END

#endif
