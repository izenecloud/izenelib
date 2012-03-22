/*-----------------------------------------------------------------------------
 *  OPTPForDelta.cpp - A optimized implementation of PForDelta.
 *      This implementation made by these authors based on a paper below:
 *       - http://dl.acm.org/citation.cfm?id=1526764
 *      And, some potions fo this code are optimized by means of a code given
 *      by Shuai Ding, who is of original authors proposing OPTPForDelta.
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

#include "util/compression/int/OPTPForDelta.hpp"

static uint32_t __optp4delta_possLogs[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 20, 32
};

uint32_t
OPTPForDelta::tryB(uint32_t b, uint32_t *in, uint32_t len)
{
        uint32_t        i;
        uint32_t        size;
        uint32_t        curExcept;
        uint32_t        *exceptionsPositions;
        uint32_t        *exceptionsValues;
        uint32_t        *exceptions;
        uint32_t        *encodedExceptions;
        uint32_t        encodedExceptions_sz;
        uint32_t        excPos;
        uint32_t        excVal;
        uint32_t        e;

        __assert(b <= 32);

        if (b == 32) {
                return len;
        } else {
                /* FIXME: To be modified */
                exceptionsPositions = new uint32_t[len];
                exceptionsValues = new uint32_t[len];
                exceptions = new uint32_t[2 * len];
                encodedExceptions = new uint32_t[2 * len + 2];

                if (exceptionsPositions == NULL || exceptionsValues == NULL ||
                                exceptions == NULL || encodedExceptions == NULL)
                        eoutput("Can't allocate memory");

                size = int_utils::div_roundup(len * b, 32);
                curExcept = 0;

                for (i = 0; i < len; i++) {
                        if (in[i] >= (1U << b)) {
                                e = in[i] >> b;
                                exceptionsPositions[curExcept] = i;
                                exceptionsValues[curExcept] = e;
                                curExcept++;
                        }
                }

                if (curExcept > 0) {
                        uint32_t        cur;
                        uint32_t        prev;
                        uint32_t        gap;

                        for (i = curExcept - 1; i > 0; i--) {
                                cur = exceptionsPositions[i];
                                prev = exceptionsPositions[i - 1];
                                gap = cur - prev;

                                exceptionsPositions[i] = gap;
                        }

                        for (i = 0;  i < curExcept; i++) {
                                excPos = (i > 0)? exceptionsPositions[i] - 1 :
                                        exceptionsPositions[i];
                                excVal = exceptionsValues[i] - 1;

                                exceptions[i] = excPos;
                                exceptions[i + curExcept] = excVal;
                        }

                        Simple16::encodeArray(exceptions, 2 * curExcept,
                                        encodedExceptions, encodedExceptions_sz);
                        size += encodedExceptions_sz;
                }
        }

        /* Finalization */
        delete[] exceptions;
        delete[] exceptionsPositions;
        delete[] exceptionsValues;
        delete[] encodedExceptions;

        return size;
}

uint32_t
OPTPForDelta::findBestB(uint32_t *in, uint32_t len)
{
        uint32_t        i;
        uint32_t        b;
        uint32_t        bsize;
        uint32_t        csize;

        b = __optp4delta_possLogs[__array_size(__optp4delta_possLogs) - 1];

        for (i = 0, bsize = len;
                        i < __array_size(__optp4delta_possLogs) - 1; i++) {
                csize = OPTPForDelta::tryB(__optp4delta_possLogs[i], in, len);

                if (csize <= bsize) {
                        b = __optp4delta_possLogs[i];
                        bsize = csize;
                }
        }

        return b;
}

void
OPTPForDelta::encodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t &nvalue)
{
        uint32_t        i;
        uint32_t        numBlocks;
        uint32_t        csize;

        numBlocks = int_utils::div_roundup(len, PFORDELTA_BLOCKSZ);

        /* Output the number of blocks */
        *out++ = numBlocks;
        nvalue = 1;

        for (i = 0; i < numBlocks; i++) {
                if (i != numBlocks - 1) {
                        PForDelta::encodeBlock(in, PFORDELTA_BLOCKSZ,
                                        out, csize, OPTPForDelta::findBestB);
                        in += PFORDELTA_BLOCKSZ;
                        out += csize;
                } else {
                        if ((len % PFORDELTA_BLOCKSZ) != 0)
                                PForDelta::encodeBlock(in, len % PFORDELTA_BLOCKSZ,
                                                out, csize, OPTPForDelta::findBestB);
                        else
                                PForDelta::encodeBlock(in, PFORDELTA_BLOCKSZ,
                                                out, csize, OPTPForDelta::findBestB);
                }

                nvalue += csize;
        }
}

void
OPTPForDelta::decodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t nvalue)
{
        PForDelta::decodeArray(in, len, out, nvalue);
}
