/*-----------------------------------------------------------------------------
 *  VSEncodingNaive.hpp - A naive implementation of VSEncoding.
 *      This implementation is used by VSE-R.
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

#ifndef VSENCODINGNAIVE_HPP
#define VSENCODINGNAIVE_HPP

#include "util/compression/int/open_coders.hpp"
#include "util/compression/int/VSEncoding.hpp"
#include "util/compression/int/BitsReader.hpp"
#include "util/compression/int/BitsWriter.hpp"

class VSEncodingNaive {
        public:
                static void encodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t &nvalue);
                static void decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue);
};

#endif /* VSENCODINGNAIVE_HPP */
