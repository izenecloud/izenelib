#include <ir/index_manager/index/SkipListWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListWriter::SkipListWriter(int skipInterval, int maxLevel, MemCache* pMemCache)
    :skipInterval_(skipInterval)
    ,maxSkipLevel_(maxLevel)
    ,numPointsInLowestLevel_(0)
    ,curDoc_(0)
    ,curOffset_(0)
    ,curPOffset_(0)
{
    ppSkipLevels_ = new VariantDataPool*[maxLevel];
    memset(ppSkipLevels_,0,maxLevel*sizeof(VariantDataPool*)); 
    for(int i = 0; i < maxLevel; i++)
	ppSkipLevels_[i] = new VariantDataPool(pMemCache);

    pLastDoc_ = new docid_t[maxLevel];
    memset(pLastDoc_,0,maxLevel*sizeof(docid_t));
    pLastOffset_ = new fileoffset_t[maxLevel];
    memset(pLastOffset_,0,maxLevel*sizeof(fileoffset_t));
    pLastPOffset_ = new fileoffset_t[maxLevel];
    memset(pLastPOffset_,0,maxLevel*sizeof(fileoffset_t));			
}

SkipListWriter::~SkipListWriter()
{
    for(int i = 0; i < maxSkipLevel_; i++)
        if(ppSkipLevels_[i])
        {
            delete ppSkipLevels_[i];
            ppSkipLevels_[i] = NULL;
        }
    delete[] ppSkipLevels_;

    delete[] pLastDoc_;
    delete[] pLastOffset_;
    delete[] pLastPOffset_;
}

void SkipListWriter::addSkipPoint()
{
    int nNumLevels;   
    int nNumPoints = ++numPointsInLowestLevel_;
    for (nNumLevels = 1; (nNumPoints % skipInterval_) == 0 && nNumLevels < maxSkipLevel_; nNumPoints /= skipInterval_) 
        nNumLevels++;

    uint64_t nChildPointer = 0;	 

    for (int level = 0; level < nNumLevels; level++) 
    {
        ppSkipLevels_[level]->addVData(curDoc_ - pLastDoc_[level]);
        ppSkipLevels_[level]->addVData((uint64_t)(curOffset_ - pLastOffset_[level]));
        ppSkipLevels_[level]->addVData((uint64_t)(curPOffset_ - pLastPOffset_[level]));

        pLastDoc_[level] = curDoc_;
        pLastOffset_[level] = curOffset_;
        pLastPOffset_[level] = curPOffset_;    
		
        uint64_t nNewChildPointer = ppSkipLevels_[level]->getRealSize();
        if (level != 0) 
        {
            // store child pointers for all levels except the lowest
            ppSkipLevels_[level]->addVData(nChildPointer);
        }
        //remember the childPointer for the next level
        nChildPointer = nNewChildPointer;
    }	
}

void SkipListWriter::write(IndexOutput* pOutput)
{
    if (ppSkipLevels_ == NULL || getNumLevels() == 0)
        return;

    for (int i = maxSkipLevel_ - 1; i >= 0; i--)
    {
        if(ppSkipLevels_[i])
        {
            fileoffset_t nLength = ppSkipLevels_[i]->getRealSize();
            if (nLength > 0) 
            {
                pOutput->writeVLong(nLength);
                ppSkipLevels_[i]->write(pOutput);
            }
        }
    }			
}

void SkipListWriter::reset()
{
    for(int i = 0; i < maxSkipLevel_; i++)
    {
        pLastDoc_[i] = 0;
        pLastOffset_[i] = 0;
        pLastPOffset_[i] = 0;
    
        if(ppSkipLevels_[i])
            ppSkipLevels_[i]->reset();
    }
}

}
NS_IZENELIB_IR_END

