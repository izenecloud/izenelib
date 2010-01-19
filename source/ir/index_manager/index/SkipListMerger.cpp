#include <ir/index_manager/index/SkipListMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListMerger::SkipListMerger(int skipInterval, int maxLevel, MemCache* pMemCache)
    :SkipListWriter(skipInterval, maxLevel, pMemCache)
    ,baseDocID_(0)
    ,baseOffset_(0)
    ,basePOffset_(0)		
{
}

SkipListMerger::~SkipListMerger()
{
}

void SkipListMerger::add(SkipListReader* pSkipReader)
{
    SkipListWriter::addSkipPoint( pSkipReader->getDoc() + baseDocID_, 
                        pSkipReader->getOffset() + baseOffset_,
                        pSkipReader->getPOffset() + basePOffset_
                      );
}

bool SkipListMerger::addToMerge(SkipListReader* pSkipReader,docid_t lastDoc)
{			
    bool ret = false;
    while(pSkipReader->nextSkip(lastDoc))
    {				
        add(pSkipReader);
        ret = true;
    }
    return ret;
}

}
NS_IZENELIB_IR_END


