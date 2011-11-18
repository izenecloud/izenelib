#include <util/compressed_vector/OrderedVector.h>

NS_IZENELIB_UTIL_BEGIN

namespace compressed_vector{

OrderedVector::OrderedVector(MemPool* pMemPool)
    :pMemPool_(pMemPool)
    ,pHeadChunk_(NULL)
    ,pTailChunk_(NULL)
    ,nTotalSize_(0)
    ,nPosInCurChunk_(0)
    ,nTotalUsed_(0)
    ,nLastVal_(0)
    ,nCount_(0)
{
}

OrderedVector::~OrderedVector()
{
}

void OrderedVector::push_back(uint32_t val)
{
    add_v_data(val - nLastVal_);
    nLastVal_ = val;
}

void OrderedVector::add_v_data(uint32_t val)
{
    if (pTailChunk_ == NULL)
    {
        add_chunk();
        add_v_data(val);
    }
    int32_t left = pTailChunk_->size - nPosInCurChunk_;
	
    if (left < 7)///at least 4 free space
    {
        pTailChunk_->size = nPosInCurChunk_;///the real size
        add_chunk();
        add_v_data(val);
        return;
    }
	
    uint32_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
        nTotalUsed_++;
    }
    pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
    nTotalUsed_++;
    nCount_++;
}

void OrderedVector::add_chunk()
{
    size_t chunkSize = std::min(CHUNK_ALLOC_UPPER_LIMIT, 
                  std::max(CHUNK_ALLOC_LOWER_LIMIT,(int)(nTotalSize_*0.5 + 0.5)));

    uint8_t* begin = (uint8_t *)pMemPool_->Allocate(chunkSize);

    detail::DataChunk* pChunk = (detail::DataChunk*)begin;
    pChunk->size = (int32_t)(chunkSize - sizeof(detail::DataChunk*) - sizeof(int32_t));
    pChunk->next = NULL;
 
    if (pTailChunk_)
        pTailChunk_->next = pChunk;
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += pChunk->size;
    nPosInCurChunk_ = 0;
}

void OrderedVector::read_internal(
    detail::DataChunk* &pDataChunk, 
    int32_t& currPosInChunk, 
    uint8_t* buffer, 
    size_t length
) const
{
    if(!pDataChunk) return;
    size_t nLen = length;
    uint8_t* pEnd = pDataChunk->data + pDataChunk->size;
    uint8_t* pData = pDataChunk->data + currPosInChunk;
    while(nLen > 0)
    {
        if(nLen > (size_t)(pEnd - pData))
        {
            memcpy(buffer,pData,pEnd - pData);
            buffer += (pEnd - pData);
            nLen -= (pEnd - pData);
            pDataChunk = pDataChunk->next;
            if(pDataChunk)
            {
                pData = pDataChunk->data;
                pEnd = pDataChunk->data + pDataChunk->size;
            }
            else
            {
                pData = NULL;
                break;
            }
        }
        else
        {
            memcpy(buffer,pData,nLen);					
            if(nLen == (size_t)(pEnd - pData))
            {
                pDataChunk = pDataChunk->next;
                if(pDataChunk)
                    pData = pDataChunk->data;
                else
                    pData = NULL;
            }						
            else
            {
                pData += nLen;
            }
            nLen = 0;
        }
    }
    currPosInChunk = (pData == NULL)? 0 : (int32_t)(pData - pDataChunk->data);
}

}
NS_IZENELIB_UTIL_END

