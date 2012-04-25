/*-----------------------------------------------------------------------------
 *  VSEncodingNaive.cpp - A naive implementation of VSEncoding.
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

#include "util/compression/int/VSEncodingNaive.hpp"

#define VSENAIVE_LOGLEN         3
#define VSENAIVE_LOGLOG         3

#define VSENAIVE_LENS_LEN       (1 << VSENAIVE_LOGLEN)

static uint32_t __vsenaive_possLens[] = {
        1, 2, 4, 6, 8, 16, 32, 64
};

static uint32_t __vsenaive_possLogs[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 20, 32
};

static uint32_t __vsenaive_remapLogs[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 16, 16, 16,
        20, 20, 20, 20,
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
};

static uint32_t __vsenaive_codeLens[] = {
        0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 0, 0, 0, 0, 0, 5,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
};

static uint32_t __vsenaive_codeLogs[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 13, 13, 13, 14, 14, 14, 14,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

#ifdef USE_BOOST_SHAREDPTR
static VSEncodingPtr __vsenaive =
                VSEncodingPtr(new VSEncoding(&__vsenaive_possLens[0],
                NULL, VSENAIVE_LENS_LEN, false));
#else
static VSEncoding *__vsenaive =
                new VSEncoding(&__vsenaive_possLens[0],
                NULL, VSENAIVE_LENS_LEN, false);
#endif /* USE_BOOST_SHAREDPTR */

void
VSEncodingNaive::encodeArray(uint32_t *in,
                uint32_t len, uint32_t *out, uint32_t &nvalue)
{
        uint32_t        i;
        uint32_t        j;
        uint32_t        maxB;
        uint32_t        numBlocks;
        uint32_t        *logs;
        uint32_t        *part;
        BitsWriter      *wt;

        logs = new uint32_t[len];

        if (logs == NULL)
                eoutput("Can't allocate memory");

        /* Compute logs of all numbers */
        for (i = 0; i < len; i++)
                logs[i] = __vsenaive_remapLogs[1 + int_utils::get_msb(in[i])];

        /* Compute optimal partition */
        part = __vsenaive->compute_OptPartition(logs, len,
                        VSENAIVE_LOGLEN + VSENAIVE_LOGLOG, numBlocks);

    	/* Ready to write */
        wt = new BitsWriter(out);

        if (wt == NULL)
                eoutput("Can't initialize a class");

        for (i = 0; i < numBlocks; i++) {
                /* Compute max B in the block */
                for (j = part[i], maxB = 0; j < part[i + 1]; j++) {
                        if (maxB < logs[j])
                                maxB = logs[j];
                }

                /* Writes the value of B and K */
                wt->bit_writer(__vsenaive_codeLogs[maxB], VSENAIVE_LOGLOG);
                wt->bit_writer(__vsenaive_codeLens[part[i + 1] - part[i]], VSENAIVE_LOGLEN);

                for (j = part[i]; j < part[i + 1]; j++)
                        wt->bit_writer(in[j], maxB);
        }

        /* Align to 32-bit */
        wt->bit_flush();

        /* Finalization */
        delete[] part;
        delete[] logs;

        nvalue = wt->written;

        delete wt;
}

void
VSEncodingNaive::decodeArray(uint32_t *in,
                uint32_t len, uint32_t *out, uint32_t nvalue)
{
        uint32_t        i;
        uint32_t        B;
        uint32_t        K;
        uint32_t        *end;
        BitsReader      *rd;

        rd = new BitsReader(in);

        if (rd == NULL)
                eoutput("Can't initialize a class");

        end = out + nvalue;

        do {
                B = __vsenaive_possLogs[rd->bit_reader(VSENAIVE_LOGLOG)];
                K = __vsenaive_possLens[rd->bit_reader(VSENAIVE_LOGLEN)];

                for (i = 0; i < K; i++)
                        out[i] = (B != 0)? rd->bit_reader(B) : 0;

                out += K;
        } while (end > out);
}
