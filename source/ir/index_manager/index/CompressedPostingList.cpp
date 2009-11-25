#include <ir/index_manager/index/CompressedPostingList.h>

using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///CompressedPostingList
bool CompressedPostingList::addPosting(uint32_t posting32)
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

    uint32_t ui = posting32;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
    return true;
}

bool CompressedPostingList::addPosting(uint64_t posting64)
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

    uint64_t ui = posting64;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)ui);

    return true;
}
int32_t CompressedPostingList::decodePosting32(uint8_t*& posting)
{
    uint8_t b = *posting++;
    int32_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *posting++;
        i |= (b & 0x7FL) << shift;
    }
    return i;
}

int64_t CompressedPostingList::decodePosting64(uint8_t*& posting)
{
    uint8_t b = *posting++;
    int64_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *posting++;
        i |= (b & 0x7FLL) << shift;
    }
    return i;
}

void CompressedPostingList::encodePosting32(uint8_t*& posting,int32_t val)
{
    uint32_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *posting++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *posting++ = (uint8_t)ui;
}
void CompressedPostingList::encodePosting64(uint8_t*& posting,int64_t val)
{
    uint64_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *posting++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *posting++ = ((uint8_t)ui);
}

void CompressedPostingList::truncTailChunk()
{
    nTotalUnused_ += pTailChunk_->size - nPosInCurChunk_;
    pTailChunk_->size = nPosInCurChunk_;
}

void CompressedPostingList::addChunk(PostingChunk* pChunk)
{
    if (pTailChunk_)
        pTailChunk_->next = pChunk;
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += pChunk->size;

    nPosInCurChunk_ = 0;
}
int32_t CompressedPostingList::getRealSize()
{
    return nTotalSize_ - nTotalUnused_;
}

void CompressedPostingList::reset()
{
    pHeadChunk_ = pTailChunk_ = NULL;
    nTotalSize_ = nPosInCurChunk_ = nTotalUnused_ = 0;
}

