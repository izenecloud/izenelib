#include <ir/index_manager/index/SkipListMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListMerger::SkipListMerger(
    int skipInterval, 
    int maxLevel, 
    boost::shared_ptr<MemCache> pMemCache)
    :SkipListWriter(skipInterval, maxLevel, pMemCache)
    ,baseDocID_(0)
    ,baseOffset_(0)
    ,basePOffset_(0)		
{
    skipIntervals_.assign(maxLevel, 0);
}

SkipListMerger::~SkipListMerger()
{
}

void SkipListMerger::writeSkipData(int level)
{
    if(skipIntervals_[level] == getSkipInterval(level))
        ppSkipLevels_[level]->addVData32( (curDoc_ - pLastDoc_[level]) << 1);
    else
    {
        ppSkipLevels_[level]->addVData32( ((curDoc_ - pLastDoc_[level]) << 1) + 1);
        ppSkipLevels_[level]->addVData32(skipIntervals_[level]);
    }
    skipIntervals_[level] = 0;
    ppSkipLevels_[level]->addVData64((uint64_t)(curOffset_ - pLastOffset_[level]));
    ppSkipLevels_[level]->addVData64((uint64_t)(curPOffset_ - pLastPOffset_[level]));
}

bool SkipListMerger::addToMerge(
    SkipListReader* pSkipReader,
    docid_t lastDoc,
    int nSkipIntervalBetweenBarrels)
{
    int lastdoc = 0;
    while(pSkipReader->nextSkip(lastDoc))
    {
    	int nCurSkipped = (pSkipReader->getSkipInterval() + nSkipIntervalBetweenBarrels);
	for(int i = 0;i < maxSkipLevel_;i++)
            skipIntervals_[i] += nCurSkipped;

	if(skipIntervals_[0] <= 1)
           continue;

        addSkipPoint( pSkipReader->getDoc() + baseDocID_, 
                            pSkipReader->getOffset() + baseOffset_,
                            pSkipReader->getPOffset() + basePOffset_
                          );
	lastdoc = pSkipReader->getDoc();
	nSkipIntervalBetweenBarrels = 0;
    }
    return true;
}

}
NS_IZENELIB_IR_END


