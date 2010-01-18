#include <ir/index_manager/index/VariantDataPool.h>

using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///VariantDataPool
int32_t VariantDataPool::UPTIGHT_ALLOC_CHUNKSIZE = 8;
int32_t VariantDataPool::UPTIGHT_ALLOC_MEMSIZE = 40000;

VariantDataPool::VariantDataPool(MemCache* pMemCache)
		:pMemCache_(pMemCache)
		,pHeadChunk_(NULL)
		,pTailChunk_(NULL)
		,nTotalSize_(0)
		,nTotalUnused_(0)
		,nPosInCurChunk_(0)
{
}

VariantDataPool::VariantDataPool(const VariantDataPool& src)
		:pMemCache_(src.pMemCache_)
		,pHeadChunk_(src.pHeadChunk_)
		,pTailChunk_(src.pTailChunk_)
		,nTotalSize_(src.nTotalSize_)
		,nTotalUnused_(src.nTotalUnused_)
		,nPosInCurChunk_(src.nPosInCurChunk_)
{
}

VariantDataPool::~VariantDataPool()
{
}

bool VariantDataPool::addVData32(uint32_t vdata32)
{
    if (pTailChunk_ == NULL)
    {
        addChunk();
        return addVData32(vdata32);
    }
    int32_t left = pTailChunk_->size - nPosInCurChunk_;

    if (left < 7)///at least 4 free space
    {
        nTotalUnused_ += left;///Unused size
        pTailChunk_->size = nPosInCurChunk_;///the real size
        addChunk();
        return addVData32(vdata32);
    }

    uint32_t ui = vdata32;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
    return true;
}

bool VariantDataPool::addVData64(uint64_t vdata64)
{
    if (pTailChunk_ == NULL)
    {
        addChunk();
        return addVData64(vdata64);
    }
    int32_t left = pTailChunk_->size - nPosInCurChunk_;
    if (left < 11)///at least 8 free space
    {
        nTotalUnused_ += left;///Unused size
        pTailChunk_->size = nPosInCurChunk_;///the real size
        addChunk();
        return addVData64(vdata64);
    }

    uint64_t ui = vdata64;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)ui);

    return true;
}

void VariantDataPool::write(IndexOutput* pOutput)
{
    VariantDataChunk* pChunk = pHeadChunk_;
    while (pChunk)
    {
        pOutput->write((const char*)pChunk->data,pChunk->size);
        pChunk = pChunk->next;
    }
}

int32_t VariantDataPool::decodeVData32(uint8_t*& vdata)
{
    uint8_t b = *vdata++;
    int32_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *vdata++;
        i |= (b & 0x7FL) << shift;
    }
    return i;
}

int64_t VariantDataPool::decodeVData64(uint8_t*& vdata)
{
    uint8_t b = *vdata++;
    int64_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *vdata++;
        i |= (b & 0x7FLL) << shift;
    }
    return i;
}

void VariantDataPool::encodeVData32(uint8_t*& vdata,int32_t val)
{
    uint32_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *vdata++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *vdata++ = (uint8_t)ui;
}
void VariantDataPool::encodeVData64(uint8_t*& vdata,int64_t val)
{
    uint64_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *vdata++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *vdata++ = ((uint8_t)ui);
}

void VariantDataPool::truncTailChunk()
{
    nTotalUnused_ += pTailChunk_->size - nPosInCurChunk_;
    pTailChunk_->size = nPosInCurChunk_;
}

void VariantDataPool::addChunk()
{
    int32_t factor = max(32,(int32_t)(nTotalSize_ + 0.5));
    int32_t chunkSize = (int32_t)Utilities::LOG2_UP(factor);

    uint8_t* begin = pMemCache_->getMem(chunkSize);
    ///allocate memory failed,decrease chunk size
    if (!begin)
    {
        ///into UPTIGHT state
        begin = pMemCache_->getMem(UPTIGHT_ALLOC_CHUNKSIZE);
        ///allocation failed again, grow memory cache.
        if (!begin)
        {
            begin  = pMemCache_->grow(UPTIGHT_ALLOC_MEMSIZE)->getMem(UPTIGHT_ALLOC_CHUNKSIZE);
            if (!begin)
            {
                SF1V5_THROW(ERROR_OUTOFMEM,"InMemoryPosting:newChunk() : Allocate memory failed.");
            }
        }
        chunkSize = UPTIGHT_ALLOC_CHUNKSIZE;
    }

    VariantDataChunk* pChunk = (VariantDataChunk*)begin;
    pChunk->size = (int32_t)(POW_TABLE[chunkSize] - sizeof(VariantDataChunk*) - sizeof(int32_t));
    pChunk->next = NULL;
 
    if (pTailChunk_)
        pTailChunk_->next = pChunk;
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += pChunk->size;

    nPosInCurChunk_ = 0;
}

int32_t VariantDataPool::getRealSize()
{
    return nTotalSize_ - nTotalUnused_;
}

void VariantDataPool::reset()
{
    pHeadChunk_ = pTailChunk_ = NULL;
    nTotalSize_ = nPosInCurChunk_ = nTotalUnused_ = 0;
}

//////////////////////////////////////////////////////
///VariantDataPoolInput
/////////////////////////////////////////////////////

VariantDataPoolInput::VariantDataPoolInput(VariantDataPool* pVDataPool)
    :pVDataPool_(pVDataPool)
    ,currPos_(0)
{
    setLength(pVDataPool->getRealSize());
    pVDataChunk_ = pVDataPool->pHeadChunk_;
    pData_ = pVDataChunk_->data;
}

VariantDataPoolInput::VariantDataPoolInput(const VariantDataPoolInput& src)
    :pVDataPool_(src.pVDataPool_)
    ,currPos_(0)
{
    setLength(pVDataPool_->getRealSize());
    pVDataChunk_ = pVDataPool_->pHeadChunk_;
    pData_ = pVDataChunk_->data;
}

VariantDataPoolInput::~VariantDataPoolInput()
{
    pVDataPool_ = NULL;
}

void VariantDataPoolInput::readInternal(char* b,size_t length,bool bCheck)
{
    if(bCheck && getFilePointer() != currPos_)
        seekInternal(getFilePointer());
    if(!pData_)
        SF1V5_THROW(ERROR_FILEIO,"VariantDataPoolInput::readInternal():read past EOF" );

    size_t nLen = length;
    uint8_t* pEnd = pVDataChunk_->data + pVDataChunk_->size;				
    while(nLen > 0)
    {
        if(nLen > (size_t)(pEnd - pData_))
        {
            memcpy(b,pData_,pEnd - pData_);
            b += (pEnd - pData_);
            nLen -= (pEnd - pData_);
            pVDataChunk_ = pVDataChunk_->next;
            if(pVDataChunk_)
            {
                pData_ = pVDataChunk_->data;
                pEnd = pVDataChunk_->data + pVDataChunk_->size;
            }
            else
            {
                pData_ = NULL;
                break;
            }
        }
        else
        {
            memcpy(b,pData_,nLen); 					
            if(nLen == pEnd - pData_)
            {
                pVDataChunk_ = pVDataChunk_->next;
                if(pVDataChunk_)
                    pData_ = pVDataChunk_->data;
                else
                    pData_ = NULL;
            }						
            else
            {
                pData_ += nLen;
            }
            nLen = 0;
        }
    }
    if(nLen > 0)
    {
        SF1V5_THROW(ERROR_FILEIO,"VariantDataPoolInput::readInternal():read past EOF" );
    }
    currPos_ += length;
}


void VariantDataPoolInput::seekInternal(int64_t position)
{
    int64_t nSkip = position;
    pData_ = pVDataChunk_->data;
    while(nSkip > 0 && pVDataChunk_)
    {
        if(nSkip >= pVDataChunk_->size)
        {
            nSkip -= pVDataChunk_->size;					
            pVDataChunk_ = pVDataChunk_->next;
            pData_ = pVDataChunk_->data;
        }
        else
        {
            pData_ = pVDataChunk_->data + nSkip;
            nSkip = 0;
        }
    }
    currPos_ = position;
    if(nSkip > 0)
        SF1V5_THROW(ERROR_FILEIO,"VariantDataPoolInput::readInternal():file IO seek error: VariantDataPoolInput" );
}

void VariantDataPoolInput::close()
{
}

IndexInput* VariantDataPoolInput::clone()
{
    return new VariantDataPoolInput(*this);
}

