/*-----------------------------------------------------------------------------
 *  BitsWriter.hpp - A coder interface to write compressed data
 *      Copied from open_coder/include/io - some codes modified.
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

#ifndef IZENELIB_AM_SUCCINCT_BITSWRITER_HPP
#define IZENELIB_AM_SUCCINCT_BITSWRITER_HPP

#include <types.h>

NS_IZENELIB_AM_BEGIN
namespace succinct
{
namespace lzend
{

using namespace std;

#define __log2_uint32(_arg1)\
        ({\
            uint32_t d;\
            __asm__("bsr %1, %0;" :"=r"(d) :"r"(_arg1));\
            d;\
        })

class BitsWriter
{
private:
    uint64_t buffer;
    uint32_t Fill;
    uint32_t *data;
    uint32_t written;

    int get_msb(uint32_t v)
    {
        return (v != 0)?
               __log2_uint32(v) : 0;
    }

public:
    BitsWriter(uint32_t *out)
    {
        data = out;
        Fill = 0;
        buffer = 0;
        written = 0;
    }

    void bit_flush()
    {
        if (Fill == 0)
            return;

        if (Fill > 32)
        {
            buffer <<= 64 - Fill;
            *data++ = buffer >> 32;
            *data++ = buffer &
                      ((1ULL << 32) - 1);
            written += 2;
            Fill = 0;
        }

        if (Fill > 0)
        {
            *data++ = buffer <<
                      (32 - Fill) &
                      ((1ULL << 32) - 1);
            written++;
        }

        buffer = 0;
        Fill = 0;
    }

    void bit_writer(uint32_t value,
                    uint32_t bits)
    {
        if (bits == 0)
            return;

        buffer = (buffer << bits) |
                 (value & ((1ULL << bits) - 1));

        Fill += bits;

        if (Fill >= 32)
        {
            *data++ = (buffer >>
                       (Fill - 32)) &
                      ((1ULL << 32) - 1);
            written++;
            Fill -= 32;
        }
    }

    /* Gamma & Delta codes */
    void N_Unary(int num)
    {
        bit_writer(0, num);
        bit_writer(1, 1);
    }

    void N_Gamma(uint32_t val)
    {
        int d = get_msb(++val);
        N_Unary(d);
        bit_writer(val, d);
    }

    uint32_t N_DeltaArray(uint32_t *in,
                          uint32_t len)
    {
        for (uint32_t i = 0; i < len; i++)
        {
            int d = get_msb(in[i] + 1);
            N_Gamma(d);
            bit_writer(in[i] + 1, d);
        }

        bit_flush();

        return written;
    }
};

}
}
NS_IZENELIB_AM_END


#endif
