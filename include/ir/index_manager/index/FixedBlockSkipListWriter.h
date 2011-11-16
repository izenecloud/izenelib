/**
* @file        FixedBlockSkipListWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Writing skiplist data for fixed block data
*/

#ifndef FIXED_BLOCK_SKIPLIST_WRITER_H
#define FIXED_BLOCK_SKIPLIST_WRITER_H

#include <ir/index_manager/index/VariantDataPool.h>
#include <ir/index_manager/utility/MemCache.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class FixedBlockSkipListWriter
{
public:
    FixedBlockSkipListWriter(boost::shared_ptr<MemCache> pMemCache);

    virtual ~FixedBlockSkipListWriter();

public:
    void addSkipPoint(docid_t docId,uint32_t numSkipped,fileoffset_t pOffset);

    fileoffset_t getRealLength();

    void write(IndexOutput* pOutput);

    void reset();

protected:
    VariantDataPool* pSkipLevel_; ///only one skip level is required

    ///each level should record the last doc id, last fileoffset of both dfp and pos postings
    docid_t lastDoc_; 
    fileoffset_t lastPOffset_;

    docid_t curDoc_;
    fileoffset_t curPOffset_;
};
}
NS_IZENELIB_IR_END

#endif

