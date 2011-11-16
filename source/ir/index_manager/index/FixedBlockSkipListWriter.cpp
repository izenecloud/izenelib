#include <ir/index_manager/index/FixedBlockSkipListWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

FixedBlockSkipListWriter::FixedBlockSkipListWriter(boost::shared_ptr<MemCache> pMemCache)
    :lastDoc_(0)
    ,lastPOffset_(0)
    ,curDoc_(0)
    ,curPOffset_(0)
{
    pSkipLevel_ = new VariantDataPool(pMemCache);
}

FixedBlockSkipListWriter::~FixedBlockSkipListWriter()
{
    delete pSkipLevel_;
}


void FixedBlockSkipListWriter::addSkipPoint(docid_t docId,uint32_t numSkipped,fileoffset_t pOffset)
{
    curDoc_ = docId;
    curPOffset_ = pOffset;

    pSkipLevel_->addVData32(curDoc_ - lastDoc_);
    pSkipLevel_->addVData32(numSkipped);
    pSkipLevel_->addVData64((uint64_t)(curPOffset_ - lastPOffset_));

    lastDoc_ = curDoc_;
    lastPOffset_ = curPOffset_;    
}

fileoffset_t FixedBlockSkipListWriter::getRealLength()
{
    fileoffset_t length = 0;
    if(pSkipLevel_->getLength() > 0)
    {
        fileoffset_t nLength = pSkipLevel_->getLength();
        length += nLength;
        length += IndexOutput::getVLongLength(nLength);
    }
    return length;
}

void FixedBlockSkipListWriter::write(IndexOutput* pOutput)
{
    if(pSkipLevel_->getLength() > 0)
    {
        pSkipLevel_->truncTailChunk();
        fileoffset_t nLength = pSkipLevel_->getLength();
        pOutput->writeVLong(nLength);
        pSkipLevel_->write(pOutput);
    }
}

void FixedBlockSkipListWriter::reset()
{
    lastDoc_= 0;
    lastPOffset_ = 0;
    
    pSkipLevel_->reset();
}

}
NS_IZENELIB_IR_END


