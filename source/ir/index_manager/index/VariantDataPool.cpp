#include <ir/index_manager/index/VariantDataPool.h>

using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///VariantDataPool
int32_t VariantDataPool::UPTIGHT_ALLOC_MEMSIZE = 10*1024*1024;

VariantDataPool::VariantDataPool(
    boost::shared_ptr<MemCache> pMemCache)
    :pMemCache_(pMemCache)
    ,pHeadChunk_(NULL)
    ,pTailChunk_(NULL)
    ,nTotalSize_(0)
    ,nPosInCurChunk_(0)
    ,nTotalUsed_(0)
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
        pTailChunk_->size = nPosInCurChunk_;///the real size
        addChunk();
        return addVData32(vdata32);
    }
    uint32_t ui = vdata32;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
        nTotalUsed_++;
    }
    pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
    nTotalUsed_++;
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
        pTailChunk_->size = nPosInCurChunk_;///the real size
        addChunk();
        return addVData64(vdata64);
    }

    uint64_t ui = vdata64;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
        nTotalUsed_++;
    }
    pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)ui);
    nTotalUsed_++;

    return true;
}

void VariantDataPool::writeVInt(int32_t i)
{
    addVData32(i);
}

void VariantDataPool::write(IndexInput* pInput,int64_t length)
{
    size_t memSize = length + sizeof(int32_t) + sizeof(VariantDataChunk*);
    uint8_t* begin = pMemCache_->getMemByRealSize(memSize);
    if (!begin)
    {
        MemCache* pUrgentMemCache = pMemCache_->grow(memSize);
  
        begin  = pUrgentMemCache->getMemByRealSize(memSize);
        if (!begin)
        {
            SF1V5_THROW(ERROR_OUTOFMEM,"VariantDataPool:write() : Allocate memory failed.");
        }
    }
    VariantDataChunk* pChunk = (VariantDataChunk*)begin;
    pChunk->size = (int32_t)length;
    pChunk->next = NULL;
    pInput->readBytes(pChunk->data,(size_t)length);
 
    if (pTailChunk_)
    {
        pTailChunk_->next = pChunk;
        pTailChunk_->size = nPosInCurChunk_;
    }
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += length;
    nTotalUsed_ += length; 

    nPosInCurChunk_ = length;
}

void VariantDataPool::write(const char* data,size_t length)
{
    size_t memSize = length + sizeof(int32_t) + sizeof(VariantDataChunk*);
    uint8_t* begin = pMemCache_->getMemByRealSize(memSize);
    if (!begin)
    {
        MemCache* pUrgentMemCache = pMemCache_->grow(memSize);
  
        begin  = pUrgentMemCache->getMemByRealSize(memSize);
        if (!begin)
        {
            SF1V5_THROW(ERROR_OUTOFMEM,"VariantDataPool:write() : Allocate memory failed.");
        }
    }
    VariantDataChunk* pChunk = (VariantDataChunk*)begin;
    pChunk->size = (int32_t)length;
    pChunk->next = NULL;

    memcpy(pChunk->data,data,length);
 
    if (pTailChunk_)
    {
        pTailChunk_->next = pChunk;
        pTailChunk_->size = nPosInCurChunk_;
    }
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += length;
    nTotalUsed_ += length; 

    nPosInCurChunk_ = length;
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
    pTailChunk_->size = nPosInCurChunk_;
}

void VariantDataPool::addChunk()
{
    int32_t factor = max(32,(int32_t)(nTotalSize_ + 0.5));
    int32_t chunkSize = (int32_t)Utilities::LOG2_UP(factor);

    uint8_t* begin = pMemCache_->getMemByLogSize(chunkSize);
    ///allocate memory failed,decrease chunk size
    if (!begin)
    {
        ///into UPTIGHT state
        begin = pMemCache_->getMemByLogSize(chunkSize);
        ///allocation failed again, grow memory cache.
        if (!begin)
        {
            MemCache* pUrgentMemCache = pMemCache_->grow(UPTIGHT_ALLOC_MEMSIZE);
            size_t urgentChunkSize = min((int32_t)Utilities::LOG2_DOWN(UPTIGHT_ALLOC_MEMSIZE),chunkSize);
  
            begin  = pUrgentMemCache->getMemByLogSize(urgentChunkSize);
            if (!begin)
            {
                SF1V5_THROW(ERROR_OUTOFMEM,"InMemoryPosting:newChunk() : Allocate memory failed.");
            }
             chunkSize = urgentChunkSize;
        }
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

int64_t VariantDataPool::getLength()
{
    return (int64_t)nTotalUsed_;
}

void VariantDataPool::reset()
{
    pHeadChunk_ = pTailChunk_ = NULL;
    nTotalSize_ = nPosInCurChunk_ = nTotalUsed_ = 0;
}

//////////////////////////////////////////////////////
///VariantDataPoolInput
/////////////////////////////////////////////////////

VariantDataPoolInput::VariantDataPoolInput(VariantDataPool* pVDataPool)
    :pVDataPool_(pVDataPool)
{
    pVHeadChunk_ = pVDataPool_->pHeadChunk_; 
    setlength(pVDataPool_->getLength());

    IndexInput::setBuffer((char*)pVHeadChunk_->data,pVHeadChunk_->size);			
    bufferSize_ = pVHeadChunk_->size;

    pVDataChunk_ = pVHeadChunk_->next;
    if(pVDataChunk_)
        pData_ = pVDataChunk_->data;
    else
        pVDataChunk_ = NULL;

    currPos_ = pVHeadChunk_->size;
    if(currPos_ > pVDataPool->getLength())
        currPos_ = pVDataPool->getLength();
}

VariantDataPoolInput::VariantDataPoolInput(const VariantDataPoolInput& src)
    :pVDataPool_(src.pVDataPool_)
{
    pVHeadChunk_ = pVDataPool_->pHeadChunk_; 
    setlength(pVDataPool_->getLength());

    IndexInput::setBuffer((char*)pVHeadChunk_->data,pVHeadChunk_->size);			
    bufferSize_ = pVHeadChunk_->size;

    pVDataChunk_ = pVHeadChunk_->next;
    if(pVDataChunk_)
        pData_ = pVDataChunk_->data;
    else
        pVDataChunk_ = NULL;

    currPos_ = pVHeadChunk_->size;
    if(currPos_ > pVDataPool_->getLength())
        currPos_ = pVDataPool_->getLength();
}

VariantDataPoolInput::~VariantDataPoolInput()
{
}

void VariantDataPoolInput::readInternal(char* b,size_t length,bool bCheck)
{
    if(bCheck && getFilePointer() != currPos_)
        seekInternal(getFilePointer());
    if(!pData_)
        SF1V5_THROW(ERROR_FILEIO,"VariantDataPoolInput::readInternal():read past EOF" );

    if(buffer_ == b)
    {
        ///IndexInput::refill()
        assert(bufferSize_ == length); 
        buffer_ = (char*)pData_;
        bufferLength_ = bufferSize_ = (pVDataChunk_->data + pVDataChunk_->size) - pData_;
        currPos_ += bufferSize_;

        pVDataChunk_ = pVDataChunk_->next;
        if(pVDataChunk_)
            pData_ = pVDataChunk_->data;
        else 
            pData_ = NULL;				
    }
    else
    {
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
                if(nLen == (size_t)(pEnd - pData_))
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
        //currPos_ += (length - nLen);
        }
        else
            currPos_ += length;
    }
}


void VariantDataPoolInput::seekInternal(int64_t position)
{
    int64_t nSkip = position;
    pVDataChunk_ = pVHeadChunk_;
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

