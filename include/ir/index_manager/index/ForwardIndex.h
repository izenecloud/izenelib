/**
* @file        ForwardIndex.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief ForwardIndex is nothing more than a hashmap whose key represents
* the term id, while the value is the term offset of that term.
* Another issue is, ForwardIndex is a forwardindex of one property instead of
* a total document
*/

#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

#include <types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <deque>
#include <vector>
using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef std::deque<unsigned int> ForwardIndexOffset;

class ForwardIndex : public rde::hash_map<unsigned int, ForwardIndexOffset* >
{
public:
    ForwardIndex(){}

    ~ForwardIndex()
    {
        for(ForwardIndex::iterator iter = this->begin(); iter != this->end(); ++iter)
            delete iter->second;
    }

    ///This interface is just to keep compatibility with other old existing codes
    ///One should not use it any longer because it will lead to extra data replication
    bool getTermIdList(std::vector<unsigned int>& termIdList) const
    {
        for(ForwardIndex::const_iterator iter = begin(); iter != end(); ++iter)
            termIdList.push_back(iter->first);
        return true;
    }

    ///This interface is just to keep compatibility with other old existing codes
    ///One should not use it any longer because it will lead to extra data replication
    bool getTermOffsetListByTermId(unsigned int termId, 
				std::vector<unsigned int >& termOffsetList) const
    {
        ForwardIndex::const_iterator iter = find(termId);
        if(iter == end())
            return false;
        termOffsetList.reserve(iter->second->size());    
        for(ForwardIndexOffset::iterator it = iter->second->begin(); it != iter->second->end(); ++it)
            termOffsetList.push_back(*it);
        return true;
    }

    ///Insert term offset to forwardindex
    bool insertTermOffset(unsigned int termId, unsigned int termOffset)
    {
        ForwardIndex::iterator it = find(termId);
        if(it != end())
        {
            (*it).second->push_back(termOffset);
        }
        else
        {
            ForwardIndexOffset* pForwardIndexOffset = new ForwardIndexOffset;
            pForwardIndexOffset->push_back(termOffset);
            (*this)[termId] = pForwardIndexOffset;
        }
        return true;
    }

public:
    ///represents the property length of a document
    unsigned int docLength_;
};

}

NS_IZENELIB_IR_END

#endif
