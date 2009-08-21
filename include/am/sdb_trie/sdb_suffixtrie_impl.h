#ifndef _SDB_TRIE_IMPL_H_
#define _SDB_TRIE_IMPL_H_

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
    typedef One2ManyMappingTable<NodeIDType, NodeIDType, LockType> SuffixTableType;
    typedef One2OneMappingTable<NodeIDType, NullType, LockType> CompleteWordTableType;

public:

    SDBSuffixTrieImpl(const std::string name)
    :   triename_(name),
        trieImpl_(triename_),
        suffixTable_(triename_)
    {
    }

    virtual ~SDBSuffixTrieImpl(){}

    inline void insert(const std::vector<CharType>& word,
                       const UserDataType userData)
    {
        NodeIDType prevNID;
        for(int i=0, len=word.size();
            i<word.size();
            i++, len--)
        {
            NodeIDType nid;
            trieImpl_.insert(word, i, len, userData, nid);

            if(i == 0) {
                NullType null;
                completeWordTable_.put(nid, null);
            } else
                suffixTable_.put(prevNID, nid);

            prevNID = nid;
        }
    }

    inline void update(const std::vector<CharType>& word,
                       const UserDataType userData)
    {
        NodeIDType prevNID;
        for(int i=0, len=word.size();
            i<word.size();
            i++, len--)
        {
            NodeIDType nid;
            trieImpl_.update(word, i, len, userData, nid);

            if(i == 0) {
                NullType null;
                completeWordTable_.put(nid, null);
            } else
                suffixTable_.put(prevNID, nid);

            prevNID = nid;
        }
    }

    inline bool find(const std::vector<CharType>& word,
                     UserDataType& userData)
    {
        NodeIDType nid;
        UserDataType tmp;
        if(false == trieImpl_.find(word, tmp, nid) )
            return false;
        if(false == completeWordTable_.get(nid))
            return false;
        userData = tmp;
    }

    inline bool iteratePrefix(const std::vector<CharType>& prefix,
                              std::vector<UserDataType>& results)
    {
        std::vector<UserDataType> tmp;
        std::vector<NodeIDType> nidList;
        trieImpl_.iteratePrefix(prefix, tmp, nidList);

        for(size_t i = 0; i<tmp.size(); i++)
        {
            if(completeWordTable_.get(nidList[i]))
            {
                results[i].push_back(tmp[i]);
            }
        }
        if(results[i].size() > 0)
            return true;
        return false;
    }

    inline bool iterateSuffix(const std::vector<CharType>& suffix,
                              std::vector<UserDataType>& results)
    {
        std::vector<UserDataType> tmp;
        std::vector<NodeIDType> nidList;
        trieImpl_.iteratePrefix(suffix, tmp, nidList);

        for(size_t i = 0; i<tmp.size(); i++)
        {
            std::vector<NodeIDType> upperNIDs;
            suffixTable_.get(nidList[i], upperNIDS);

            if(completeWordTable_.get(nidList[i]))
            {
                results[i].push_back(tmp[i]);
            }
        }
    }

    inline bool iterateSubString(const std::vector<CharType>& prefix,
                              std::vector<UserDataType>& results)
    {
    }

private:

    std::string triename_;

    SDBTrieImpl<CharType, UserDataType, NodeIDType, LockType> trieImpl_;

    SuffixTableType suffixTable_;

    CompleteWordTableType completeWordTable_;

};

NS_IZENELIB_AM_END

#endif
