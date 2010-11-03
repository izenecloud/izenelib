#include <ir/index_manager/index/FixedBlockSkipListReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

FixedBlockSkipListReader::FixedBlockSkipListReader(IndexInput* pSkipInput, int startBlock)
	:currBlockId_(startBlock)
	,totalSkipped_(0)
	,numSkipped_(0)
	,skipDoc_(0)
	,lastDoc_(0)
	,lastBlockId_(0)
	,pOffset_(0)
	,lastPOffset_(0)
{
    skipStream_ = pSkipInput->clone();
    fileoffset_t length = skipStream_->readVLong();
    skipStream_->setlength(skipStream_->getFilePointer() + length);
}		

FixedBlockSkipListReader::~FixedBlockSkipListReader()
{
    delete skipStream_;
}

docid_t FixedBlockSkipListReader::skipTo(docid_t target)
{
    while(target > skipDoc_)
    {
	if (!loadNextSkip()) 
           break;
    }
    return lastDoc_;
}

bool FixedBlockSkipListReader::nextSkip(docid_t docID)
{
    if(skipDoc_ == 0)
    {
        if(!loadNextSkip())
            return false;
    }
    if(skipDoc_ <= docID)
    {
        loadNextSkip();
        return true;
    }
    return false;
}

bool FixedBlockSkipListReader::loadNextSkip()
{
    lastDoc_ = skipDoc_;
    lastPOffset_ = pOffset_;
    totalSkipped_ = numSkipped_;
    lastBlockId_ = currBlockId_;
    if (skipStream_->isEof()) 
    {
        /// this skip list is exhausted
        skipDoc_ = (docid_t)-1;
        return false;
    }

    /// read next skip entry
    ++currBlockId_;
    skipDoc_ += skipStream_->readVInt();
    numSkipped_ += skipStream_->readVInt();
    pOffset_ += skipStream_->readVLong();
    return true;
}

}
NS_IZENELIB_IR_END


