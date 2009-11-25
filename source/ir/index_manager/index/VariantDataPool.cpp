#include <ir/index_manager/index/VariantDataPool.h>

using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///VariantDataPool
bool VariantDataPool::addVData(uint32_t vdata32)
{
    if (pTailChunk_ == NULL)
        return false;
    int32_t left = pTailChunk_->size - nPosInCurChunk_;

    if (left < 7)///at least 4 free space
    {
        nTotalUnused_ += left;///Unused size
        pTailChunk_->size = nPosInCurChunk_;///the real size
        return false;
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

bool VariantDataPool::addVData(uint64_t vdata64)
{
    if (pTailChunk_ == NULL)
        return false;
    int32_t left = pTailChunk_->size - nPosInCurChunk_;
    if (left < 11)///at least 8 free space
    {
        nTotalUnused_ += left;///Unused size
        pTailChunk_->size = nPosInCurChunk_;///the real size
        return false;
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

void VariantDataPool::addChunk(VariantDataChunk* pChunk)
{
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

