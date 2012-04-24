/*-----------------------------------------------------------------------------
 *  VSE-R.hpp - A encoder/decoder for a other version of VSEncoding.
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

#ifndef VSE_R_HPP
#define VSE_R_HPP

#include "util/compression/int/open_coders.hpp"
#include "util/compression/int/Delta.hpp"
#include "util/compression/int/VSEncoding.hpp"
#include "util/compression/int/VSEncodingNaive.hpp"
#include "util/compression/int/BitsReader.hpp"
#include "util/compression/int/BitsWriter.hpp"

class VSE_R {
        public:
                static void encodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t &nvalue);
                static void decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue);
};

#endif /* VSE_R_HPP */
