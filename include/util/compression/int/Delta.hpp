/*-----------------------------------------------------------------------------
 *  Delta.hpp - A encoder/decoder for Delta.
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

#ifndef DELTA_HPP
#define DELTA_HPP

#include "util/compression/int/open_coders.hpp"
#include "util/compression/int/BitsWriter.hpp"
#include "util/compression/int/BitsReader.hpp"

class Delta {
        public:
                static void encodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t &nvalue) {
                        BitsWriter *wt = new BitsWriter(out);
                        nvalue = wt->N_DeltaArray(in, len);
                        delete wt;
                }

                static void decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue) {
                        BitsReader *rd = new BitsReader(in);
                        rd->N_DeltaArray(out, nvalue);
                        delete rd;
                }

                static void FU_decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue) {
                        BitsReader *rd = new BitsReader(in);
                        rd->FU_DeltaArray(out, nvalue);
                        delete rd;
                }

                static void FG_decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue) {
                        BitsReader *rd = new BitsReader(in);
                        rd->FG_DeltaArray(out, nvalue);
                        delete rd;
                }

                static void F_decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue) {
                        BitsReader *rd = new BitsReader(in);
                        rd->F_DeltaArray(out, nvalue);
                        delete rd;
                }
};

#endif /* DELTA_HPP */
