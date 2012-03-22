/*-----------------------------------------------------------------------------
 *  BitsWriter.h - A code to write compressed data during encoding.
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

#include "util/compression/int/BitsWriter.hpp"

BitsWriter::BitsWriter(uint32_t *out)
{
        data = out;
        Fill = 0;
        buffer = 0;
        written = 0;
}

void
BitsWriter::bit_flush()
{
        if (Fill == 0)
                return;

        if (Fill > 32) {
                buffer <<= 64 - Fill;
                *data++ = buffer >> 32;
                *data++ = buffer & ((1ULL << 32) - 1);
                written += 2;
                Fill = 0;
        }

        if (Fill > 0) {
                *data++ = buffer << (32 - Fill) & ((1ULL << 32) - 1);
                written++;
        }

        buffer = 0;
        Fill = 0;
}

void
BitsWriter::bit_writer(uint32_t value, uint32_t bits)
{
        if (bits == 0)
                return;

        buffer = (buffer << bits) | (value & ((1ULL << bits) - 1));

        Fill += bits;

        if (Fill >= 32) {
                *data++ = (buffer >> (Fill - 32)) & ((1ULL << 32) - 1);
                written++;
                Fill -= 32;
        }
}

uint32_t *
BitsWriter::ret_pos()
{
        return data;
}

void
BitsWriter::N_Unary(int num)
{
        bit_writer(0, num);
        bit_writer(1, 1);
}

uint32_t
BitsWriter::N_UnaryArray(uint32_t *in, uint32_t len)
{
        uint32_t        i;

        for (i = 0; i < len; i++) {
                bit_writer(0, in[i]);
                bit_writer(1, 1);
        }

        bit_flush();

        return written;
}

void
BitsWriter::N_Gamma(uint32_t val)
{
        int d;

        d = int_utils::get_msb(++val);

        N_Unary(d);
        bit_writer(val, d);
}

uint32_t
BitsWriter::N_GammaArray(uint32_t *in, uint32_t len)
{
        int             d;
        uint32_t        i;

        for (i = 0; i < len; i++) {
                d = int_utils::get_msb(in[i] + 1);
                N_Unary(d);
                bit_writer(in[i] + 1, d);
        }

        bit_flush();

        return written;
}

uint32_t
BitsWriter::N_DeltaArray(uint32_t *in, uint32_t len)
{
        int             d;
        uint32_t        i;

        for (i = 0; i < len; i++) {
                d = int_utils::get_msb(in[i] + 1);
                N_Gamma(d);
                bit_writer(in[i] + 1, d);
        }

        bit_flush();

        return written;
}

void
BitsWriter::writeMinimalBinary(uint32_t x, uint32_t b)
{
        int             d;
        uint32_t        m;

        __assert(data != NULL);

        d = int_utils::get_msb(b);
        m = (1ULL << (d + 1)) - b;

        if (x < m)
                bit_writer(x, d);
        else
                bit_writer(x + m, d + 1);
}

void
BitsWriter::InterpolativeArray(uint32_t *in, uint32_t len,
                uint32_t offset, uint32_t lo, uint32_t hi)
{
        uint32_t        h;
        uint32_t        m;

        __assert(lo <= hi);

        if (len == 0)
                return;

        if (len == 1) {
                writeMinimalBinary(in[offset] - lo, hi - lo + 1);
                return;
        }

        h = len / 2;
        m = in[offset + h];

        writeMinimalBinary(m - (lo + h), hi - len + h + 1 - (lo + h) + 1);

        InterpolativeArray(in, h, offset, lo, m - 1);
        InterpolativeArray(in, len - h - 1, offset + h + 1, m + 1, hi);
}
