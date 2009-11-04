#include <ir/index_manager/index/CompressedPostingList.h>

using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///CompressedPostingList
bool CompressedPostingList::addPosting(uint32_t posting32)
{
    if (pTailChunk == NULL)
        return false;
    int32_t left = pTailChunk->size - nPosInCurChunk;

    if (left < 7)///at least 7 free space
    {
        nTotalUnused += left;///Unused size
        pTailChunk->size = nPosInCurChunk;///the real size
        return false;
    }

    uint32_t ui = posting32;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk->data[nPosInCurChunk++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk->data[nPosInCurChunk++] = (uint8_t)ui;
    return true;
}

bool CompressedPostingList::addPosting(uint64_t posting64)
{
    if (pTailChunk == NULL)
        return false;
    int32_t left = pTailChunk->size - nPosInCurChunk;
    if (left < 11)///at least 11 free space
    {
        nTotalUnused += left;///Unused size
        pTailChunk->size = nPosInCurChunk;///the real size
        return false;
    }

    uint64_t ui = posting64;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk->data[nPosInCurChunk++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk->data[nPosInCurChunk++] = ((uint8_t)ui);

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
    nTotalUnused += pTailChunk->size - nPosInCurChunk;
    pTailChunk->size = nPosInCurChunk;
}

void CompressedPostingList::addChunk(PostingChunk* pChunk)
{
    if (pTailChunk)
        pTailChunk->next = pChunk;
    pTailChunk = pChunk;
    if (!pHeadChunk)
        pHeadChunk = pTailChunk;
    nTotalSize += pChunk->size;

    nPosInCurChunk = 0;
}
int32_t CompressedPostingList::getRealSize()
{
    return nTotalSize - nTotalUnused;
}

void CompressedPostingList::reset()
{
    pHeadChunk = pTailChunk = NULL;
    nTotalSize = nPosInCurChunk = nTotalUnused = 0;
}

