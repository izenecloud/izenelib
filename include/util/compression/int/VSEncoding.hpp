/*-----------------------------------------------------------------------------
 *  VSEncoding.hpp - To scan the optimal partitions in a list.
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

#ifndef VSENCODING_HPP
#define VSENCODING_HPP

#include "util/compression/int/open_coders.hpp"

/*
 * If a list is larger that this parameter, then
 * it is split into subblocks that are compressed
 * separatelly. Compression is slightly worse, but
 * the decompression is more cache-friendly.
 */
#define VSENCODING_BLOCKSZ      65536U

class VSEncoding {
        private:
                /*
                 * In some cases, there might be asymmetry between possible
                 * lenghts of blocks if they are formed by zeros(posszLens)
                 * or larger numbers(possLens).
                 */
                bool            aligned;
                uint32_t        *possLens;
                uint32_t        *posszLens;
                uint32_t        poss_sz;
                uint32_t        maxBlk;

        public:
                VSEncoding(uint32_t *lens, uint32_t *zlens, uint32_t size, bool cflag);

                /*
                 * Compute the optimal sub-lists from lists.
                 *      len: The length of the sequence of lists
                 *      fixCost: The fix cost in bits that we pay for  each block
                 */
                uint32_t *compute_OptPartition(uint32_t *seq,
                                uint32_t len, uint32_t fixCost, uint32_t &pSize);
};

#ifdef USE_BOOST_SHAREDPTR
typedef boost::shared_ptr<VSEncoding>   VSEncodingPtr;
#endif /* USE_BOOST_SHAREDPTR */

#endif /* VSENCODING_HPP */
