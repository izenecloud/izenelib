/*-----------------------------------------------------------------------------
 *  BitsWriter.hpp - A coder interface to write compressed data.
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

#ifndef BITSWRITER_HPP
#define BITSWRITER_HPP

#include "util/compression/int/open_coders.hpp"

using namespace std;

class BitsWriter {
        private:
                uint64_t        buffer;
                uint32_t        Fill;
                uint32_t        *data;

        public:
                uint32_t        written;

                BitsWriter(uint32_t *out);

                void bit_flush();
                void bit_writer(uint32_t value, uint32_t bits);

                uint32_t *ret_pos();

                /* For Unary codes */
                void N_Unary(int num);
                uint32_t N_UnaryArray(uint32_t *in, uint32_t len);

                /* For Delta codes */
                void N_Gamma(uint32_t val);
                uint32_t N_GammaArray(uint32_t *in, uint32_t len);

                /* For Delta codes */
                uint32_t N_DeltaArray(uint32_t *in, uint32_t len);

                /* For Binary Interpolative codes */
                void writeMinimalBinary(uint32_t x, uint32_t b);
                void InterpolativeArray(uint32_t *in, uint32_t len,
                                uint32_t offset, uint32_t lo, uint32_t hi);
};

#endif /* BITS_WRITER_HPP */
