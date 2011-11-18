#include <ir/index_manager/index/SkipListWriter.h>
#include <ir/index_manager/index/SkipListReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

SkipListWriter::SkipListWriter(
    int skipInterval, 
    int maxLevel, 
    boost::shared_ptr<MemCache> pMemCache)
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

void SkipListWriter::writeSkipData(int level)
{
    ppSkipLevels_[level]->addVData32((curDoc_ - pLastDoc_[level])<<1);
    ppSkipLevels_[level]->addVData64((uint64_t)(curOffset_ - pLastOffset_[level]));
    ppSkipLevels_[level]->addVData64((uint64_t)(curPOffset_ - pLastPOffset_[level]));
}

void SkipListWriter::addSkipPoint(docid_t docId,fileoffset_t offset,fileoffset_t pOffset)
{
    curDoc_ = docId;
    curOffset_ = offset;
    curPOffset_ = pOffset;

    int nNumLevels;   
    int nNumPoints = ++numPointsInLowestLevel_;
    for (nNumLevels = 1; (nNumPoints % skipInterval_) == 0 && nNumLevels < maxSkipLevel_; nNumPoints /= skipInterval_) 
        nNumLevels++;

    uint64_t nChildPointer = 0;	 

    for (int level = 0; level < nNumLevels; level++) 
    {
        writeSkipData(level);

        pLastDoc_[level] = curDoc_;
        pLastOffset_[level] = curOffset_;
        pLastPOffset_[level] = curPOffset_;    
		
        uint64_t nNewChildPointer = ppSkipLevels_[level]->getLength();
        if (level != 0) 
        {
            // store child pointers for all levels except the lowest
            ppSkipLevels_[level]->addVData64(nChildPointer);
        }
        //remember the childPointer for the next level
        nChildPointer = nNewChildPointer;
    }	
}

fileoffset_t SkipListWriter::getRealLength()
{
    if (ppSkipLevels_ == NULL || getNumLevels() == 0)
        return 0;

    fileoffset_t length = 0;
    for (int i = maxSkipLevel_ - 1; i >= 0; i--)
    {
        if(ppSkipLevels_[i] &&ppSkipLevels_[i]->getLength() > 0)
        {
            fileoffset_t nLength = ppSkipLevels_[i]->getLength();
            length += nLength;
            length += IndexOutput::getVLongLength(nLength);
        }
    }
    return length;
}

void SkipListWriter::write(IndexOutput* pOutput)
{
    if (ppSkipLevels_ == NULL || getNumLevels() == 0)
        return;

    for (int i = maxSkipLevel_ - 1; i >= 0; i--)
    {
        if(ppSkipLevels_[i] &&ppSkipLevels_[i]->getLength() > 0)
        {
            ppSkipLevels_[i]->truncTailChunk();
            fileoffset_t nLength = ppSkipLevels_[i]->getLength();
            pOutput->writeVLong(nLength);
            ppSkipLevels_[i]->write(pOutput);
        }
    }
    numPointsInLowestLevel_ = 0;
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

SkipListReader* SkipListWriter::getSkipListReader()
{
    return new SkipListReader(ppSkipLevels_,getSkipInterval(0),getNumLevels());
}

}
NS_IZENELIB_IR_END

