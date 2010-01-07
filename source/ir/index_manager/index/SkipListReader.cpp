#include <ir/index_manager/index/SkipListReader.h>
#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListReader::SkipListReader(IndexInput* pSkipInput, int skipInterval, int numSkipLevels)
	: loaded_(false)
	, defaultSkipInterval_(skipInterval)
	, numSkipLevels_(numSkipLevels)
	, numSkipped_(0)
	, totalSkipped_(0)
	, curSkipInterval_(0)
	, lastChildPointer_(0)
{
    skipDoc_ = new docid_t[numSkipLevels_];
    memset(skipDoc_,0,numSkipLevels_ * sizeof(docid_t));

    skipInterval_ = new int[numSkipLevels_];
    memset(skipInterval_,0,numSkipLevels_ * sizeof(int));

    numSkipped_ = new int[numSkipLevels_];
    memset(numSkipped_,0,numSkipLevels_ * sizeof(int));

    childPointer_ = new fileoffset_t[numSkipLevels_];
    memset(childPointer_,0,numSkipLevels_ * sizeof(fileoffset_t));

    skipPointer_= new fileoffset_t[numSkipLevels_];
    memset(skipPointer_,0,numSkipLevels_ * sizeof(fileoffset_t));

    skipStream_ = new IndexInput*[numSkipLevels_];
    memset(skipStream_, 0, numSkipLevels_ * sizeof(IndexInput*));
    skipStream_[0] = pSkipInput->clone();

    offsets_ = new fileoffset_t[numSkipLevels_];
    memset(offsets_,0,numSkipLevels_ * sizeof(fileoffset_t));

    pOffsets_ = new fileoffset_t[numSkipLevels_];
    memset(pOffsets_,0,numSkipLevels_ * sizeof(fileoffset_t));
}		

SkipListReader::~SkipListReader()
{
}

docid_t SkipListReader::skipTo(docid_t docID)
{
    if (!loaded_) 
    {
        loadSkipLevels();
        loaded_ = true;
    }
	
    /// walk up the levels until highest level is found that has a skip for this target
    int level = 0;
    while (level < (numSkipLevels_-1) && docID > skipDoc_[level + 1]) 
    {
        level++;
    }

    while (level >= 0) 
    {
        if (docID > skipDoc_[level]) 
        {
            if (!loadNextSkip(level)) 
                continue;
        }
        else 
        {
            /// no more skips on this level, go down one level
            if (level > 0 && lastChildPointer_ > skipStream_[level - 1]->getFilePointer()) 
            {
                seekChild(level - 1);
            } 
            level--;
        }
    }
    return curDoc_;
}

bool SkipListReader::nextSkip(docid_t docID)
{
    if (!loaded_) 
    {
        loadSkipLevels();
        loaded_ = true;
    }
    if(skipDoc_[0] == 0)
    {
        if(!loadNextSkip(0))
            return false;
    }
    if(skipDoc_[0] <= docID)
    {
        loadNextSkip(0);
        return true;
    }
    return false;
}

void SkipListReader::seekChild(int level)
{
    skipStream_[level]->seek(lastChildPointer_);
    skipDoc_[level] = curDoc_;
    numSkipped_[level] = numSkipped_[level + 1];
    if (level > 0)
    {
        childPointer_[level] = skipStream_[level]->readVLong() + childPointer_[level - 1];
    }
}

bool SkipListReader::loadNextSkip(int level)
{
    curDoc_ = skipDoc_[level];
    lastChildPointer_ = childPointer_[level];
    lastOffset_ = offsets_[level];
    lastPOffset_ = pOffsets_[level];
    totalSkipped_ = numSkipped_[level];
    curSkipInterval_ = skipInterval_[level];

    if (skipStream_[level]->isEof()) 
    {
        /// this skip list is exhausted
        skipDoc_[level] = BAD_DOCID;
        return false;
    }

    /// read next skip entry
    readSkipPoint(level, skipStream_[level]);
    return true;
}

void SkipListReader::readSkipPoint(int level,IndexInput* pLevelInput)
{
    skipDoc_[level] += pLevelInput->readVInt();
    offsets_[level] += pLevelInput->readVInt();
    pOffsets_[level] += pLevelInput->readVInt();

    skipInterval_[level] = getLevelSkipInterval(level);
    numSkipped_[level] += skipInterval_[level];
    if (level > 0) 
    {
        /// read the child pointer if we are not on the leaf level
        childPointer_[level] = pLevelInput->readVLong() + skipPointer_[level- 1];
    }
}

void SkipListReader::loadSkipLevels()
{
    for (int i = numSkipLevels_-1; i >= 0; i--)
    {
        fileoffset_t length = skipStream_[0]->readVLong();
        skipPointer_[i] = skipStream_[0]->getFilePointer();
        if(i > 0)
        {
            skipStream_[i] = skipStream_[0]->clone();
        }
        skipStream_[i]->setlength(skipStream_[0]->getFilePointer() + length);
        if(i > 0)
        {
            skipStream_[0]->seek(skipStream_[0]->getFilePointer() + length);
        }
    }
}

void SkipListReader::reset()
{
    curSkipInterval_ = 0;
    numSkipped_ = 0;
    curDoc_ = 0;
    lastChildPointer_ = 0;
	
    memset(skipDoc_,0,numSkipLevels_ * sizeof(docid_t));
    memset(skipInterval_,0,numSkipLevels_ * sizeof(int));
    memset(numSkipped_,0,numSkipLevels_ * sizeof(int));
    memset(childPointer_,0,numSkipLevels_ * sizeof(fileoffset_t));			

    for(int i = 0;i < numSkipLevels_; i++)
    {
        if(skipStream_[i])
            skipStream_[i]->seek(skipPointer_[i]);
    }
    loaded_ = false;
}

}
NS_IZENELIB_IR_END

