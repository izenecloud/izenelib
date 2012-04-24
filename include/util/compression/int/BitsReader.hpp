/*-----------------------------------------------------------------------------
 *  BitsReader.hpp - A coder interface to read compressed data.
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

#ifndef BITSREADER_HPP
#define BITSREADER_HPP

#include <iostream>
#include <stdint.h>

#include "util/compression/int/open_coders.hpp"
/* A transformation table for fast decoding */
#include "util/compression/int/decUnary.hpp"
#include "util/compression/int/decGamma.hpp"
#include "util/compression/int/decDelta.hpp"

using namespace std;

class BitsReader {
        private:
                uint64_t        buffer;
                uint32_t        Fill;
                uint32_t        *data;

        public:
                BitsReader(uint32_t *in);

                uint32_t bit_reader(uint32_t bits);

                /* Unary code */
                void N_UnaryArray(uint32_t *out, uint32_t nvalues);
                void F_UnaryArray(uint32_t *out, uint32_t nvalues);

                uint32_t N_Unary();
                uint32_t F_Unary();
                uint32_t F_Unary32();
                uint32_t F_Unary16();

                /* Gamma code */
                void N_GammaArray(uint32_t *out, uint32_t nvalues);
                void F_GammaArray(uint32_t *out, uint32_t nvalues);
                void FU_GammaArray(uint32_t *out, uint32_t nvalues);

                uint32_t N_Gamma();
                uint32_t F_Gamma();
                uint32_t FU_Gamma();

                /* Delta code */
                void N_DeltaArray(uint32_t *out, uint32_t nvalues);
                void FU_DeltaArray(uint32_t *out, uint32_t nvalues);
                void FG_DeltaArray(uint32_t *out, uint32_t nvalues);
                void F_DeltaArray(uint32_t* out, uint32_t nvalues);

                uint32_t N_Delta();
                uint32_t F_Delta();
                uint32_t FU_Delta();

                /* Binary Interpolative code */
                void InterpolativeArray(uint32_t* out, uint32_t nvalues,
                                uint32_t offset, uint32_t lo, uint32_t hi);

                uint32_t readMinimalBinary(uint32_t b);
};

#endif /* BITSREADER_HPP */
