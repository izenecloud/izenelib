#include <ir/index_manager/index/SkipListMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListMerger::SkipListMerger(int skipInterval, int maxLevel, MemCache* pMemCache)
    :SkipListWriter(skipInterval, maxLevel, pMemCache)
    ,baseDocID_(0)
    ,baseOffset_(0)
    ,basePOffset_(0)		
{
    pSkipInterval_ = new int[maxLevel];
    memset(pSkipInterval_,0,maxLevel * sizeof(int));
}

SkipListMerger::~SkipListMerger()
{
    if(pSkipInterval_)
    {
        delete[] pSkipInterval_;
        pSkipInterval_ = NULL;
    }
}

bool SkipListMerger::setSkipPoint(SkipListReader* pSkipReader,int nAppendedSkipInterval)
{
    int nCurSkipped = (pSkipReader->getCurSkipInterval() + nAppendedSkipInterval);
    for(int i = 0;i < maxSkipLevel_;i++)
        ppSkipLevels_[i] += nCurSkipped;

    if(pSkipInterval_[0] <= 1)
        return false;

    curDoc_ = pSkipReader->getDoc() + baseDocID_;
    curOffset_ = pSkipReader->getOffset() + baseOffset_;
    curPOffset_ = pSkipReader->getPOffset() + basePOffset_;
    return true;
}

bool SkipListMerger::addToMerge(SkipListReader* pSkipReader,docid_t lastDoc,int nAppendedSkipInterval)
{			
    bool ret = false;
    while(pSkipReader->stepTo(lastDoc))
    {				
        if(setSkipPoint(pSkipReader,nAppendedSkipInterval))					
            addSkipPoint();
        ret = true;
        nAppendedSkipInterval = 0;
    }
    return ret;
}

void SkipListMerger::writeSkipData(int level,IndexOutput* pSkipLevelOutput)
{			
    if(pSkipInterval_[level] == getSkipInterval(level))
        pSkipLevelOutput->writeVInt( (curDoc_ - pLastDoc_[level]) << 1);
    else
    {
        pSkipLevelOutput->writeVInt( ((curDoc_ - pLastDoc_[level]) << 1) + 1);
        pSkipLevelOutput->writeVInt(pSkipInterval_[level]);
    }
    pSkipInterval_[level] = 0;

    pSkipLevelOutput->writeVLong(curOffset_ - pLastOffset_[level]);
    pSkipLevelOutput->writeVLong(curPOffset_ - pLastPOffset_[level]);

    pLastDoc_[level] = curDoc_;
    pLastOffset_[level] = curOffset_;
    pLastPOffset_[level] = curPOffset_;
}

void SkipListMerger::reset()
{
    SkipListWriter::reset();
    for(int i = 0;i < maxSkipLevel_;i++)
        pSkipInterval_[i] = 0;
}

}
NS_IZENELIB_IR_END


