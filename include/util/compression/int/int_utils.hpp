/*-----------------------------------------------------------------------------
 *  int_utils.hpp - Utilities used by encoders/decodes
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

#ifndef INT_UTILS_HPP
#define INT_UTILS_HPP

#include "util/compression/int/open_coders.hpp"

#define __log2_uint32(_arg1)            \
        ({                              \
                uint32_t        d;      \
                __asm__("bsr %1, %0;" :"=r"(d) :"r"(_arg1));    \
                d;                      \
        })

#define __array_size(x)         (sizeof(x) / sizeof(x[0]))

class int_utils {
        public:
                static int get_msb(uint32_t v);
                static uint32_t div_roundup(uint32_t v, uint32_t div);
                static double get_time(void);
                static uint32_t *open_and_mmap_file(char *filen,
                                bool write, uint64_t &len);
                static void close_file(uint32_t *adr, uint64_t len);
};

#endif  /* INT_UTILS_HPP */
