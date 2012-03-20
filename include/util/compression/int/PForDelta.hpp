/*-----------------------------------------------------------------------------
 *  PForDelta.hpp - A encoder/decoder for PForDelta.
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

#ifndef PFORDELTA_HPP
#define PFORDELTA_HPP

#include "util/compression/int/open_coders.hpp"
#include "util/compression/int/Simple16.hpp"
#include "util/compression/int/BitsWriter.hpp"

#define PFORDELTA_NBLOCK        1
#define PFORDELTA_BLOCKSZ       (32 * PFORDELTA_NBLOCK)

/* FIXME: There is a bug with 128 of PFORDELTA_BLOCKSZ */
//#define PFORDELTA_BLOCKSZ       (128 * PFORDELTA_NBLOCK)

class PForDelta {
        public:
                static uint32_t tryB(uint32_t b, uint32_t *in,
                                uint32_t len);
                static uint32_t findBestB(uint32_t *in, uint32_t len);

                /* A last argument for OPTPForDelta */
                static void encodeBlock(uint32_t *in,
                                uint32_t len, uint32_t *out,
                                uint32_t &nvalue,
                                uint32_t (*find)(uint32_t *in, uint32_t len));

                static void encodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t &nvalue);
                static void decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue);
};

#endif /* PFORDELTA_HPP */
