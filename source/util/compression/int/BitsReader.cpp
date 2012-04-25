/*-----------------------------------------------------------------------------
 *  BitsReader.h - A code to read compressed data for decoding.
 *
 *  Coding-Style:
 *      emacs) Mode: C, tab-width: 8, c-basic-offset: 8, indent-tabs-mode: nil
 *      vi) tabstop: 8, expandtab
 *
 *  Authors:
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *      Fabrizio Silvestri <fabrizio.silvestri_at_isti.cnr.it>
 *      Rossano Venturini <rossano.venturini_at_isti.cnr.it>
 *-----------------------------------------------------------------------------
 */

#include "util/compression/int/BitsReader.hpp"

#define BITSRD_M64      0x000000000000ffff
#define BITSRD_M32      0x0000ffff

#define BITSRD_BUFFILL()        \
        if (Fill < 16) {        \
                buffer = (buffer << 32) | *data;        \
                data++;         \
                Fill += 32;     \
        }


BitsReader::BitsReader(uint32_t *in)
{
        data = in;
        Fill = 32;
        buffer = *data++;
}

uint32_t
BitsReader::bit_reader(uint32_t bits)
{
        __assert(bits <= 32);

        if (bits == 0)
                return 0;

        if (Fill < bits) {
                buffer = (buffer << 32) | *data++;
                Fill += 32;
        }

        Fill -= bits;

        return (buffer >> Fill) & ((1ULL << bits) - 1);
}

uint32_t
BitsReader::N_Unary()
{
        uint32_t        count;

        count = 0;
        while (bit_reader(1) == 0)
                count++;

        return count;
}

void
BitsReader::N_UnaryArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;
        uint32_t        count;

        i = 0;
        count = 0;

        while (i < nvalues) {
                while (bit_reader(1) == 0)
                        count++;

                out[i++] = count;
                count = 0;
        }
}

void
BitsReader::F_UnaryArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = F_Unary();
}

/*
* The F_Unary decoding uses decUn:2exp16 Unary
* indicized table, from decUnary.h
*/
uint32_t
BitsReader::F_Unary()
{
        return BitsReader::F_Unary32();
}

uint32_t
BitsReader::F_Unary32()
{
        uint32_t        dec;

        BITSRD_BUFFILL();

        dec = decUnary[(buffer >> (Fill - 16)) & BITSRD_M64];

        if (dec == 16) {
                Fill -= 16;
                return F_Unary() + dec;
        } else {
                Fill -= dec + 1;
                return dec;
        }

        /* Not reach here */
        return 0;
}

uint32_t
BitsReader::F_Unary16()
{
        uint32_t        dec;

        BITSRD_BUFFILL();

        dec = decUnary[(buffer >> (Fill - 16)) & BITSRD_M64];
        Fill -= dec + 1;

        return dec;
}

uint32_t
BitsReader::N_Gamma()
{
        uint32_t        d;
        uint32_t        count;

        count = 0;

        while (bit_reader(1) == 0)
                count++;

        d = ((1 << count) | bit_reader(count));

        return d - 1;
}

/*
 * The F_Gamma decoding uses decUn:2exp16 Gamma
 * indicized table, from decGamma.h
 */
uint32_t
BitsReader::F_Gamma()
{
        uint32_t        dec;

        BITSRD_BUFFILL();

        dec = decGamma[(buffer >> (Fill - 16)) & BITSRD_M64];

        if (dec == 0) {
                return N_Gamma();
        } else {
                Fill -= (dec >> 16);
                return (dec & BITSRD_M32) - 1;
        }
}

uint32_t
BitsReader::FU_Gamma()
{
        uint32_t        count;

        count = F_Unary32();

        return ((1 << count) | bit_reader(count)) - 1;
}

void
BitsReader::N_GammaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = N_Gamma();
}

void
BitsReader::F_GammaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = F_Gamma();
}

void
BitsReader::FU_GammaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = FU_Gamma();
}

uint32_t
BitsReader::N_Delta()
{
        uint32_t        count;

        count = N_Gamma();

        return ((1 << count) | bit_reader(count)) - 1;
}

/*
 * The Delta decoding uses decUn:2exp16 Delta
 * indicized table, from decDelta.h
 */
uint32_t
BitsReader::F_Delta()
{
        uint32_t        dec;

        BITSRD_BUFFILL();

        dec = decDelta[(buffer >> (Fill - 16)) & BITSRD_M64];

        if (dec == 0) {
                return N_Delta();
        } else {
                Fill -= (dec >> 16);
                return (dec & BITSRD_M32) - 1;
        }
}

uint32_t
BitsReader::FU_Delta()
{
        uint32_t        count;
        uint32_t        log;

        count = F_Unary16();
        log = ((1 << count) | bit_reader(count)) - 1;

        return ((1 << log) | bit_reader(log)) - 1;
}

void
BitsReader::N_DeltaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = N_Delta();
}

void
BitsReader::FU_DeltaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = FU_Delta();
}

void
BitsReader::FG_DeltaArray(uint32_t *out, uint32_t nvalues)
{
        uint32_t        i;
        uint32_t        count;

        i = 0;

        while (i < nvalues) {
                count = F_Gamma();
                out[i++] = ((1 << count) | bit_reader(count)) - 1;
        }
}

void
BitsReader::F_DeltaArray(uint32_t* out, uint32_t nvalues)
{
        uint32_t        i;

        i = 0;

        while (i < nvalues)
                out[i++] = F_Delta();
}

/*
* readMinmalBinary()
*      requirements: b >= 1
*/
uint32_t
BitsReader::readMinimalBinary(uint32_t b)
{
        uint32_t        m;
        uint32_t        x;
        uint32_t        d;

        __assert(data != NULL);

        d = int_utils::get_msb(b);
        m = (1ULL << (d + 1)) - b;

        x = bit_reader(d);

        if (x < m)
                return x;
        else
                return (x << 1) + bit_reader(1) - m;
}

void
BitsReader::InterpolativeArray(uint32_t* out, uint32_t nvalues,
                uint32_t offset, uint32_t lo, uint32_t hi)
{
        uint32_t        h;
        uint32_t        m;

        __assert(lo <= hi);

        if (nvalues == 0)
                return;

        if (nvalues == 1) {
                if (out != NULL)
                        out[offset] = readMinimalBinary(hi - lo + 1) + lo;
                else
                        readMinimalBinary(hi - lo + 1);

                return;
        }

        h = nvalues / 2;
        m = readMinimalBinary(hi - nvalues + h + 1 - (lo + h) + 1) + lo + h;

        if (out != NULL)
                out[offset + h] = m;

        InterpolativeArray(out, h, offset, lo, m - 1);
        InterpolativeArray(out, nvalues - h - 1, offset + h + 1, m + 1, hi);
}
