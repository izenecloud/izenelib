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
    typedef One2OneMappingTable<NodeIDType, NullType, LockType> IsWordTableType;

    typedef One2ManyMappingTable<NodeIDType, NodeIDType, LockType> Suffix2WordTableType;

public:

    SDBSuffixTrieImpl(const std::string name)
    :   triename_(name),
        trieImpl_(triename_),
        iswordTable_(triename_ + ".isword2.table"),
        suffix2wordTable_(triename_ + ".suffix2word.table")
    {
    }

    virtual ~SDBSuffixTrieImpl(){}

    inline void insert(const std::vector<CharType>& word,
                       NodeIDType& nid)
    {
        NodeIDType rootNID = NodeIDTraits<NodeIDType>::RootValue;
        for(size_t i=0, len=word.size();
            i<word.size();
            i++, len--)
        {
            NodeIDType currNID = NodeIDTraits<NodeIDType>::RootValue;
            trieImpl_.insert(word, i, len, currNID);

            if(i == 0) {
                nid = rootNID = currNID;
                iswordTable_.put(rootNID);
            }
            suffix2wordTable_.put(currNID, rootNID);
        }
    }

    inline bool find(const std::vector<CharType>& word,
                     NodeIDType& nid)
    {
        NodeIDType suffixNID;
        if(!trieImpl_.find(word, suffixNID))
            return false;
        if(!iswordTable_.get(suffixNID))
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

        size_t t1 = nidList.size();
        for(size_t i =0; i<suffixNIDs.size(); i++)
        {
            if(iswordTable_.get(suffixNIDs[i]))
                nidList.push_back(suffixNIDs[i]);
        }
        size_t t2 = nidList.size();
        if(t2>t1) return true;
        return false;
    }

    inline bool suffixIterate(const std::vector<CharType>& suffix,
                              std::vector<NodeIDType>& nidList)
    {
        if(suffix.size() == 0)
            return prefixIterate(suffix, nidList);

        NodeIDType nid;
        if(!trieImpl_.find(suffix, nid))
            return false;

        size_t t1 = nidList.size();
        suffix2wordTable_.get(nid, nidList);
        size_t t2 = nidList.size();
        if(t2>t1) return true;
        return false;
    }

    inline bool substringIterate(const std::vector<CharType>& substring,
                              std::vector<NodeIDType>& nidList)
    {
        std::vector<NodeIDType> suffixNIDs;
        if(!trieImpl_.prefixIterate(substring, suffixNIDs))
            return false;

        std::vector<NodeIDType> wordNIDs;
        size_t t1 = nidList.size();
        for(size_t i = 0; i<suffixNIDs.size(); i++)
        {
            wordNIDs.clear();
            suffix2wordTable_.get(suffixNIDs[i], wordNIDs);

            for(size_t j = 0; j< wordNIDs.size(); j++)
            {
               if(std::find(nidList.begin(),nidList.end(),wordNIDs[j]) == nidList.end())
                    nidList.push_back(wordNIDs[j]);
            }
        }
        size_t t2 = nidList.size();
        if(t2>t1) return true;
        return false;
    }

private:

    std::string triename_;

    SDBTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

    IsWordTableType iswordTable_;

    Suffix2WordTableType suffix2wordTable_;

};

NS_IZENELIB_AM_END

#endif
