#ifndef _SDB_TRIE_IMPL_H_
#define _SDB_TRIE_IMPL_H_

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
class SDBTrieImpl
{
    typedef LeafNodeTable<NodeIDType, UserDataType, LockType> LeafNodeTableType;
    typedef EdgeTable<CharType, NodeIDType, LockType> EdgeTableType;

public:

    SDBTrieImpl(const std::string name)
    :   triename_(name),
        edgeTable_(triename_ + ".edge.table"),
        leafnodeTable_(triename_ + ".leafnode.table")
    {
    }

    virtual ~SDBTrieImpl(){}

    inline void insert(const std::vector<CharType>& word,
                       const UserDataType userData,
                       NodeIDType& nid)
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
        nid = leafNID;
    }

    inline void update(const std::vector<CharType>& word,
                       const UserDataType userData,
                       NodeIDType& nid)
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
        nid = leafNID;
    }

    inline bool find(const std::vector<CharType>& word,
                     UserDataType& userData,
                     NodeIDType& nid)
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
        nid = parentNID;
        return true;
    }

    /**
     *
     */
    inline bool prefixIterate(const std::vector<CharType>& prefix,
        std::vector<UserDataType>& userDataList,
        std::vector<NodeIDType>& nidList)
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
        {
            userDataList.push_back(userdata);
            nidList.push_back(parentNID);
        }

        std::vector<NodeIDType> childNIDList;
        edgeTable_.iterate(parentNID, childNIDList) ;
        for( size_t i = 0; i < childNIDList.size(); i++ )
        {
            if(leafnodeTable_.get(childNIDList[i], userdata))
            {
                userDataList.push_back(userdata);
                nidList.push_back(childNIDList[i]);
            }
        }

        return true;
    }

	/** Returns the number of words in the trie.
	 *
	 * @return The number of words.
	 */
    inline unsigned int num_items()
    {
        return leafnodeTable_.num_items();
    }

    inline void display()
    {
        edgeTable_.display();
    }

protected:

private:

    std::string triename_;

    EdgeTableType edgeTable_;

    LeafNodeTableType leafnodeTable_;

};

NS_IZENELIB_AM_END

#endif
