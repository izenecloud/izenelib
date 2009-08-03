#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

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
				std::vector<std::pair<unsigned int, unsigned int> >& termOffsetList)
    {
        ForwardIndexOffset* pForwardIndexOffset = (ForwardIndexOffset*)(*this)[termId];
        termOffsetList.resize(pForwardIndexOffset->size());
        for(ForwardIndexOffset::iterator iter = pForwardIndexOffset->begin(); iter != pForwardIndexOffset->end(); ++iter)
            termOffsetList.push_back(std::make_pair(iter->first, iter->second));
        return true;
    }
	
};

}

NS_IZENELIB_IR_END

#endif
