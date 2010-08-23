#ifndef CODING_H_
#define CODING_H_

#include <cassert>
#include <stdint.h>

#include <iostream>

#include <util/compression/int/unpack.h>

namespace izenelib{namespace util{namespace compression{

#define TESTBIT(buffer, bp) ((((buffer)[(bp)>>5])>>((bp)&31))&1)

class coding
{
public:
    coding()
    {
        MASK[0] = 0x00000000;
        MASK[1] = 0x00000001;
        MASK[2] = 0x00000003;
        MASK[3] = 0x00000007;
        MASK[4] = 0x0000000f;
        MASK[5] = 0x0000001f;
        MASK[6] = 0x0000003f;
        MASK[7] = 0x0000007f;
        MASK[8] = 0x000000ff;
        MASK[9] = 0x000001ff;
        MASK[10] = 0x000003ff;
        MASK[11] = 0x000007ff;
        MASK[12] = 0x00000fff;
        MASK[13] = 0x00001fff;
        MASK[14] = 0x00003fff;
        MASK[15] = 0x00007fff;
        MASK[16] = 0x0000ffff;
        MASK[17] = 0x0001ffff;
        MASK[18] = 0x0003ffff;
        MASK[19] = 0x0007ffff;
        MASK[20] = 0x000fffff;
        MASK[21] = 0x001fffff;
        MASK[22] = 0x003fffff;
        MASK[23] = 0x007fffff;
        MASK[24] = 0x00ffffff;
        MASK[25] = 0x01ffffff;
        MASK[26] = 0x03ffffff;
        MASK[27] = 0x07ffffff;
        MASK[28] = 0x0fffffff;
        MASK[29] = 0x1fffffff;
        MASK[30] = 0x3fffffff;
        MASK[31] = 0x7fffffff;
        MASK[32] = 0xffffffff;

        unpack[0] = unpack0;
        unpack[1] = unpack1;
        unpack[2] = unpack2;
        unpack[3] = unpack3;
        unpack[4] = unpack4;
        unpack[5] = unpack5;
        unpack[6] = unpack6;
        unpack[7] = unpack7;
        unpack[8] = unpack8;
        unpack[9] = unpack9;
        unpack[10] = unpack10;
        unpack[11] = unpack11;
        unpack[12] = unpack12;
        unpack[13] = unpack13;
        unpack[14] = unpack16;
        unpack[15] = unpack20;
        unpack[16] = unpack32;
    }

protected:
    // Packs the 'b' least significant bits of the 'n' elements from input buffer at 'v' into output buffer at 'w'.
    // 'v' is a pointer to the input array of the numbers to be packed.
    // 'b' is the bit width per element in the resulting compressed array.
    // 'n' is the number of elements to be packed.
    // 'w' is a pointer to the output array.
    // Note: this function does not modify the output array pointer, it will need to be calculated and modified outside this function.
    void pack(unsigned int* v, unsigned int b, unsigned int n, unsigned int* w)
    {
        unsigned int i;
        int bp, wp, s;

        for (bp = 0, i = 0; i < n; i++, bp += b)
        {
            wp = bp >> 5;
            s = 32 - b - (bp & 31);
            if (s >= 0)
                w[wp] |= (v[i] << s);
            else
            {
                s = -s;
                w[wp] |= (v[i] >> s);
                w[wp + 1] = (v[i] << (32 - s));
            }
        }
    }

    // Reads certain bits from the input buffer.
    // 'buf' is a pointer to the input buffer.
    // 'bp' is the "bit pointer" (offset in number of bits) indicating the current position in the input buffer.
    // 'b' is the number of bits to be read starting from the bit pointer.
    // Returns the number corresponding to the 'b' bits from the input buffer.
    unsigned int readBits(unsigned int* buf, unsigned int* bp, unsigned int b)
    {
        unsigned int bPtr;
        unsigned int w;
        unsigned int v;

        bPtr = (*bp) & 31;
        w = (32 - bPtr > b) ? b : (32 - bPtr);
        v = ((buf[(*bp) >> 5] >> bPtr) & MASK[w]);
        (*bp) += w;

        if (b == w)
            return v;

        v = v | ((buf[(*bp) >> 5] & MASK[b - w]) << w);
        (*bp) += (b - w);
        return v;
    }

    // Packs a number into certain bits and writes it to the output buffer.
    // 'buf' is a pointer to the output buffer.
    // 'bp' is the "bit pointer" (offset in number of bits) indicating the current position in the output buffer.
    // 'val' is the value of the number to be packed.
    // 'bits' is the bit width of the value to pack.
    void writeBits(unsigned int* buf, unsigned int* bp, unsigned int val, unsigned int bits)
    {
        unsigned int bPtr;
        unsigned int w;

        bPtr = (*bp) & 31;
        if (bPtr == 0)
            buf[(*bp) >> 5] = 0;

        w = (32 - bPtr > bits) ? bits : (32 - bPtr);
        buf[(*bp) >> 5] |= ((val & MASK[w]) << bPtr);
        (*bp) += w;

        if (bits - w > 0)
        {
            buf[(*bp) >> 5] = (val >> w) & MASK[bits - w];
            (*bp) += (bits - w);
        }
    }

    // Writes a single bit to the buffer at a bit position.
    // 'buf' is a pointer to the output buffer.
    // 'bp' is the "bit pointer" (offset in number of bits) into the output buffer at which to set the bit.
    // 'val' is the value to set the bit to.
    void setBit(unsigned char* buf, unsigned int* bp, unsigned int val)
    {
        unsigned int bPtr;
        bPtr = (*bp) & 7;
        if (bPtr == 0)
            buf[(*bp) >> 3] = 0;
        if (val == 1)
            buf[(*bp) >> 3] |= (1 << bPtr);
        (*bp)++;
    }

    unsigned MASK[33];
    pf unpack[17];
};

}}}

#endif /* CODING_H_ */
