#ifndef OPT_P4_H_GUARD
#define OPT_P4_H_GUARD

#include "pf.hpp"


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

static uint32_t findBestB(uint32_t* docid)
{
    static const uint32_t bits[16] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 65535, 1048575};
    uint32_t offset[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (uint32_t i = 0; i < BLOCK_SIZE; ++i)
    {
        offset[0] += (docid[i] > bits[0]);
        offset[1] += (docid[i] > bits[1]);
        offset[2] += (docid[i] > bits[2]);
        offset[3] += (docid[i] > bits[3]);
        offset[4] += (docid[i] > bits[4]);
        offset[5] += (docid[i] > bits[5]);
        offset[6] += (docid[i] > bits[6]);
        offset[7] += (docid[i] > bits[7]);
        offset[8] += (docid[i] > bits[8]);
        offset[9] += (docid[i] > bits[9]);
        offset[10] += (docid[i] > bits[10]);
        offset[11] += (docid[i] > bits[11]);
        offset[12] += (docid[i] > bits[12]);
        offset[13] += (docid[i] > bits[13]);
        offset[14] += (docid[i] > bits[14]);
        offset[15] += (docid[i] > bits[15]);
    }

    uint32_t bestB = 0;
    uint32_t bestOffset = 0xFFFFFFFF;
    for (uint32_t i = 0; i < 17; ++i)
    {
        uint32_t temp = (offset[i] << 5) + cnum[i] * BLOCK_SIZE;
        if (temp < bestOffset)
        {
            bestB = i;
            bestOffset = temp;
        }
    }

    return bestB;
}

static uint32_t OPT4(uint32_t *doc_id, uint32_t list_size, uint32_t *aux, bool delta)
{
    uint32_t size = 0;
    uint32_t ex_n = 0;

    if (delta)
    {
        if (list_size > 1)
        {
            if (doc_id[0] < doc_id[1])
            {
                for (int i = list_size - 1; i > 0; --i)
                {
                    doc_id[i] -= doc_id[i - 1];
                }
            }
            else
            {
                for (uint32_t i = 0; i < list_size - 1; ++i)
                {
                    doc_id[i] -= doc_id[i + 1];
                }
            }
        }
    }

    uint32_t b = findBestB(doc_id);
    uint32_t *ww = aux;
    detailed_p4_encode(&ww, doc_id, b, &size, &ex_n);

    return size;
}

}

NS_IZENELIB_IR_END

#endif
