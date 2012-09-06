/*-----------------------------------------------------------------------------
 *  BitsReader.hpp - A coder interface to read compressed data
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

#ifndef IZENELIB_AM_SUCCINCT_LZEND_BITSREADER_HPP
#define IZENELIB_AM_SUCCINCT_LZEND_BITSREADER_HPP

#include <types.h>

NS_IZENELIB_AM_BEGIN
namespace succinct
{
namespace lzend
{

using namespace std;

class BitsReader
{
private:
    uint64_t buffer;
    uint32_t Fill;
    uint32_t *data;

public:
    BitsReader(uint32_t *in)
    {
        data = in;
        Fill = 32;
        buffer = *data++;
    }

    uint32_t bit_reader(uint32_t bits)
    {
        if (bits == 0)
            return 0;

        if (Fill < bits)
        {
            buffer = (buffer << 32) | *data++;
            Fill += 32;
        }

        Fill -= bits;

        return (buffer >> Fill) & ((1ULL << bits) - 1);
    }

    /* Gamma & Delta code */
    uint32_t N_Gamma()
    {
        uint32_t count = 0;
        while (bit_reader(1) == 0)
            count++;

        uint32_t d = ((1 << count) |
                      bit_reader(count));

        return d - 1;
    }

    uint32_t N_Delta()
    {
        uint32_t count = N_Gamma();
        return ((1 << count) |
                bit_reader(count)) - 1;
    }

    void N_DeltaArray(uint32_t *out,
                      uint32_t nvalues)
    {
        uint32_t i = 0;
        while (i < nvalues)
            out[i++] = N_Delta();
    }
};

}
}
NS_IZENELIB_AM_END

#endif /* __BITSREADER_HPP__ */
