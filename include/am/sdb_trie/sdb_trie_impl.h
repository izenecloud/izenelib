#ifndef _SDB_TRIE_IMPL_H_
#define _SDB_TRIE_IMPL_H_

#include <am/concept/DataType.h>

#include "edge_table.h"
#include "one2one_mapping_table.h"
#include "traits.h"

NS_IZENELIB_AM_BEGIN

template <typename CharType,
          typename UserDataType = NullType,
          typename NodeIDType = uint64_t,
          typename LockType = izenelib::util::NullLock>
class SDBTrieImpl
{

    typedef EdgeTable<CharType, NodeIDType, LockType> EdgeTableType;

    typedef One2OneMappingTable<NodeIDType, NullType> NodeTypeTableType;

public:

    SDBTrieImpl(const std::string name)
    :   triename_(name),
        edgeTable_(triename_ + ".edge.table"),
        nodetypeTable_(triename_ + ".nodetype.table")
    {
    }

    virtual ~SDBTrieImpl(){}

    inline void insert(const std::vector<CharType>& word,
                       NodeIDType& nid)
    {
        insert(word, 0, word.size(), nid);
    }

    inline void insert(const std::vector<CharType>& word,
                       const size_t startPos, const size_t len,
                       NodeIDType& nid)
    {
        if(len == 0) return;

        const size_t endPos = startPos + len -1;
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=startPos; i< endPos; i++ )
        {
            NodeIDType childNID;
            edgeTable_.put(word[i], parentNID, childNID);
            parentNID = childNID;
        }
        edgeTable_.put(word[endPos], parentNID, nid);
        NullType null;
        nodetypeTable_.put(nid, null);
    }

    inline bool find(const std::vector<CharType>& word,
                     NodeIDType& nid)
    {
        return find(word, 0, word.size(), nid);
    }


    inline bool find(const std::vector<CharType>& word,
                     const size_t startPos, const size_t len,
                     NodeIDType& nid)
    {
        if(len == 0) return false;

        const size_t endPos = startPos + len -1;
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=startPos; i<=endPos; i++ )
        {
            NodeIDType childNID;
            if( !edgeTable_.get(word[i], parentNID, childNID) )
                return false;
            parentNID = childNID;
        }
        nid = parentNID;

        NullType null;
        if(nodetypeTable_.get(nid, null))
            return true;

        return false;
    }

    inline bool prefixIterate(const std::vector<CharType>& prefix,
        std::vector<NodeIDType>& nidList)
    {
        return prefixIterate(prefix, 0, prefix.size(), nidList);
    }


    inline bool prefixIterate(const std::vector<CharType>& prefix,
        const size_t startPos, const size_t len,
        std::vector<NodeIDType>& nidList)
    {
        NodeIDType parentNID = NodeIDTraits<NodeIDType>::RootValue;
        for( size_t i=startPos; i<startPos + len; i++ )
        {
            NodeIDType childNID;
            if( !edgeTable_.get(prefix[i], parentNID, childNID) )
                return false;
            parentNID = childNID;
        }

        std::vector<NodeIDType> tmp;
        tmp.push_back(parentNID);
        edgeTable_.iterate(parentNID, tmp);

        NullType null;
        for(size_t i=0; i<tmp.size(); i++)
        {
            if(nodetypeTable_.get(tmp[i], null))
                nidList.push_back(tmp[i]);
        }

        return true;
    }

    inline void display()
    {
        edgeTable_.display();
    }

protected:

private:

    std::string triename_;

    EdgeTableType edgeTable_;

    NodeTypeTableType nodetypeTable_;

};

NS_IZENELIB_AM_END

#endif
