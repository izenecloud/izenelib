/*-----------------------------------------------------------------------------
 *  VSEncoding.cpp - To scan the optimal partitions in a list.
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

#ifndef VSENCODING_CPP
#define VSENCODING_CPP

#include "util/compression/int/VSEncoding.hpp"

VSEncoding::VSEncoding(uint32_t *lens, uint32_t *zlens, uint32_t size, bool cflag)
{
        possLens = lens;
        posszLens = zlens;
        poss_sz = size;
        aligned = cflag;

        /* Set the max length of sequences */
        maxBlk = possLens[poss_sz - 1];

        if (posszLens != NULL &&
                        maxBlk < posszLens[poss_sz - 1])
                maxBlk = posszLens[poss_sz - 1];
}

uint32_t *
VSEncoding::compute_OptPartition(uint32_t *seq,
                uint32_t len, uint32_t fixCost, uint32_t &pSize)
{
        int             *SSSP;
        uint32_t        i;
        uint32_t        maxB;
        uint32_t        *part;
        uint64_t        curCost;
        uint64_t        *cost;

        /* It will store the shortest path */
        SSSP = new int[len + 1];

        /* cost[i] will contain the cost of encoding up to i-th position */
        cost = new uint64_t[len + 1];

        if (SSSP == NULL || cost == NULL)
                eoutput("Can't allocate memory");

        for (i = 0; i <= len; i++) {
                SSSP[i] = -1;
                cost[i] = 0;
        }

        /*
         * This loop computes the cost of the optimal partition.
         * The computation of the max log in each block is done
         * by scanning. Probably we could obtain a faster solution
         * by using RMQ data structures. We use this trivial
         * solution since construction time is not our main concern.
         */
        {
                int     mleft;
                int     j;
                int     g;
                int     l;

                for (i = 1; i <= len; i++) {
                        mleft = ((int)(i - maxBlk) > 0)? i - maxBlk : 0;

                        for (maxB = 0, l = 0, g = 0, j = i - 1; j >= mleft; j--) {
                                if (maxB < seq[j])
                                        maxB = seq[j];

                                if (posszLens == NULL) {
                                        /*
                                         * FIXME: If the gaps of elements in possLens[] are
                                         * sparse, a process below is more efficient to hop
                                         * these gaps using the elements rather than
                                         * decrementing j.
                                         */
                                        if (i - j != possLens[l])
                                                continue;
                                        else
                                                l++;
                                } else {
                                        /*
                                         * Treat runs of 0 in a different way.
                                         * They could form larger blocks!
                                         */
                                        if (maxB != 0) {
                                                mleft = ((int)(i - maxBlk) > 0)?
                                                        i - possLens[poss_sz - 1] : 0;

                                                if (i - j != possLens[l])
                                                        continue;

                                                if (i - j == possLens[l])
                                                        l++;
                                        } else {
                                                if (i - j == possLens[l])
                                                        l++;

                                                if (i - j != posszLens[g])
                                                        continue;

                                                if (i - j == posszLens[g])
                                                        g++;
                                        }
                                }

                                /* Caluculate costs */
                                if (aligned)
                                        curCost = cost[j] + int_utils::div_roundup((i - j) * maxB, 32) + fixCost;
                                else
                                        curCost = cost[j] + (i - j) * maxB + fixCost;

                                if (SSSP[i] == -1)
                                        cost[i] = curCost + 1;

                                if (curCost <= cost[i]) {
                                        cost[i] = curCost;
                                        SSSP[i] = j;
                                }
                        }
                }
        }

        /* Compute number of nodes in the path */
        {
                int     next;

                pSize = 0;
                next = len;

                while (next != 0) {
                        next = SSSP[next];
                        pSize++;
                }

                /*
                 * Obtain the optimal partition starting
                 * from the last block.
                 */
                part = new uint32_t[pSize + 1];

                if (part == NULL)
                        eoutput("Can't allocate memory");

                i = pSize;
                next = len;

                while (next != 0) {
                        part[i--] = next;
                        next = SSSP[next];
                }

                part[0] = 0;
        }

        /* Finalization */
        delete[] SSSP;
        delete[] cost;

        return part;
}

#endif /* VSENCODING_CPP */
