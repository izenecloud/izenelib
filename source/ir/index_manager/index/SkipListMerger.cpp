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

bool SkipListMerger::addToMerge(SkipListReader* pSkipReader,docid_t lastDoc)
{			
    bool ret = false;
    while(pSkipReader->nextSkip(lastDoc))
    {
        SkipListWriter::addSkipPoint( pSkipReader->getDoc() + baseDocID_, 
                                                  pSkipReader->getOffset() + baseOffset_,
                                                  pSkipReader->getPOffset() + basePOffset_
                                                );
        ret = true;
    }
    if(pSkipReader->getDoc()>0 && pSkipReader->getDoc()<BAD_DOCID)
    {
        SkipListWriter::addSkipPoint( pSkipReader->getDoc() + baseDocID_, 
                                                  pSkipReader->getOffset() + baseOffset_,
                                                  pSkipReader->getPOffset() + basePOffset_
                                                );
    }

    return ret;
}

}
NS_IZENELIB_IR_END


