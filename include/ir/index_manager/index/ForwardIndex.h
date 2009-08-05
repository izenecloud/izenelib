#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

#include <types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <deque>
#include <vector>
using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef unsigned int WordOffset;
typedef unsigned int CharOffset;
typedef std::deque<std::pair<unsigned int, unsigned int> > ForwardIndexOffset;

class ForwardIndex : public rde::hash_map<unsigned int, ForwardIndexOffset* >
{
public:
    ForwardIndex(){}

    ~ForwardIndex()
    {
        for(ForwardIndex::iterator iter = this->begin(); iter != this->end(); ++iter)
            delete iter->second;
    }

    bool getTermIdList(std::vector<unsigned int>& termIdList) const
    {
        for(ForwardIndex::const_iterator iter = begin(); iter != end(); ++iter)
            termIdList.push_back(iter->first);
        return true;
    }

    bool getTermOffsetListByTermId(unsigned int termId, 
				std::vector<std::pair<unsigned int, unsigned int> >& termOffsetList) const
    {
        ForwardIndex::const_iterator iter = find(termId);
        if(iter == end())
            return false;
        termOffsetList.resize(iter->second->size());
        for(ForwardIndexOffset::iterator it = iter->second->begin(); it != iter->second->end(); ++it)
            termOffsetList.push_back(std::make_pair(it->first, it->second));
        return true;
    }

    bool insertTermOffset(unsigned int termId,
                                    pair<unsigned int, unsigned int>& termOffset)
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

	
};

}

NS_IZENELIB_IR_END

#endif
