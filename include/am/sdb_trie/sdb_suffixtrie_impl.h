#ifndef _SDB_SUFFIX_TRIE_IMPL_H_
#define _SDB_SUFFIX_TRIE_IMPL_H_

#include "sdb_trie_impl.h"
#include "one2one_mapping_table.h"
#include "one2many_mapping_table.h"


NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBSuffixTrieImpl
{
    typedef One2OneMappingTable<NodeIDType, NullType, LockType> WordTableType;

    typedef One2ManyMappingTable<NodeIDType, NodeIDType, LockType> Suffix2WordTableType;

public:

    SDBSuffixTrieImpl(const std::string name)
    :   triename_(name),
        trieImpl_(triename_),
        wordTable_(triename_ + ".word.table"),
        suffix2wordTable_(triename_ + "suffix2word.table")
    {
    }

    virtual ~SDBSuffixTrieImpl(){}

    inline void insert(const std::vector<CharType>& word,
                       NodeIDType& nid)
    {
        NodeIDType prevNID;
        NodeIDType rootNID;
        for(int i=0, len=word.size();
            i<word.size();
            i++, len--)
        {
            NodeIDType currNID;
            trieImpl_.insert(word, i, len, currNID);

            if(i == 0) {
                nid = rootNID = currNID;
                NullType null;
                wordTable_.put(rootNID, null);
            } else {
                suffix2wordTable_.put(currNID, rootNID);
            }
            prevNID = currNID;
        }
    }

    inline bool find(const std::vector<CharType>& word,
                     NodeIDType& nid)
    {
        NodeIDType suffixNID;
        if(!trieImpl_.find(word, suffixNID))
            return false;
        NullType null;
        if(!wordTable_.get(suffixNID, null))
            return false;
        nid = suffixNID;
        return true;
    }

    inline bool prefixIterate(const std::vector<CharType>& prefix,
                              std::vector<NodeIDType>& nidList)
    {
        std::vector<NodeIDType> suffixNIDs;
        if( !trieImpl_.prefixIterate(prefix, suffixNIDs) )
            return false;

        for(size_t i =0; i<suffixNIDs.size(); i++)
        {
            NullType null;
            if(wordTable_.get(suffixNIDs[i], null))
                nidList.push_back(suffixNIDs[i]);
        }
        return true;
    }

    inline bool suffixIterate(const std::vector<CharType>& suffix,
                              std::vector<NodeIDType>& nidList)
    {
        NodeIDType nid;
        if(!trieImpl_.find(suffix, nid))
            return false;

        suffix2wordTable_.get(nid, nidList);
        return true;
    }

    inline bool substringIterate(const std::vector<CharType>& substring,
                              std::vector<NodeIDType>& nidList)
    {
        std::vector<NodeIDType> suffixNIDs;
        trieImpl_.prefixIterate(substring, suffixNIDs);

        UserDataType tmp;
        std::vector<NodeIDType> wordNIDs;
        for(size_t i = 0; i<suffixNIDs.size(); i++)
        {
            wordNIDs.clear();
            suffix2wordTable_.get(suffixNIDs[i], wordNIDs);

            for(size_t j = 0; j< wordNIDs.size(); j++)
            {
                if(std::find(nidList.begin(),nidList.end(),wordNIDs[i]) == nidList.end())
                    nidList.push_back(wordNIDs[i]);
            }
        }
    }

private:

    std::string triename_;

    SDBTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

    WordTableType wordTable_;

    Suffix2WordTableType suffix2wordTable_;

};

NS_IZENELIB_AM_END

#endif
