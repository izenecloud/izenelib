/*-----------------------------------------------------------------------------
 *  Simple16.hpp - A encoder/decoder for Simple16.
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

#ifndef SIMPLE16_HPP
#define SIMPLE16_HPP

#include "util/compression/int/open_coders.hpp"
#include "util/compression/int/BitsWriter.hpp"

class Simple16 {
        private:
                /*
                 * These functions judging how many constant-size
                 * integers to include in a single 32-bit area
                 * based on Simple9 coders.
                 */
                static bool try28_1bit (uint32_t *n, uint32_t nvalue);
                static bool try14_2bit (uint32_t *n, uint32_t nvalue);
                static bool try7_4bit (uint32_t *n, uint32_t nvalue);
                static bool try4_7bit (uint32_t *n, uint32_t nvalue);
                static bool try2_14bit (uint32_t *n, uint32_t nvalue);

                 /*
                 * Simple16 appends more functions to deal with
                 * variable-size integers in as single 32-bit areas.
                 */
                static bool try7_2bit_14_1bit(uint32_t *n, uint32_t nvalue);
                static bool try14_1bit_7_2bit(uint32_t *n, uint32_t nvalue);
                static bool try1_4bit_8_3bit(uint32_t *n, uint32_t nvalue);
                static bool try4_5bit_2_4bit(uint32_t *n, uint32_t nvalue);
                static bool try2_4bit_4_5bit(uint32_t *n, uint32_t nvalue);
                static bool try3_6bit_2_5bit(uint32_t *n, uint32_t nvalue);
                static bool try2_5bit_3_6bit(uint32_t *n, uint32_t nvalue);
                static bool try1_10bit_2_9bit(uint32_t *n, uint32_t nvalue);

                static bool try7_1bit_7_2bit_7_1bit(uint32_t *n, uint32_t nvalue);
                static bool try1_3bit_4_4bit_3_3bit(uint32_t *n, uint32_t nvalue);

        public:
                static void encodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t &nvalue);
                static void decodeArray(uint32_t *in, uint32_t len,
                                uint32_t *out, uint32_t nvalue);
};

#endif /* SIMPLE16_HPP */
