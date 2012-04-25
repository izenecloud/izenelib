/*-----------------------------------------------------------------------------
 *  VSE-R.cpp - A original implementation of VSEncoding.
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

#include "util/compression/int/VSE-R.hpp"

#define VSER_LOGS_LEN   32

/* A set of unpacking functions */
static void __vser_unpack1(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack2(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack3(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack4(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack5(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack6(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack7(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack8(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack9(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack10(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack11(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack12(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack13(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack14(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack15(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack16(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack17(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack18(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack19(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack20(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack21(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack22(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack23(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack24(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack25(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack26(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack27(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack28(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack29(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack30(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack31(uint32_t *out, uint32_t *in, uint32_t bs);
static void __vser_unpack32(uint32_t *out, uint32_t *in, uint32_t bs);

/* A interface of unpacking functions above */
typedef void (*__vser_unpacker)(uint32_t *out, uint32_t *in, uint32_t bs);

static __vser_unpacker       __vser_unpack[] = {
        __vser_unpack1, __vser_unpack2,
        __vser_unpack3, __vser_unpack4,
        __vser_unpack5, __vser_unpack6,
        __vser_unpack7, __vser_unpack8,
        __vser_unpack9, __vser_unpack10,
        __vser_unpack11, __vser_unpack12,
        __vser_unpack13, __vser_unpack14,
        __vser_unpack15, __vser_unpack16,
        __vser_unpack17, __vser_unpack18,
        __vser_unpack19, __vser_unpack20,
        __vser_unpack21, __vser_unpack22,
        __vser_unpack23, __vser_unpack24,
        __vser_unpack25, __vser_unpack26,
        __vser_unpack27, __vser_unpack28,
        __vser_unpack29, __vser_unpack30,
        __vser_unpack31, __vser_unpack32
};

void
VSE_R::encodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t &nvalue)
{
        uint32_t        i;
        uint32_t        maxL;
        uint32_t        csize;
        uint32_t        *logs;
        uint32_t        *p;
        uint32_t        hist[VSER_LOGS_LEN + 1];
        BitsWriter      *wt[VSER_LOGS_LEN + 1];

        logs = new uint32_t[len];

        if (logs == NULL)
                eoutput("Can't allocate memory");

        /* Compute logs of all numbers */
        for (i = 0; i < len; i++) {
                if (in[i] != 0)
                        logs[i] = int_utils::get_msb(in[i] + 1);
                else
                        logs[i] = 0;
        }

        VSEncodingNaive::encodeArray(logs, len, out + 1, csize);

        /* Output the information of a 1st section */
        *out = csize;
        out += csize + 1;
        nvalue = csize + 1;

        /*
         * hist[i] stores the number of occs of number whose
         * log is equal to i.
         */
        for (i = 0; i <= VSER_LOGS_LEN; i++)
                hist[i] = 0;

        /* Count the number of occs */
        for (i = 0; i < len; i++) {
                if (logs[i] != 0)
                        hist[logs[i]]++;
        }

        for (i = 0, maxL = 0; i <= VSER_LOGS_LEN; i++) {
                if (hist[i] != 0)
                        maxL = i;
        }

        /* Write the number of occs resorting to Delta code */
        hist[0] = maxL;
        Delta::encodeArray(hist, maxL + 1, out + 1, csize);

        /* Output the information of a 2nd section */
        *out = csize;
        out += csize + 1;
        nvalue += csize + 1;

        /* Ready to write each integer */
        for (i = 1, csize = 0, p = out; i <= maxL; i++) {
                if (hist[i] != 0) {
                        wt[i] = new BitsWriter(p);
                        csize += int_utils::div_roundup(i * hist[i], 32);
                        p += int_utils::div_roundup(i * hist[i], 32);
                }
        }

        nvalue += csize;

        /* Write the number in blocks depending on their logs */
        for (i = 0; i < len; i++) {
                if (logs[i] != 0)
                        wt[logs[i]]->bit_writer(in[i] + 1, logs[i]);
        }

        for (i = 1; i <= maxL; i++) {
                if (hist[i] != 0) {
                        wt[i]->bit_flush();
                        delete wt[i];
                }
        }

        delete[] logs;
}

void
VSE_R::decodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t nvalue)
{
        uint32_t        i;
        uint32_t        n;
        uint32_t        nlen;
        uint32_t        maxL;
        uint32_t        *ins;
        uint32_t        *outs;
        uint32_t        *pblk[VSER_LOGS_LEN + 1];
        BitsReader      *rd;

        outs = new uint32_t[nvalue + TAIL_MERGIN];

        if (outs == NULL)
                eoutput("Can't allocate memory");

        rd = new BitsReader(in + *in + 2);

        if (rd == NULL)
                eoutput("Can't initialize a class");

        VSEncodingNaive::decodeArray(in + 1, nvalue, out, nvalue);

        /* *in stores the length of the Delta encoded block */
        in += *in + 1;
        ins = in + *in + 1;

        for (i = 1, nlen = 0, pblk[0] = outs,
                        maxL = rd->F_Delta(); i <= maxL; i++) {
                n = rd->F_Delta();

                if (n != 0) {
                        pblk[i] = &outs[nlen];
                        (__vser_unpack[i - 1])(&outs[nlen], ins,
                                        int_utils::div_roundup(n * i, 32));
                        ins += int_utils::div_roundup(n * i, 32);
                        nlen +=  n;
                }
        }

        for (i = 0; i < nvalue; i++)
                out[i] = (out[i] == 0)? 0 : *(pblk[out[i]])++ - 1;

        delete[] outs;
}

/* --- Intra functions below --- */

void
__vser_unpack1(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 1, out += 32, in += 1) {
                out[0] = (1 << 1) | ((in[0] >> 31));
                out[1] = (1 << 1) | ((in[0] >> 30) & 0x01);
                out[2] = (1 << 1) | ((in[0] >> 29) & 0x01);
                out[3] = (1 << 1) | ((in[0] >> 28) & 0x01);
                out[4] = (1 << 1) | ((in[0] >> 27) & 0x01);
                out[5] = (1 << 1) | ((in[0] >> 26) & 0x01);
                out[6] = (1 << 1) | ((in[0] >> 25) & 0x01);
                out[7] = (1 << 1) | ((in[0] >> 24) & 0x01);
                out[8] = (1 << 1) | ((in[0] >> 23) & 0x01);
                out[9] = (1 << 1) | ((in[0] >> 22) & 0x01);
                out[10] = (1 << 1) | ((in[0] >> 21) & 0x01);
                out[11] = (1 << 1) | ((in[0] >> 20) & 0x01);
                out[12] = (1 << 1) | ((in[0] >> 19) & 0x01);
                out[13] = (1 << 1) | ((in[0] >> 18) & 0x01);
                out[14] = (1 << 1) | ((in[0] >> 17) & 0x01);
                out[15] = (1 << 1) | ((in[0] >> 16) & 0x01);
                out[16] = (1 << 1) | ((in[0] >> 15) & 0x01);
                out[17] = (1 << 1) | ((in[0] >> 14) & 0x01);
                out[18] = (1 << 1) | ((in[0] >> 13) & 0x01);
                out[19] = (1 << 1) | ((in[0] >> 12) & 0x01);
                out[20] = (1 << 1) | ((in[0] >> 11) & 0x01);
                out[21] = (1 << 1) | ((in[0] >> 10) & 0x01);
                out[22] = (1 << 1) | ((in[0] >> 9) & 0x01);
                out[23] = (1 << 1) | ((in[0] >> 8) & 0x01);
                out[24] = (1 << 1) | ((in[0] >> 7) & 0x01);
                out[25] = (1 << 1) | ((in[0] >> 6) & 0x01);
                out[26] = (1 << 1) | ((in[0] >> 5) & 0x01);
                out[27] = (1 << 1) | ((in[0] >> 4) & 0x01);
                out[28] = (1 << 1) | ((in[0] >> 3) & 0x01);
                out[29] = (1 << 1) | ((in[0] >> 2) & 0x01);
                out[30] = (1 << 1) | ((in[0] >> 1) & 0x01);
                out[31] = (1 << 1) | (in[0] & 0x01);
        }
}

void
__vser_unpack2(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 2, out += 32, in += 2) {
                out[0] = (1 << 2) | ((in[0] >> 30));
                out[1] = (1 << 2) | ((in[0] >> 28) & 0x03);
                out[2] = (1 << 2) | ((in[0] >> 26) & 0x03);
                out[3] = (1 << 2) | ((in[0] >> 24) & 0x03);
                out[4] = (1 << 2) | ((in[0] >> 22) & 0x03);
                out[5] = (1 << 2) | ((in[0] >> 20) & 0x03);
                out[6] = (1 << 2) | ((in[0] >> 18) & 0x03);
                out[7] = (1 << 2) | ((in[0] >> 16) & 0x03);
                out[8] = (1 << 2) | ((in[0] >> 14) & 0x03);
                out[9] = (1 << 2) | ((in[0] >> 12) & 0x03);
                out[10] = (1 << 2) | ((in[0] >> 10) & 0x03);
                out[11] = (1 << 2) | ((in[0] >> 8) & 0x03);
                out[12] = (1 << 2) | ((in[0] >> 6) & 0x03);
                out[13] = (1 << 2) | ((in[0] >> 4) & 0x03);
                out[14] = (1 << 2) | ((in[0] >> 2) & 0x03);
                out[15] = (1 << 2) | (in[0] & 0x03);
                out[16] = (1 << 2) | ((in[1] >> 30));
                out[17] = (1 << 2) | ((in[1] >> 28) & 0x03);
                out[18] = (1 << 2) | ((in[1] >> 26) & 0x03);
                out[19] = (1 << 2) | ((in[1] >> 24) & 0x03);
                out[20] = (1 << 2) | ((in[1] >> 22) & 0x03);
                out[21] = (1 << 2) | ((in[1] >> 20) & 0x03);
                out[22] = (1 << 2) | ((in[1] >> 18) & 0x03);
                out[23] = (1 << 2) | ((in[1] >> 16) & 0x03);
                out[24] = (1 << 2) | ((in[1] >> 14) & 0x03);
                out[25] = (1 << 2) | ((in[1] >> 12) & 0x03);
                out[26] = (1 << 2) | ((in[1] >> 10) & 0x03);
                out[27] = (1 << 2) | ((in[1] >> 8) & 0x03);
                out[28] = (1 << 2) | ((in[1] >> 6) & 0x03);
                out[29] = (1 << 2) | ((in[1] >> 4) & 0x03);
                out[30] = (1 << 2) | ((in[1] >> 2) & 0x03);
                out[31] = (1 << 2) | (in[1] & 0x03);
        }
}

void
__vser_unpack3(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 3, out += 32, in += 3) {
                out[0] = (1 << 3) | ((in[0] >> 29));
                out[1] = (1 << 3) | ((in[0] >> 26) & 0x07);
                out[2] = (1 << 3) | ((in[0] >> 23) & 0x07);
                out[3] = (1 << 3) | ((in[0] >> 20) & 0x07);
                out[4] = (1 << 3) | ((in[0] >> 17) & 0x07);
                out[5] = (1 << 3) | ((in[0] >> 14) & 0x07);
                out[6] = (1 << 3) | ((in[0] >> 11) & 0x07);
                out[7] = (1 << 3) | ((in[0] >> 8) & 0x07);
                out[8] = (1 << 3) | ((in[0] >> 5) & 0x07);
                out[9] = (1 << 3) | ((in[0] >> 2) & 0x07);
                out[10] = (in[0] << 1) & 0x07;
                out[10] |= (1 << 3) | ((in[1] >> 31));
                out[11] = (1 << 3) | ((in[1] >> 28) & 0x07);
                out[12] = (1 << 3) | ((in[1] >> 25) & 0x07);
                out[13] = (1 << 3) | ((in[1] >> 22) & 0x07);
                out[14] = (1 << 3) | ((in[1] >> 19) & 0x07);
                out[15] = (1 << 3) | ((in[1] >> 16) & 0x07);
                out[16] = (1 << 3) | ((in[1] >> 13) & 0x07);
                out[17] = (1 << 3) | ((in[1] >> 10) & 0x07);
                out[18] = (1 << 3) | ((in[1] >> 7) & 0x07);
                out[19] = (1 << 3) | ((in[1] >> 4) & 0x07);
                out[20] = (1 << 3) | ((in[1] >> 1) & 0x07);
                out[21] = (in[1] << 2) & 0x07;
                out[21] |= (1 << 3) | ((in[2] >> 30));
                out[22] = (1 << 3) | ((in[2] >> 27) & 0x07);
                out[23] = (1 << 3) | ((in[2] >> 24) & 0x07);
                out[24] = (1 << 3) | ((in[2] >> 21) & 0x07);
                out[25] = (1 << 3) | ((in[2] >> 18) & 0x07);
                out[26] = (1 << 3) | ((in[2] >> 15) & 0x07);
                out[27] = (1 << 3) | ((in[2] >> 12) & 0x07);
                out[28] = (1 << 3) | ((in[2] >> 9) & 0x07);
                out[29] = (1 << 3) | ((in[2] >> 6) & 0x07);
                out[30] = (1 << 3) | ((in[2] >> 3) & 0x07);
                out[31] = (1 << 3) | (in[2] & 0x07);
        }
}

void
__vser_unpack4(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 4, out += 32, in += 4) {
                out[0] = (1 << 4) | ((in[0] >> 28));
                out[1] = (1 << 4) | ((in[0] >> 24) & 0x0f);
                out[2] = (1 << 4) | ((in[0] >> 20) & 0x0f);
                out[3] = (1 << 4) | ((in[0] >> 16) & 0x0f);
                out[4] = (1 << 4) | ((in[0] >> 12) & 0x0f);
                out[5] = (1 << 4) | ((in[0] >> 8) & 0x0f);
                out[6] = (1 << 4) | ((in[0] >> 4) & 0x0f);
                out[7] = (1 << 4) | (in[0] & 0x0f);
                out[8] = (1 << 4) | ((in[1] >> 28) & 0x0f);
                out[9] = (1 << 4) | ((in[1] >> 24) & 0x0f);
                out[10] = (1 << 4) | ((in[1] >> 20) & 0x0f);
                out[11] = (1 << 4) | ((in[1] >> 16) & 0x0f);
                out[12] = (1 << 4) | ((in[1] >> 12) & 0x0f);
                out[13] = (1 << 4) | ((in[1] >> 8) & 0x0f);
                out[14] = (1 << 4) | ((in[1] >> 4) & 0x0f);
                out[15] = (1 << 4) | (in[1] & 0x0f);
                out[16] = (1 << 4) | ((in[2] >> 28) & 0x0f);
                out[17] = (1 << 4) | ((in[2] >> 24) & 0x0f);
                out[18] = (1 << 4) | ((in[2] >> 20) & 0x0f);
                out[19] = (1 << 4) | ((in[2] >> 16) & 0x0f);
                out[20] = (1 << 4) | ((in[2] >> 12) & 0x0f);
                out[21] = (1 << 4) | ((in[2] >> 8) & 0x0f);
                out[22] = (1 << 4) | ((in[2] >> 4) & 0x0f);
                out[23] = (1 << 4) | (in[2] & 0x0f);
                out[24] = (1 << 4) | ((in[3] >> 28) & 0x0f);
                out[25] = (1 << 4) | ((in[3] >> 24) & 0x0f);
                out[26] = (1 << 4) | ((in[3] >> 20) & 0x0f);
                out[27] = (1 << 4) | ((in[3] >> 16) & 0x0f);
                out[28] = (1 << 4) | ((in[3] >> 12) & 0x0f);
                out[29] = (1 << 4) | ((in[3] >> 8) & 0x0f);
                out[30] = (1 << 4) | ((in[3] >> 4) & 0x0f);
                out[31] = (1 << 4) | (in[3] & 0x0f);
        }
}

void
__vser_unpack5(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 5, out += 32, in += 5) {
                out[0] = (1 << 5) | ((in[0] >> 27));
                out[1] = (1 << 5) | ((in[0] >> 22) & 0x1f);
                out[2] = (1 << 5) | ((in[0] >> 17) & 0x1f);
                out[3] = (1 << 5) | ((in[0] >> 12) & 0x1f);
                out[4] = (1 << 5) | ((in[0] >> 7) & 0x1f);
                out[5] = (1 << 5) | ((in[0] >> 2) & 0x1f);
                out[6] = (in[0] << 3) & 0x1f;
                out[6] |=  (1 << 5) | ((in[1] >> 29));
                out[7] = (1 << 5) | ((in[1] >> 24) & 0x1f);
                out[8] = (1 << 5) | ((in[1] >> 19) & 0x1f);
                out[9] = (1 << 5) | ((in[1] >> 14) & 0x1f);
                out[10] = (1 << 5) | ((in[1] >> 9) & 0x1f);
                out[11] = (1 << 5) | ((in[1] >> 4) & 0x1f);
                out[12] = (in[1] << 1) & 0x1f;
                out[12] |=  (1 << 5) | ((in[2] >> 31));
                out[13] = (1 << 5) | ((in[2] >> 26) & 0x1f);
                out[14] = (1 << 5) | ((in[2] >> 21) & 0x1f);
                out[15] = (1 << 5) | ((in[2] >> 16) & 0x1f);
                out[16] = (1 << 5) | ((in[2] >> 11) & 0x1f);
                out[17] = (1 << 5) | ((in[2] >> 6) & 0x1f);
                out[18] = (1 << 5) | ((in[2] >> 1) & 0x1f);
                out[19] = (in[2] << 4) & 0x1f;
                out[19] |=  (1 << 5) | ((in[3] >> 28));
                out[20] = (1 << 5) | ((in[3] >> 23) & 0x1f);
                out[21] = (1 << 5) | ((in[3] >> 18) & 0x1f);
                out[22] = (1 << 5) | ((in[3] >> 13) & 0x1f);
                out[23] = (1 << 5) | ((in[3] >> 8) & 0x1f);
                out[24] = (1 << 5) | ((in[3] >> 3) & 0x1f);
                out[25] = (in[3] << 2) & 0x1f;
                out[25] |=  (1 << 5) | ((in[4] >> 30));
                out[26] = (1 << 5) | ((in[4] >> 25) & 0x1f);
                out[27] = (1 << 5) | ((in[4] >> 20) & 0x1f);
                out[28] = (1 << 5) | ((in[4] >> 15) & 0x1f);
                out[29] = (1 << 5) | ((in[4] >> 10) & 0x1f);
                out[30] = (1 << 5) | ((in[4] >> 5) & 0x1f);
                out[31] = (1 << 5) | (in[4] & 0x1f);
        }
}

void
__vser_unpack6(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 6, out += 32, in += 6) {
                out[0] = (1 << 6) | ((in[0] >> 26));
                out[1] = (1 << 6) | ((in[0] >> 20) & 0x3f);
                out[2] = (1 << 6) | ((in[0] >> 14) & 0x3f);
                out[3] = (1 << 6) | ((in[0] >> 8) & 0x3f);
                out[4] = (1 << 6) | ((in[0] >> 2) & 0x3f);
                out[5] = (in[0] << 4) & 0x3f;
                out[5] |=  (1 << 6) | ((in[1] >> 28));
                out[6] = (1 << 6) | ((in[1] >> 22) & 0x3f);
                out[7] = (1 << 6) | ((in[1] >> 16) & 0x3f);
                out[8] = (1 << 6) | ((in[1] >> 10) & 0x3f);
                out[9] = (1 << 6) | ((in[1] >> 4) & 0x3f);
                out[10] = (in[1] << 2) & 0x3f;
                out[10] |=  (1 << 6) | ((in[2] >> 30));
                out[11] = (1 << 6) | ((in[2] >> 24) & 0x3f);
                out[12] = (1 << 6) | ((in[2] >> 18) & 0x3f);
                out[13] = (1 << 6) | ((in[2] >> 12) & 0x3f);
                out[14] = (1 << 6) | ((in[2] >> 6) & 0x3f);
                out[15] = (1 << 6) | (in[2] & 0x3f);
                out[16] = (1 << 6) | ((in[3] >> 26));
                out[17] = (1 << 6) | ((in[3] >> 20) & 0x3f);
                out[18] = (1 << 6) | ((in[3] >> 14) & 0x3f);
                out[19] = (1 << 6) | ((in[3] >> 8) & 0x3f);
                out[20] = (1 << 6) | ((in[3] >> 2) & 0x3f);
                out[21] = (in[3] << 4) & 0x3f;
                out[21] |=  (1 << 6) | ((in[4] >> 28));
                out[22] = (1 << 6) | ((in[4] >> 22) & 0x3f);
                out[23] = (1 << 6) | ((in[4] >> 16) & 0x3f);
                out[24] = (1 << 6) | ((in[4] >> 10) & 0x3f);
                out[25] = (1 << 6) | ((in[4] >> 4) & 0x3f);
                out[26] = (in[4] << 2) & 0x3f;
                out[26] |=  (1 << 6) | ((in[5] >> 30));
                out[27] = (1 << 6) | ((in[5] >> 24) & 0x3f);
                out[28] = (1 << 6) | ((in[5] >> 18) & 0x3f);
                out[29] = (1 << 6) | ((in[5] >> 12) & 0x3f);
                out[30] = (1 << 6) | ((in[5] >> 6) & 0x3f);
                out[31] = (1 << 6) | (in[5] & 0x3f);
        }
}

void
__vser_unpack7(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 7, out += 32, in += 7) {
                out[0] = (1 << 7) | ((in[0] >> 25));
                out[1] = (1 << 7) | ((in[0] >> 18) & 0x7f);
                out[2] = (1 << 7) | ((in[0] >> 11) & 0x7f);
                out[3] = (1 << 7) | ((in[0] >> 4) & 0x7f);
                out[4] = (in[0] << 3) & 0x7f;
                out[4] |=  (1 << 7) | ((in[1] >> 29));
                out[5] = (1 << 7) | ((in[1] >> 22) & 0x7f);
                out[6] = (1 << 7) | ((in[1] >> 15) & 0x7f);
                out[7] = (1 << 7) | ((in[1] >> 8) & 0x7f);
                out[8] = (1 << 7) | ((in[1] >> 1) & 0x7f);
                out[9] = (in[1] << 6) & 0x7f;
                out[9] |=  (1 << 7) | ((in[2] >> 26));
                out[10] = (1 << 7) | ((in[2] >> 19) & 0x7f);
                out[11] = (1 << 7) | ((in[2] >> 12) & 0x7f);
                out[12] = (1 << 7) | ((in[2] >> 5) & 0x7f);
                out[13] = (in[2] << 2) & 0x7f;
                out[13] |=  (1 << 7) | ((in[3] >> 30));
                out[14] = (1 << 7) | ((in[3] >> 23) & 0x7f);
                out[15] = (1 << 7) | ((in[3] >> 16) & 0x7f);
                out[16] = (1 << 7) | ((in[3] >> 9) & 0x7f);
                out[17] = (1 << 7) | ((in[3] >> 2) & 0x7f);
                out[18] = (in[3] << 5) & 0x7f;
                out[18] |=  (1 << 7) | ((in[4] >> 27));
                out[19] = (1 << 7) | ((in[4] >> 20) & 0x7f);
                out[20] = (1 << 7) | ((in[4] >> 13) & 0x7f);
                out[21] = (1 << 7) | ((in[4] >> 6) & 0x7f);
                out[22] = (in[4] << 1) & 0x7f;
                out[22] |=  (1 << 7) | ((in[5] >> 31));
                out[23] = (1 << 7) | ((in[5] >> 24) & 0x7f);
                out[24] = (1 << 7) | ((in[5] >> 17) & 0x7f);
                out[25] = (1 << 7) | ((in[5] >> 10) & 0x7f);
                out[26] = (1 << 7) | ((in[5] >> 3) & 0x7f);
                out[27] = (in[5] << 4) & 0x7f;
                out[27] |=  (1 << 7) | ((in[6] >> 28));
                out[28] = (1 << 7) | ((in[6] >> 21) & 0x7f);
                out[29] = (1 << 7) | ((in[6] >> 14) & 0x7f);
                out[30] = (1 << 7) | ((in[6] >> 7) & 0x7f);
                out[31] = (1 << 7) | (in[6] & 0x7f);
        }
}

void
__vser_unpack8(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 8, out += 32, in += 8) {
                out[0] = (1 << 8) | ((in[0] >> 24));
                out[1] = (1 << 8) | ((in[0] >> 16) & 0xff);
                out[2] = (1 << 8) | ((in[0] >> 8) & 0xff);
                out[3] = (1 << 8) | (in[0] & 0xff);
                out[4] = (1 << 8) | ((in[1] >> 24));
                out[5] = (1 << 8) | ((in[1] >> 16) & 0xff);
                out[6] = (1 << 8) | ((in[1] >> 8) & 0xff);
                out[7] = (1 << 8) | (in[1] & 0xff);
                out[8] = (1 << 8) | ((in[2] >> 24));
                out[9] = (1 << 8) | ((in[2] >> 16) & 0xff);
                out[10] = (1 << 8) | ((in[2] >> 8) & 0xff);
                out[11] = (1 << 8) | (in[2] & 0xff);
                out[12] = (1 << 8) | ((in[3] >> 24));
                out[13] = (1 << 8) | ((in[3] >> 16) & 0xff);
                out[14] = (1 << 8) | ((in[3] >> 8) & 0xff);
                out[15] = (1 << 8) | (in[3] & 0xff);
                out[16] = (1 << 8) | ((in[4] >> 24));
                out[17] = (1 << 8) | ((in[4] >> 16) & 0xff);
                out[18] = (1 << 8) | ((in[4] >> 8) & 0xff);
                out[19] = (1 << 8) | (in[4] & 0xff);
                out[20] = (1 << 8) | ((in[5] >> 24));
                out[21] = (1 << 8) | ((in[5] >> 16) & 0xff);
                out[22] = (1 << 8) | ((in[5] >> 8) & 0xff);
                out[23] = (1 << 8) | (in[5] & 0xff);
                out[24] = (1 << 8) | ((in[6] >> 24));
                out[25] = (1 << 8) | ((in[6] >> 16) & 0xff);
                out[26] = (1 << 8) | ((in[6] >> 8) & 0xff);
                out[27] = (1 << 8) | (in[6] & 0xff);
                out[28] = (1 << 8) | ((in[7] >> 24));
                out[29] = (1 << 8) | ((in[7] >> 16) & 0xff);
                out[30] = (1 << 8) | ((in[7] >> 8) & 0xff);
                out[31] = (1 << 8) | (in[7] & 0xff);
        }
}

void
__vser_unpack9(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;
        for (i = 0; i < bs; i += 9, out += 32, in += 9) {
                out[0] = (1 << 9) | ((in[0] >> 23));
                out[1] = (1 << 9) | ((in[0] >> 14) & 0x01ff);
                out[2] = (1 << 9) | ((in[0] >> 5) & 0x01ff);
                out[3] = (in[0] << 4) & 0x01ff;
                out[3] |=  (1 << 9) | ((in[1] >> 28));
                out[4] = (1 << 9) | ((in[1] >> 19) & 0x01ff);
                out[5] = (1 << 9) | ((in[1] >> 10) & 0x01ff);
                out[6] = (1 << 9) | ((in[1] >> 1) & 0x01ff);
                out[7] = (in[1] << 8) & 0x01ff;
                out[7] |=  (1 << 9) | ((in[2] >> 24));
                out[8] = (1 << 9) | ((in[2] >> 15) & 0x01ff);
                out[9] = (1 << 9) | ((in[2] >> 6) & 0x01ff);
                out[10] = (in[2] << 3) & 0x01ff;
                out[10] |=  (1 << 9) | ((in[3] >> 29));
                out[11] = (1 << 9) | ((in[3] >> 20) & 0x01ff);
                out[12] = (1 << 9) | ((in[3] >> 11) & 0x01ff);
                out[13] = (1 << 9) | ((in[3] >> 2) & 0x01ff);
                out[14] = (in[3] << 7) & 0x01ff;
                out[14] |=  (1 << 9) | ((in[4] >> 25));
                out[15] = (1 << 9) | ((in[4] >> 16) & 0x01ff);
                out[16] = (1 << 9) | ((in[4] >> 7) & 0x01ff);
                out[17] = (in[4] << 2) & 0x01ff;
                out[17] |=  (1 << 9) | ((in[5] >> 30));
                out[18] = (1 << 9) | ((in[5] >> 21) & 0x01ff);
                out[19] = (1 << 9) | ((in[5] >> 12) & 0x01ff);
                out[20] = (1 << 9) | ((in[5] >> 3) & 0x01ff);
                out[21] = (in[5] << 6) & 0x01ff;
                out[21] |=  (1 << 9) | ((in[6] >> 26));
                out[22] = (1 << 9) | ((in[6] >> 17) & 0x01ff);
                out[23] = (1 << 9) | ((in[6] >> 8) & 0x01ff);
                out[24] = (in[6] << 1) & 0x01ff;
                out[24] |=  (1 << 9) | ((in[7] >> 31));
                out[25] = (1 << 9) | ((in[7] >> 22) & 0x01ff);
                out[26] = (1 << 9) | ((in[7] >> 13) & 0x01ff);
                out[27] = (1 << 9) | ((in[7] >> 4) & 0x01ff);
                out[28] = (in[7] << 5) & 0x01ff;
                out[28] |=  (1 << 9) | ((in[8] >> 27));
                out[29] = (1 << 9) | ((in[8] >> 18) & 0x01ff);
                out[30] = (1 << 9) | ((in[8] >> 9) & 0x01ff);
                out[31] = (1 << 9) | (in[8] & 0x01ff);
        }
}

void
__vser_unpack10(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 10, out += 32, in += 10) {
                out[0] = (1 << 10) | ((in[0] >> 22));
                out[1] = (1 << 10) | ((in[0] >> 12) & 0x03ff);
                out[2] = (1 << 10) | ((in[0] >> 2) & 0x03ff);
                out[3] = (in[0] << 8) & 0x03ff;
                out[3] |=  (1 << 10) | ((in[1] >> 24));
                out[4] = (1 << 10) | ((in[1] >> 14) & 0x03ff);
                out[5] = (1 << 10) | ((in[1] >> 4) & 0x03ff);
                out[6] = (in[1] << 6) & 0x03ff;
                out[6] |=  (1 << 10) | ((in[2] >> 26));
                out[7] = (1 << 10) | ((in[2] >> 16) & 0x03ff);
                out[8] = (1 << 10) | ((in[2] >> 6) & 0x03ff);
                out[9] = (in[2] << 4) & 0x03ff;
                out[9] |=  (1 << 10) | ((in[3] >> 28));
                out[10] = (1 << 10) | ((in[3] >> 18) & 0x03ff);
                out[11] = (1 << 10) | ((in[3] >> 8) & 0x03ff);
                out[12] = (in[3] << 2) & 0x03ff;
                out[12] |=  (1 << 10) | ((in[4] >> 30));
                out[13] = (1 << 10) | ((in[4] >> 20) & 0x03ff);
                out[14] = (1 << 10) | ((in[4] >> 10) & 0x03ff);
                out[15] = (1 << 10) | (in[4] & 0x03ff);
                out[16] = (1 << 10) | ((in[5] >> 22));
                out[17] = (1 << 10) | ((in[5] >> 12) & 0x03ff);
                out[18] = (1 << 10) | ((in[5] >> 2) & 0x03ff);
                out[19] = (in[5] << 8) & 0x03ff;
                out[19] |=  (1 << 10) | ((in[6] >> 24));
                out[20] = (1 << 10) | ((in[6] >> 14) & 0x03ff);
                out[21] = (1 << 10) | ((in[6] >> 4) & 0x03ff);
                out[22] = (in[6] << 6) & 0x03ff;
                out[22] |=  (1 << 10) | ((in[7] >> 26));
                out[23] = (1 << 10) | ((in[7] >> 16) & 0x03ff);
                out[24] = (1 << 10) | ((in[7] >> 6) & 0x03ff);
                out[25] = (in[7] << 4) & 0x03ff;
                out[25] |=  (1 << 10) | ((in[8] >> 28));
                out[26] = (1 << 10) | ((in[8] >> 18) & 0x03ff);
                out[27] = (1 << 10) | ((in[8] >> 8) & 0x03ff);
                out[28] = (in[8] << 2) & 0x03ff;
                out[28] |=  (1 << 10) | ((in[9] >> 30));
                out[29] = (1 << 10) | ((in[9] >> 20) & 0x03ff);
                out[30] = (1 << 10) | ((in[9] >> 10) & 0x03ff);
                out[31] = (1 << 10) | (in[9] & 0x03ff);
        }
}

void
__vser_unpack11(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 11, out += 32, in += 11) {
                out[0] = (1 << 11) | ((in[0] >> 21));
                out[1] = (1 << 11) | ((in[0] >> 10) & 0x07ff);
                out[2] = (in[0] << 1) & 0x07ff;
                out[2] |=  (1 << 11) | ((in[1] >> 31));
                out[3] = (1 << 11) | ((in[1] >> 20) & 0x07ff);
                out[4] = (1 << 11) | ((in[1] >> 9) & 0x07ff);
                out[5] = (in[1] << 2) & 0x07ff;
                out[5] |=  (1 << 11) | ((in[2] >> 30));
                out[6] = (1 << 11) | ((in[2] >> 19) & 0x07ff);
                out[7] = (1 << 11) | ((in[2] >> 8) & 0x07ff);
                out[8] = (in[2] << 3) & 0x07ff;
                out[8] |=  (1 << 11) | ((in[3] >> 29));
                out[9] = (1 << 11) | ((in[3] >> 18) & 0x07ff);
                out[10] = (1 << 11) | ((in[3] >> 7) & 0x07ff);
                out[11] = (in[3] << 4) & 0x07ff;
                out[11] |=  (1 << 11) | ((in[4] >> 28));
                out[12] = (1 << 11) | ((in[4] >> 17) & 0x07ff);
                out[13] = (1 << 11) | ((in[4] >> 6) & 0x07ff);
                out[14] = (in[4] << 5) & 0x07ff;
                out[14] |=  (1 << 11) | ((in[5] >> 27));
                out[15] = (1 << 11) | ((in[5] >> 16) & 0x07ff);
                out[16] = (1 << 11) | ((in[5] >> 5) & 0x07ff);
                out[17] = (in[5] << 6) & 0x07ff;
                out[17] |=  (1 << 11) | ((in[6] >> 26));
                out[18] = (1 << 11) | ((in[6] >> 15) & 0x07ff);
                out[19] = (1 << 11) | ((in[6] >> 4) & 0x07ff);
                out[20] = (in[6] << 7) & 0x07ff;
                out[20] |=  (1 << 11) | ((in[7] >> 25));
                out[21] = (1 << 11) | ((in[7] >> 14) & 0x07ff);
                out[22] = (1 << 11) | ((in[7] >> 3) & 0x07ff);
                out[23] = (in[7] << 8) & 0x07ff;
                out[23] |=  (1 << 11) | ((in[8] >> 24));
                out[24] = (1 << 11) | ((in[8] >> 13) & 0x07ff);
                out[25] = (1 << 11) | ((in[8] >> 2) & 0x07ff);
                out[26] = (in[8] << 9) & 0x07ff;
                out[26] |=  (1 << 11) | ((in[9] >> 23));
                out[27] = (1 << 11) | ((in[9] >> 12) & 0x07ff);
                out[28] = (1 << 11) | ((in[9] >> 1) & 0x07ff);
                out[29] = (in[9] << 10) & 0x07ff;
                out[29] |=  (1 << 11) | ((in[10] >> 22));
                out[30] = (1 << 11) | ((in[10] >> 11) & 0x07ff);
                out[31] = (1 << 11) | (in[10] & 0x07ff);
        }
}

void
__vser_unpack12(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 12, out += 32, in += 12) {
                out[0] = (1 << 12) | ((in[0] >> 20));
                out[1] = (1 << 12) | ((in[0] >> 8) & 0x0fff);
                out[2] = (in[0] << 4) & 0x0fff;
                out[2] |=  (1 << 12) | ((in[1] >> 28));
                out[3] = (1 << 12) | ((in[1] >> 16) & 0x0fff);
                out[4] = (1 << 12) | ((in[1] >> 4) & 0x0fff);
                out[5] = (in[1] << 8) & 0x0fff;
                out[5] |=  (1 << 12) | ((in[2] >> 24));
                out[6] = (1 << 12) | ((in[2] >> 12) & 0x0fff);
                out[7] = (1 << 12) | (in[2] & 0x0fff);
                out[8] = (1 << 12) | ((in[3] >> 20));
                out[9] = (1 << 12) | ((in[3] >> 8) & 0x0fff);
                out[10] = (in[3] << 4) & 0x0fff;
                out[10] |=  (1 << 12) | ((in[4] >> 28));
                out[11] = (1 << 12) | ((in[4] >> 16) & 0x0fff);
                out[12] = (1 << 12) | ((in[4] >> 4) & 0x0fff);
                out[13] = (in[4] << 8) & 0x0fff;
                out[13] |=  (1 << 12) | ((in[5] >> 24));
                out[14] = (1 << 12) | ((in[5] >> 12) & 0x0fff);
                out[15] = (1 << 12) | (in[5] & 0x0fff);
                out[16] = (1 << 12) | ((in[6] >> 20));
                out[17] = (1 << 12) | ((in[6] >> 8) & 0x0fff);
                out[18] = (in[6] << 4) & 0x0fff;
                out[18] |=  (1 << 12) | ((in[7] >> 28));
                out[19] = (1 << 12) | ((in[7] >> 16) & 0x0fff);
                out[20] = (1 << 12) | ((in[7] >> 4) & 0x0fff);
                out[21] = (in[7] << 8) & 0x0fff;
                out[21] |=  (1 << 12) | ((in[8] >> 24));
                out[22] = (1 << 12) | ((in[8] >> 12) & 0x0fff);
                out[23] = (1 << 12) | (in[8] & 0x0fff);
                out[24] = (1 << 12) | ((in[9] >> 20));
                out[25] = (1 << 12) | ((in[9] >> 8) & 0x0fff);
                out[26] = (in[9] << 4) & 0x0fff;
                out[26] |=  (1 << 12) | ((in[10] >> 28));
                out[27] = (1 << 12) | ((in[10] >> 16) & 0x0fff);
                out[28] = (1 << 12) | ((in[10] >> 4) & 0x0fff);
                out[29] = (in[10] << 8) & 0x0fff;
                out[29] |=  (1 << 12) | ((in[11] >> 24));
                out[30] = (1 << 12) | ((in[11] >> 12) & 0x0fff);
                out[31] = (1 << 12) | (in[11] & 0x0fff);
        }
}

void
__vser_unpack13(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 13, out += 32, in += 13) {
                out[0] = (1 << 13) | ((in[0] >> 19));
                out[1] = (1 << 13) | ((in[0] >> 6) & 0x1fff);
                out[2] = (in[0] << 7) & 0x1fff;
                out[2] |=  (1 << 13) | ((in[1] >> 25));
                out[3] = (1 << 13) | ((in[1] >> 12) & 0x1fff);
                out[4] = (in[1] << 1) & 0x1fff;
                out[4] |=  (1 << 13) | ((in[2] >> 31));
                out[5] = (1 << 13) | ((in[2] >> 18) & 0x1fff);
                out[6] = (1 << 13) | ((in[2] >> 5) & 0x1fff);
                out[7] = (in[2] << 8) & 0x1fff;
                out[7] |=  (1 << 13) | ((in[3] >> 24));
                out[8] = (1 << 13) | ((in[3] >> 11) & 0x1fff);
                out[9] = (in[3] << 2) & 0x1fff;
                out[9] |=  (1 << 13) | ((in[4] >> 30));
                out[10] = (1 << 13) | ((in[4] >> 17) & 0x1fff);
                out[11] = (1 << 13) | ((in[4] >> 4) & 0x1fff);
                out[12] = (in[4] << 9) & 0x1fff;
                out[12] |=  (1 << 13) | ((in[5] >> 23));
                out[13] = (1 << 13) | ((in[5] >> 10) & 0x1fff);
                out[14] = (in[5] << 3) & 0x1fff;
                out[14] |=  (1 << 13) | ((in[6] >> 29));
                out[15] = (1 << 13) | ((in[6] >> 16) & 0x1fff);
                out[16] = (1 << 13) | ((in[6] >> 3) & 0x1fff);
                out[17] = (in[6] << 10) & 0x1fff;
                out[17] |=  (1 << 13) | ((in[7] >> 22));
                out[18] = (1 << 13) | ((in[7] >> 9) & 0x1fff);
                out[19] = (in[7] << 4) & 0x1fff;
                out[19] |=  (1 << 13) | ((in[8] >> 28));
                out[20] = (1 << 13) | ((in[8] >> 15) & 0x1fff);
                out[21] = (1 << 13) | ((in[8] >> 2) & 0x1fff);
                out[22] = (in[8] << 11) & 0x1fff;
                out[22] |=  (1 << 13) | ((in[9] >> 21));
                out[23] = (1 << 13) | ((in[9] >> 8) & 0x1fff);
                out[24] = (in[9] << 5) & 0x1fff;
                out[24] |=  (1 << 13) | ((in[10] >> 27));
                out[25] = (1 << 13) | ((in[10] >> 14) & 0x1fff);
                out[26] = (1 << 13) | ((in[10] >> 1) & 0x1fff);
                out[27] = (in[10] << 12) & 0x1fff;
                out[27] |=  (1 << 13) | ((in[11] >> 20));
                out[28] = (1 << 13) | ((in[11] >> 7) & 0x1fff);
                out[29] = (in[11] << 6) & 0x1fff;
                out[29] |=  (1 << 13) | ((in[12] >> 26));
                out[30] = (1 << 13) | ((in[12] >> 13) & 0x1fff);
                out[31] = (1 << 13) | (in[12] & 0x1fff);
        }
}

void
__vser_unpack14(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 14, out += 32, in += 14) {
                out[0] = (1 << 14) | ((in[0] >> 18));
                out[1] = (1 << 14) | ((in[0] >> 4) & 0x3fff);
                out[2] = (in[0] << 10) & 0x3fff;
                out[2] |=  (1 << 14) | ((in[1] >> 22));
                out[3] = (1 << 14) | ((in[1] >> 8) & 0x3fff);
                out[4] = (in[1] << 6) & 0x3fff;
                out[4] |=  (1 << 14) | ((in[2] >> 26));
                out[5] = (1 << 14) | ((in[2] >> 12) & 0x3fff);
                out[6] = (in[2] << 2) & 0x3fff;
                out[6] |=  (1 << 14) | ((in[3] >> 30));
                out[7] = (1 << 14) | ((in[3] >> 16) & 0x3fff);
                out[8] = (1 << 14) | ((in[3] >> 2) & 0x3fff);
                out[9] = (in[3] << 12) & 0x3fff;
                out[9] |=  (1 << 14) | ((in[4] >> 20));
                out[10] = (1 << 14) | ((in[4] >> 6) & 0x3fff);
                out[11] = (in[4] << 8) & 0x3fff;
                out[11] |=  (1 << 14) | ((in[5] >> 24));
                out[12] = (1 << 14) | ((in[5] >> 10) & 0x3fff);
                out[13] = (in[5] << 4) & 0x3fff;
                out[13] |=  (1 << 14) | ((in[6] >> 28));
                out[14] = (1 << 14) | ((in[6] >> 14) & 0x3fff);
                out[15] = (1 << 14) | (in[6] & 0x3fff);
                out[16] = (1 << 14) | ((in[7] >> 18));
                out[17] = (1 << 14) | ((in[7] >> 4) & 0x3fff);
                out[18] = (in[7] << 10) & 0x3fff;
                out[18] |=  (1 << 14) | ((in[8] >> 22));
                out[19] = (1 << 14) | ((in[8] >> 8) & 0x3fff);
                out[20] = (in[8] << 6) & 0x3fff;
                out[20] |=  (1 << 14) | ((in[9] >> 26));
                out[21] = (1 << 14) | ((in[9] >> 12) & 0x3fff);
                out[22] = (in[9] << 2) & 0x3fff;
                out[22] |=  (1 << 14) | ((in[10] >> 30));
                out[23] = (1 << 14) | ((in[10] >> 16) & 0x3fff);
                out[24] = (1 << 14) | ((in[10] >> 2) & 0x3fff);
                out[25] = (in[10] << 12) & 0x3fff;
                out[25] |=  (1 << 14) | ((in[11] >> 20));
                out[26] = (1 << 14) | ((in[11] >> 6) & 0x3fff);
                out[27] = (in[11] << 8) & 0x3fff;
                out[27] |=  (1 << 14) | ((in[12] >> 24));
                out[28] = (1 << 14) | ((in[12] >> 10) & 0x3fff);
                out[29] = (in[12] << 4) & 0x3fff;
                out[29] |=  (1 << 14) | ((in[13] >> 28));
                out[30] = (1 << 14) | ((in[13] >> 14) & 0x3fff);
                out[31] = (1 << 14) | (in[13] & 0x3fff);
        }
}

void
__vser_unpack15(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 15, out += 32, in += 15) {
                out[0] = (1 << 15) | ((in[0] >> 17));
                out[1] = (1 << 15) | ((in[0] >> 2) & 0x7fff);
                out[2] = (in[0] << 13) & 0x7fff;
                out[2] |=  (1 << 15) | ((in[1] >> 19));
                out[3] = (1 << 15) | ((in[1] >> 4) & 0x7fff);
                out[4] = (in[1] << 11) & 0x7fff;
                out[4] |=  (1 << 15) | ((in[2] >> 21));
                out[5] = (1 << 15) | ((in[2] >> 6) & 0x7fff);
                out[6] = (in[2] << 9) & 0x7fff;
                out[6] |=  (1 << 15) | ((in[3] >> 23));
                out[7] = (1 << 15) | ((in[3] >> 8) & 0x7fff);
                out[8] = (in[3] << 7) & 0x7fff;
                out[8] |=  (1 << 15) | ((in[4] >> 25));
                out[9] = (1 << 15) | ((in[4] >> 10) & 0x7fff);
                out[10] = (in[4] << 5) & 0x7fff;
                out[10] |=  (1 << 15) | ((in[5] >> 27));
                out[11] = (1 << 15) | ((in[5] >> 12) & 0x7fff);
                out[12] = (in[5] << 3) & 0x7fff;
                out[12] |=  (1 << 15) | ((in[6] >> 29));
                out[13] = (1 << 15) | ((in[6] >> 14) & 0x7fff);
                out[14] = (in[6] << 1) & 0x7fff;
                out[14] |=  (1 << 15) | ((in[7] >> 31));
                out[15] = (1 << 15) | ((in[7] >> 16) & 0x7fff);
                out[16] = (1 << 15) | ((in[7] >> 1) & 0x7fff);
                out[17] = (in[7] << 14) & 0x7fff;
                out[17] |=  (1 << 15) | ((in[8] >> 18));
                out[18] = (1 << 15) | ((in[8] >> 3) & 0x7fff);
                out[19] = (in[8] << 12) & 0x7fff;
                out[19] |=  (1 << 15) | ((in[9] >> 20));
                out[20] = (1 << 15) | ((in[9] >> 5) & 0x7fff);
                out[21] = (in[9] << 10) & 0x7fff;
                out[21] |=  (1 << 15) | ((in[10] >> 22));
                out[22] = (1 << 15) | ((in[10] >> 7) & 0x7fff);
                out[23] = (in[10] << 8) & 0x7fff;
                out[23] |=  (1 << 15) | ((in[11] >> 24));
                out[24] = (1 << 15) | ((in[11] >> 9) & 0x7fff);
                out[25] = (in[11] << 6) & 0x7fff;
                out[25] |=  (1 << 15) | ((in[12] >> 26));
                out[26] = (1 << 15) | ((in[12] >> 11) & 0x7fff);
                out[27] = (in[12] << 4) & 0x7fff;
                out[27] |=  (1 << 15) | ((in[13] >> 28));
                out[28] = (1 << 15) | ((in[13] >> 13) & 0x7fff);
                out[29] = (in[13] << 2) & 0x7fff;
                out[29] |=  (1 << 15) | ((in[14] >> 30));
                out[30] = (1 << 15) | ((in[14] >> 15) & 0x7fff);
                out[31] = (1 << 15) | (in[14] & 0x7fff);
        }
}

void
__vser_unpack16(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 16, out += 32, in += 16) {
                out[0] = (1 << 16) | ((in[0] >> 16));
                out[1] = (1 << 16) | (in[0] & 0xffff);
                out[2] = (1 << 16) | ((in[1] >> 16));
                out[3] = (1 << 16) | (in[1] & 0xffff);
                out[4] = (1 << 16) | ((in[2] >> 16));
                out[5] = (1 << 16) | (in[2] & 0xffff);
                out[6] = (1 << 16) | ((in[3] >> 16));
                out[7] = (1 << 16) | (in[3] & 0xffff);
                out[8] = (1 << 16) | ((in[4] >> 16));
                out[9] = (1 << 16) | (in[4] & 0xffff);
                out[10] = (1 << 16) | ((in[5] >> 16));
                out[11] = (1 << 16) | (in[5] & 0xffff);
                out[12] = (1 << 16) | ((in[6] >> 16));
                out[13] = (1 << 16) | (in[6] & 0xffff);
                out[14] = (1 << 16) | ((in[7] >> 16));
                out[15] = (1 << 16) | (in[7] & 0xffff);
                out[16] = (1 << 16) | ((in[8] >> 16));
                out[17] = (1 << 16) | (in[8] & 0xffff);
                out[18] = (1 << 16) | ((in[9] >> 16));
                out[19] = (1 << 16) | (in[9] & 0xffff);
                out[20] = (1 << 16) | ((in[10] >> 16));
                out[21] = (1 << 16) | (in[10] & 0xffff);
                out[22] = (1 << 16) | ((in[11] >> 16));
                out[23] = (1 << 16) | (in[11] & 0xffff);
                out[24] = (1 << 16) | ((in[12] >> 16));
                out[25] = (1 << 16) | (in[12] & 0xffff);
                out[26] = (1 << 16) | ((in[13] >> 16));
                out[27] = (1 << 16) | (in[13] & 0xffff);
                out[28] = (1 << 16) | ((in[14] >> 16));
                out[29] = (1 << 16) | (in[14] & 0xffff);
                out[30] = (1 << 16) | ((in[15] >> 16));
                out[31] = (1 << 16) | (in[15] & 0xffff);
        }
}

void
__vser_unpack17(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 17, out += 32, in += 17) {
                out[0] = (1 << 17) | ((in[0] >> 15));
                out[1] = (in[0] << 2) & 0x01ffff;
                out[1] |=  (1 << 17) | ((in[1] >> 30));
                out[2] = (1 << 17) | ((in[1] >> 13) & 0x01ffff);
                out[3] = (in[1] << 4) & 0x01ffff;
                out[3] |=  (1 << 17) | ((in[2] >> 28));
                out[4] = (1 << 17) | ((in[2] >> 11) & 0x01ffff);
                out[5] = (in[2] << 6) & 0x01ffff;
                out[5] |=  (1 << 17) | ((in[3] >> 26));
                out[6] = (1 << 17) | ((in[3] >> 9) & 0x01ffff);
                out[7] = (in[3] << 8) & 0x01ffff;
                out[7] |=  (1 << 17) | ((in[4] >> 24));
                out[8] = (1 << 17) | ((in[4] >> 7) & 0x01ffff);
                out[9] = (in[4] << 10) & 0x01ffff;
                out[9] |=  (1 << 17) | ((in[5] >> 22));
                out[10] = (1 << 17) | ((in[5] >> 5) & 0x01ffff);
                out[11] = (in[5] << 12) & 0x01ffff;
                out[11] |=  (1 << 17) | ((in[6] >> 20));
                out[12] = (1 << 17) | ((in[6] >> 3) & 0x01ffff);
                out[13] = (in[6] << 14) & 0x01ffff;
                out[13] |=  (1 << 17) | ((in[7] >> 18));
                out[14] = (1 << 17) | ((in[7] >> 1) & 0x01ffff);
                out[15] = (in[7] << 16) & 0x01ffff;
                out[15] |=  (1 << 17) | ((in[8] >> 16));
                out[16] = (in[8] << 1) & 0x01ffff;
                out[16] |=  (1 << 17) | ((in[9] >> 31));
                out[17] = (1 << 17) | ((in[9] >> 14) & 0x01ffff);
                out[18] = (in[9] << 3) & 0x01ffff;
                out[18] |=  (1 << 17) | ((in[10] >> 29));
                out[19] = (1 << 17) | ((in[10] >> 12) & 0x01ffff);
                out[20] = (in[10] << 5) & 0x01ffff;
                out[20] |=  (1 << 17) | ((in[11] >> 27));
                out[21] = (1 << 17) | ((in[11] >> 10) & 0x01ffff);
                out[22] = (in[11] << 7) & 0x01ffff;
                out[22] |=  (1 << 17) | ((in[12] >> 25));
                out[23] = (1 << 17) | ((in[12] >> 8) & 0x01ffff);
                out[24] = (in[12] << 9) & 0x01ffff;
                out[24] |=  (1 << 17) | ((in[13] >> 23));
                out[25] = (1 << 17) | ((in[13] >> 6) & 0x01ffff);
                out[26] = (in[13] << 11) & 0x01ffff;
                out[26] |=  (1 << 17) | ((in[14] >> 21));
                out[27] = (1 << 17) | ((in[14] >> 4) & 0x01ffff);
                out[28] = (in[14] << 13) & 0x01ffff;
                out[28] |=  (1 << 17) | ((in[15] >> 19));
                out[29] = (1 << 17) | ((in[15] >> 2) & 0x01ffff);
                out[30] = (in[15] << 15) & 0x01ffff;
                out[30] |=  (1 << 17) | ((in[16] >> 17));
                out[31] = (1 << 17) | (in[16] & 0x01ffff);
        }
}

void
__vser_unpack18(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 18, out += 32, in += 18) {
                out[0] = (1 << 18) | ((in[0] >> 14));
                out[1] = (in[0] << 4) & 0x03ffff;
                out[1] |=  (1 << 18) | ((in[1] >> 28));
                out[2] = (1 << 18) | ((in[1] >> 10) & 0x03ffff);
                out[3] = (in[1] << 8) & 0x03ffff;
                out[3] |=  (1 << 18) | ((in[2] >> 24));
                out[4] = (1 << 18) | ((in[2] >> 6) & 0x03ffff);
                out[5] = (in[2] << 12) & 0x03ffff;
                out[5] |=  (1 << 18) | ((in[3] >> 20));
                out[6] = (1 << 18) | ((in[3] >> 2) & 0x03ffff);
                out[7] = (in[3] << 16) & 0x03ffff;
                out[7] |=  (1 << 18) | ((in[4] >> 16));
                out[8] = (in[4] << 2) & 0x03ffff;
                out[8] |=  (1 << 18) | ((in[5] >> 30));
                out[9] = (1 << 18) | ((in[5] >> 12) & 0x03ffff);
                out[10] = (in[5] << 6) & 0x03ffff;
                out[10] |=  (1 << 18) | ((in[6] >> 26));
                out[11] = (1 << 18) | ((in[6] >> 8) & 0x03ffff);
                out[12] = (in[6] << 10) & 0x03ffff;
                out[12] |=  (1 << 18) | ((in[7] >> 22));
                out[13] = (1 << 18) | ((in[7] >> 4) & 0x03ffff);
                out[14] = (in[7] << 14) & 0x03ffff;
                out[14] |=  (1 << 18) | ((in[8] >> 18));
                out[15] = (1 << 18) | (in[8] & 0x03ffff);
                out[16] = (1 << 18) | ((in[9] >> 14));
                out[17] = (in[9] << 4) & 0x03ffff;
                out[17] |=  (1 << 18) | ((in[10] >> 28));
                out[18] = (1 << 18) | ((in[10] >> 10) & 0x03ffff);
                out[19] = (in[10] << 8) & 0x03ffff;
                out[19] |=  (1 << 18) | ((in[11] >> 24));
                out[20] = (1 << 18) | ((in[11] >> 6) & 0x03ffff);
                out[21] = (in[11] << 12) & 0x03ffff;
                out[21] |=  (1 << 18) | ((in[12] >> 20));
                out[22] = (1 << 18) | ((in[12] >> 2) & 0x03ffff);
                out[23] = (in[12] << 16) & 0x03ffff;
                out[23] |=  (1 << 18) | ((in[13] >> 16));
                out[24] = (in[13] << 2) & 0x03ffff;
                out[24] |=  (1 << 18) | ((in[14] >> 30));
                out[25] = (1 << 18) | ((in[14] >> 12) & 0x03ffff);
                out[26] = (in[14] << 6) & 0x03ffff;
                out[26] |=  (1 << 18) | ((in[15] >> 26));
                out[27] = (1 << 18) | ((in[15] >> 8) & 0x03ffff);
                out[28] = (in[15] << 10) & 0x03ffff;
                out[28] |=  (1 << 18) | ((in[16] >> 22));
                out[29] = (1 << 18) | ((in[16] >> 4) & 0x03ffff);
                out[30] = (in[16] << 14) & 0x03ffff;
                out[30] |=  (1 << 18) | ((in[17] >> 18));
                out[31] = (1 << 18) | (in[17] & 0x03ffff);
        }
}

void
__vser_unpack19(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 19, out += 32, in += 19) {
                out[0] = (1 << 19) | ((in[0] >> 13));
                out[1] = (in[0] << 6) & 0x07ffff;
                out[1] |=  (1 << 19) | ((in[1] >> 26));
                out[2] = (1 << 19) | ((in[1] >> 7) & 0x07ffff);
                out[3] = (in[1] << 12) & 0x07ffff;
                out[3] |=  (1 << 19) | ((in[2] >> 20));
                out[4] = (1 << 19) | ((in[2] >> 1) & 0x07ffff);
                out[5] = (in[2] << 18) & 0x07ffff;
                out[5] |=  (1 << 19) | ((in[3] >> 14));
                out[6] = (in[3] << 5) & 0x07ffff;
                out[6] |=  (1 << 19) | ((in[4] >> 27));
                out[7] = (1 << 19) | ((in[4] >> 8) & 0x07ffff);
                out[8] = (in[4] << 11) & 0x07ffff;
                out[8] |=  (1 << 19) | ((in[5] >> 21));
                out[9] = (1 << 19) | ((in[5] >> 2) & 0x07ffff);
                out[10] = (in[5] << 17) & 0x07ffff;
                out[10] |=  (1 << 19) | ((in[6] >> 15));
                out[11] = (in[6] << 4) & 0x07ffff;
                out[11] |=  (1 << 19) | ((in[7] >> 28));
                out[12] = (1 << 19) | ((in[7] >> 9) & 0x07ffff);
                out[13] = (in[7] << 10) & 0x07ffff;
                out[13] |=  (1 << 19) | ((in[8] >> 22));
                out[14] = (1 << 19) | ((in[8] >> 3) & 0x07ffff);
                out[15] = (in[8] << 16) & 0x07ffff;
                out[15] |=  (1 << 19) | ((in[9] >> 16));
                out[16] = (in[9] << 3) & 0x07ffff;
                out[16] |=  (1 << 19) | ((in[10] >> 29));
                out[17] = (1 << 19) | ((in[10] >> 10) & 0x07ffff);
                out[18] = (in[10] << 9) & 0x07ffff;
                out[18] |=  (1 << 19) | ((in[11] >> 23));
                out[19] = (1 << 19) | ((in[11] >> 4) & 0x07ffff);
                out[20] = (in[11] << 15) & 0x07ffff;
                out[20] |=  (1 << 19) | ((in[12] >> 17));
                out[21] = (in[12] << 2) & 0x07ffff;
                out[21] |=  (1 << 19) | ((in[13] >> 30));
                out[22] = (1 << 19) | ((in[13] >> 11) & 0x07ffff);
                out[23] = (in[13] << 8) & 0x07ffff;
                out[23] |=  (1 << 19) | ((in[14] >> 24));
                out[24] = (1 << 19) | ((in[14] >> 5) & 0x07ffff);
                out[25] = (in[14] << 14) & 0x07ffff;
                out[25] |=  (1 << 19) | ((in[15] >> 18));
                out[26] = (in[15] << 1) & 0x07ffff;
                out[26] |=  (1 << 19) | ((in[16] >> 31));
                out[27] = (1 << 19) | ((in[16] >> 12) & 0x07ffff);
                out[28] = (in[16] << 7) & 0x07ffff;
                out[28] |=  (1 << 19) | ((in[17] >> 25));
                out[29] = (1 << 19) | ((in[17] >> 6) & 0x07ffff);
                out[30] = (in[17] << 13) & 0x07ffff;
                out[30] |=  (1 << 19) | ((in[18] >> 19));
                out[31] = (1 << 19) | (in[18] & 0x07ffff);
        }
}

void
__vser_unpack20(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 20, out += 32, in += 20) {
                out[0] = (1 << 20) | ((in[0] >> 12));
                out[1] = (in[0] << 8) & 0x0fffff;
                out[1] |=  (1 << 20) | ((in[1] >> 24));
                out[2] = (1 << 20) | ((in[1] >> 4) & 0x0fffff);
                out[3] = (in[1] << 16) & 0x0fffff;
                out[3] |=  (1 << 20) | ((in[2] >> 16));
                out[4] = (in[2] << 4) & 0x0fffff;
                out[4] |=  (1 << 20) | ((in[3] >> 28));
                out[5] = (1 << 20) | ((in[3] >> 8) & 0x0fffff);
                out[6] = (in[3] << 12) & 0x0fffff;
                out[6] |=  (1 << 20) | ((in[4] >> 20));
                out[7] = (1 << 20) | (in[4] & 0x0fffff);
                out[8] = (1 << 20) | ((in[5] >> 12));
                out[9] = (in[5] << 8) & 0x0fffff;
                out[9] |=  (1 << 20) | ((in[6] >> 24));
                out[10] = (1 << 20) | ((in[6] >> 4) & 0x0fffff);
                out[11] = (in[6] << 16) & 0x0fffff;
                out[11] |=  (1 << 20) | ((in[7] >> 16));
                out[12] = (in[7] << 4) & 0x0fffff;
                out[12] |=  (1 << 20) | ((in[8] >> 28));
                out[13] = (1 << 20) | ((in[8] >> 8) & 0x0fffff);
                out[14] = (in[8] << 12) & 0x0fffff;
                out[14] |=  (1 << 20) | ((in[9] >> 20));
                out[15] = (1 << 20) | (in[9] & 0x0fffff);
                out[16] = (1 << 20) | ((in[10] >> 12));
                out[17] = (in[10] << 8) & 0x0fffff;
                out[17] |=  (1 << 20) | ((in[11] >> 24));
                out[18] = (1 << 20) | ((in[11] >> 4) & 0x0fffff);
                out[19] = (in[11] << 16) & 0x0fffff;
                out[19] |=  (1 << 20) | ((in[12] >> 16));
                out[20] = (in[12] << 4) & 0x0fffff;
                out[20] |=  (1 << 20) | ((in[13] >> 28));
                out[21] = (1 << 20) | ((in[13] >> 8) & 0x0fffff);
                out[22] = (in[13] << 12) & 0x0fffff;
                out[22] |=  (1 << 20) | ((in[14] >> 20));
                out[23] = (1 << 20) | (in[14] & 0x0fffff);
                out[24] = (1 << 20) | ((in[15] >> 12));
                out[25] = (in[15] << 8) & 0x0fffff;
                out[25] |=  (1 << 20) | ((in[16] >> 24));
                out[26] = (1 << 20) | ((in[16] >> 4) & 0x0fffff);
                out[27] = (in[16] << 16) & 0x0fffff;
                out[27] |=  (1 << 20) | ((in[17] >> 16));
                out[28] = (in[17] << 4) & 0x0fffff;
                out[28] |=  (1 << 20) | ((in[18] >> 28));
                out[29] = (1 << 20) | ((in[18] >> 8) & 0x0fffff);
                out[30] = (in[18] << 12) & 0x0fffff;
                out[30] |=  (1 << 20) | ((in[19] >> 20));
                out[31] = (1 << 20) | (in[19] & 0x0fffff);
        }
}

void
__vser_unpack21(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 21, out += 32, in += 21) {
                out[0] = (1 << 21) | ((in[0] >> 11));
                out[1] = (in[0] << 10) & 0x1fffff;
                out[1] |=  (1 << 21) | ((in[1] >> 22));
                out[2] = (1 << 21) | ((in[1] >> 1) & 0x1fffff);
                out[3] = (in[1] << 20) & 0x1fffff;
                out[3] |=  (1 << 21) | ((in[2] >> 12));
                out[4] = (in[2] << 9) & 0x1fffff;
                out[4] |=  (1 << 21) | ((in[3] >> 23));
                out[5] = (1 << 21) | ((in[3] >> 2) & 0x1fffff);
                out[6] = (in[3] << 19) & 0x1fffff;
                out[6] |=  (1 << 21) | ((in[4] >> 13));
                out[7] = (in[4] << 8) & 0x1fffff;
                out[7] |=  (1 << 21) | ((in[5] >> 24));
                out[8] = (1 << 21) | ((in[5] >> 3) & 0x1fffff);
                out[9] = (in[5] << 18) & 0x1fffff;
                out[9] |=  (1 << 21) | ((in[6] >> 14));
                out[10] = (in[6] << 7) & 0x1fffff;
                out[10] |=  (1 << 21) | ((in[7] >> 25));
                out[11] = (1 << 21) | ((in[7] >> 4) & 0x1fffff);
                out[12] = (in[7] << 17) & 0x1fffff;
                out[12] |=  (1 << 21) | ((in[8] >> 15));
                out[13] = (in[8] << 6) & 0x1fffff;
                out[13] |=  (1 << 21) | ((in[9] >> 26));
                out[14] = (1 << 21) | ((in[9] >> 5) & 0x1fffff);
                out[15] = (in[9] << 16) & 0x1fffff;
                out[15] |=  (1 << 21) | ((in[10] >> 16));
                out[16] = (in[10] << 5) & 0x1fffff;
                out[16] |=  (1 << 21) | ((in[11] >> 27));
                out[17] = (1 << 21) | ((in[11] >> 6) & 0x1fffff);
                out[18] = (in[11] << 15) & 0x1fffff;
                out[18] |=  (1 << 21) | ((in[12] >> 17));
                out[19] = (in[12] << 4) & 0x1fffff;
                out[19] |=  (1 << 21) | ((in[13] >> 28));
                out[20] = (1 << 21) | ((in[13] >> 7) & 0x1fffff);
                out[21] = (in[13] << 14) & 0x1fffff;
                out[21] |=  (1 << 21) | ((in[14] >> 18));
                out[22] = (in[14] << 3) & 0x1fffff;
                out[22] |=  (1 << 21) | ((in[15] >> 29));
                out[23] = (1 << 21) | ((in[15] >> 8) & 0x1fffff);
                out[24] = (in[15] << 13) & 0x1fffff;
                out[24] |=  (1 << 21) | ((in[16] >> 19));
                out[25] = (in[16] << 2) & 0x1fffff;
                out[25] |=  (1 << 21) | ((in[17] >> 30));
                out[26] = (1 << 21) | ((in[17] >> 9) & 0x1fffff);
                out[27] = (in[17] << 12) & 0x1fffff;
                out[27] |=  (1 << 21) | ((in[18] >> 20));
                out[28] = (in[18] << 1) & 0x1fffff;
                out[28] |=  (1 << 21) | ((in[19] >> 31));
                out[29] = (1 << 21) | ((in[19] >> 10) & 0x1fffff);
                out[30] = (in[19] << 11) & 0x1fffff;
                out[30] |=  (1 << 21) | ((in[20] >> 21));
                out[31] = (1 << 21) | (in[20] & 0x1fffff);
        }
}

void
__vser_unpack22(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 22, out += 32, in += 22) {
                out[0] = (1 << 22) | ((in[0] >> 10));
                out[1] = (in[0] << 12) & 0x3fffff;
                out[1] |=  (1 << 22) | ((in[1] >> 20));
                out[2] = (in[1] << 2) & 0x3fffff;
                out[2] |=  (1 << 22) | ((in[2] >> 30));
                out[3] = (1 << 22) | ((in[2] >> 8) & 0x3fffff);
                out[4] = (in[2] << 14) & 0x3fffff;
                out[4] |=  (1 << 22) | ((in[3] >> 18));
                out[5] = (in[3] << 4) & 0x3fffff;
                out[5] |=  (1 << 22) | ((in[4] >> 28));
                out[6] = (1 << 22) | ((in[4] >> 6) & 0x3fffff);
                out[7] = (in[4] << 16) & 0x3fffff;
                out[7] |=  (1 << 22) | ((in[5] >> 16));
                out[8] = (in[5] << 6) & 0x3fffff;
                out[8] |=  (1 << 22) | ((in[6] >> 26));
                out[9] = (1 << 22) | ((in[6] >> 4) & 0x3fffff);
                out[10] = (in[6] << 18) & 0x3fffff;
                out[10] |=  (1 << 22) | ((in[7] >> 14));
                out[11] = (in[7] << 8) & 0x3fffff;
                out[11] |=  (1 << 22) | ((in[8] >> 24));
                out[12] = (1 << 22) | ((in[8] >> 2) & 0x3fffff);
                out[13] = (in[8] << 20) & 0x3fffff;
                out[13] |=  (1 << 22) | ((in[9] >> 12));
                out[14] = (in[9] << 10) & 0x3fffff;
                out[14] |=  (1 << 22) | ((in[10] >> 22));
                out[15] = (1 << 22) | (in[10] & 0x3fffff);
                out[16] = (1 << 22) | ((in[11] >> 10));
                out[17] = (in[11] << 12) & 0x3fffff;
                out[17] |=  (1 << 22) | ((in[12] >> 20));
                out[18] = (in[12] << 2) & 0x3fffff;
                out[18] |=  (1 << 22) | ((in[13] >> 30));
                out[19] = (1 << 22) | ((in[13] >> 8) & 0x3fffff);
                out[20] = (in[13] << 14) & 0x3fffff;
                out[20] |=  (1 << 22) | ((in[14] >> 18));
                out[21] = (in[14] << 4) & 0x3fffff;
                out[21] |=  (1 << 22) | ((in[15] >> 28));
                out[22] = (1 << 22) | ((in[15] >> 6) & 0x3fffff);
                out[23] = (in[15] << 16) & 0x3fffff;
                out[23] |=  (1 << 22) | ((in[16] >> 16));
                out[24] = (in[16] << 6) & 0x3fffff;
                out[24] |=  (1 << 22) | ((in[17] >> 26));
                out[25] = (1 << 22) | ((in[17] >> 4) & 0x3fffff);
                out[26] = (in[17] << 18) & 0x3fffff;
                out[26] |=  (1 << 22) | ((in[18] >> 14));
                out[27] = (in[18] << 8) & 0x3fffff;
                out[27] |=  (1 << 22) | ((in[19] >> 24));
                out[28] = (1 << 22) | ((in[19] >> 2) & 0x3fffff);
                out[29] = (in[19] << 20) & 0x3fffff;
                out[29] |=  (1 << 22) | ((in[20] >> 12));
                out[30] = (in[20] << 10) & 0x3fffff;
                out[30] |=  (1 << 22) | ((in[21] >> 22));
                out[31] = (1 << 22) | (in[21] & 0x3fffff);
        }
}

void
__vser_unpack23(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 23, out += 32, in += 23) {
                out[0] = (1 << 23) | ((in[0] >> 9));
                out[1] = (in[0] << 14) & 0x7fffff;
                out[1] |=  (1 << 23) | ((in[1] >> 18));
                out[2] = (in[1] << 5) & 0x7fffff;
                out[2] |=  (1 << 23) | ((in[2] >> 27));
                out[3] = (1 << 23) | ((in[2] >> 4) & 0x7fffff);
                out[4] = (in[2] << 19) & 0x7fffff;
                out[4] |=  (1 << 23) | ((in[3] >> 13));
                out[5] = (in[3] << 10) & 0x7fffff;
                out[5] |=  (1 << 23) | ((in[4] >> 22));
                out[6] = (in[4] << 1) & 0x7fffff;
                out[6] |=  (1 << 23) | ((in[5] >> 31));
                out[7] = (1 << 23) | ((in[5] >> 8) & 0x7fffff);
                out[8] = (in[5] << 15) & 0x7fffff;
                out[8] |=  (1 << 23) | ((in[6] >> 17));
                out[9] = (in[6] << 6) & 0x7fffff;
                out[9] |=  (1 << 23) | ((in[7] >> 26));
                out[10] = (1 << 23) | ((in[7] >> 3) & 0x7fffff);
                out[11] = (in[7] << 20) & 0x7fffff;
                out[11] |=  (1 << 23) | ((in[8] >> 12));
                out[12] = (in[8] << 11) & 0x7fffff;
                out[12] |=  (1 << 23) | ((in[9] >> 21));
                out[13] = (in[9] << 2) & 0x7fffff;
                out[13] |=  (1 << 23) | ((in[10] >> 30));
                out[14] = (1 << 23) | ((in[10] >> 7) & 0x7fffff);
                out[15] = (in[10] << 16) & 0x7fffff;
                out[15] |=  (1 << 23) | ((in[11] >> 16));
                out[16] = (in[11] << 7) & 0x7fffff;
                out[16] |=  (1 << 23) | ((in[12] >> 25));
                out[17] = (1 << 23) | ((in[12] >> 2) & 0x7fffff);
                out[18] = (in[12] << 21) & 0x7fffff;
                out[18] |=  (1 << 23) | ((in[13] >> 11));
                out[19] = (in[13] << 12) & 0x7fffff;
                out[19] |=  (1 << 23) | ((in[14] >> 20));
                out[20] = (in[14] << 3) & 0x7fffff;
                out[20] |=  (1 << 23) | ((in[15] >> 29));
                out[21] = (1 << 23) | ((in[15] >> 6) & 0x7fffff);
                out[22] = (in[15] << 17) & 0x7fffff;
                out[22] |=  (1 << 23) | ((in[16] >> 15));
                out[23] = (in[16] << 8) & 0x7fffff;
                out[23] |=  (1 << 23) | ((in[17] >> 24));
                out[24] = (1 << 23) | ((in[17] >> 1) & 0x7fffff);
                out[25] = (in[17] << 22) & 0x7fffff;
                out[25] |=  (1 << 23) | ((in[18] >> 10));
                out[26] = (in[18] << 13) & 0x7fffff;
                out[26] |=  (1 << 23) | ((in[19] >> 19));
                out[27] = (in[19] << 4) & 0x7fffff;
                out[27] |=  (1 << 23) | ((in[20] >> 28));
                out[28] = (1 << 23) | ((in[20] >> 5) & 0x7fffff);
                out[29] = (in[20] << 18) & 0x7fffff;
                out[29] |=  (1 << 23) | ((in[21] >> 14));
                out[30] = (in[21] << 9) & 0x7fffff;
                out[30] |=  (1 << 23) | ((in[22] >> 23));
                out[31] = (1 << 23) | (in[22] & 0x7fffff);
        }
}

void
__vser_unpack24(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 24, out += 32, in += 24) {
                out[0] = (1 << 24) | ((in[0] >> 8));
                out[1] = (in[0] << 16) & 0xffffff;
                out[1] |=  (1 << 24) | ((in[1] >> 16));
                out[2] = (in[1] << 8) & 0xffffff;
                out[2] |=  (1 << 24) | ((in[2] >> 24));
                out[3] = (1 << 24) | (in[2] & 0xffffff);
                out[4] = (1 << 24) | ((in[3] >> 8));
                out[5] = (in[3] << 16) & 0xffffff;
                out[5] |=  (1 << 24) | ((in[4] >> 16));
                out[6] = (in[4] << 8) & 0xffffff;
                out[6] |=  (1 << 24) | ((in[5] >> 24));
                out[7] = (1 << 24) | (in[5] & 0xffffff);
                out[8] = (1 << 24) | ((in[6] >> 8));
                out[9] = (in[6] << 16) & 0xffffff;
                out[9] |=  (1 << 24) | ((in[7] >> 16));
                out[10] = (in[7] << 8) & 0xffffff;
                out[10] |=  (1 << 24) | ((in[8] >> 24));
                out[11] = (1 << 24) | (in[8] & 0xffffff);
                out[12] = (1 << 24) | ((in[9] >> 8));
                out[13] = (in[9] << 16) & 0xffffff;
                out[13] |=  (1 << 24) | ((in[10] >> 16));
                out[14] = (in[10] << 8) & 0xffffff;
                out[14] |=  (1 << 24) | ((in[11] >> 24));
                out[15] = (1 << 24) | (in[11] & 0xffffff);
                out[16] = (1 << 24) | ((in[12] >> 8));
                out[17] = (in[12] << 16) & 0xffffff;
                out[17] |=  (1 << 24) | ((in[13] >> 16));
                out[18] = (in[13] << 8) & 0xffffff;
                out[18] |=  (1 << 24) | ((in[14] >> 24));
                out[19] = (1 << 24) | (in[14] & 0xffffff);
                out[20] = (1 << 24) | ((in[15] >> 8));
                out[21] = (in[15] << 16) & 0xffffff;
                out[21] |=  (1 << 24) | ((in[16] >> 16));
                out[22] = (in[16] << 8) & 0xffffff;
                out[22] |=  (1 << 24) | ((in[17] >> 24));
                out[23] = (1 << 24) | (in[17] & 0xffffff);
                out[24] = (1 << 24) | ((in[18] >> 8));
                out[25] = (in[18] << 16) & 0xffffff;
                out[25] |=  (1 << 24) | ((in[19] >> 16));
                out[26] = (in[19] << 8) & 0xffffff;
                out[26] |=  (1 << 24) | ((in[20] >> 24));
                out[27] = (1 << 24) | (in[20] & 0xffffff);
                out[28] = (1 << 24) | ((in[21] >> 8));
                out[29] = (in[21] << 16) & 0xffffff;
                out[29] |=  (1 << 24) | ((in[22] >> 16));
                out[30] = (in[22] << 8) & 0xffffff;
                out[30] |=  (1 << 24) | ((in[23] >> 24));
                out[31] = (1 << 24) | (in[23] & 0xffffff);
        }
}

void
__vser_unpack25(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 25, out += 32, in += 25) {
                out[0] = (1 << 25) | ((in[0] >> 7));
                out[1] = (in[0] << 18) & 0x01ffffff;
                out[1] |=  (1 << 25) | ((in[1] >> 14));
                out[2] = (in[1] << 11) & 0x01ffffff;
                out[2] |=  (1 << 25) | ((in[2] >> 21));
                out[3] = (in[2] << 4) & 0x01ffffff;
                out[3] |=  (1 << 25) | ((in[3] >> 28));
                out[4] = (1 << 25) | ((in[3] >> 3) & 0x01ffffff);
                out[5] = (in[3] << 22) & 0x01ffffff;
                out[5] |=  (1 << 25) | ((in[4] >> 10));
                out[6] = (in[4] << 15) & 0x01ffffff;
                out[6] |=  (1 << 25) | ((in[5] >> 17));
                out[7] = (in[5] << 8) & 0x01ffffff;
                out[7] |=  (1 << 25) | ((in[6] >> 24));
                out[8] = (in[6] << 1) & 0x01ffffff;
                out[8] |=  (1 << 25) | ((in[7] >> 31));
                out[9] = (1 << 25) | ((in[7] >> 6) & 0x01ffffff);
                out[10] = (in[7] << 19) & 0x01ffffff;
                out[10] |=  (1 << 25) | ((in[8] >> 13));
                out[11] = (in[8] << 12) & 0x01ffffff;
                out[11] |=  (1 << 25) | ((in[9] >> 20));
                out[12] = (in[9] << 5) & 0x01ffffff;
                out[12] |=  (1 << 25) | ((in[10] >> 27));
                out[13] = (1 << 25) | (((in[10] >> 2) & 0x01ffffff));
                out[14] = (in[10] << 23) & 0x01ffffff;
                out[14] |=  (1 << 25) | ((in[11] >> 9));
                out[15] = (in[11] << 16) & 0x01ffffff;
                out[15] |=  (1 << 25) | ((in[12] >> 16));
                out[16] = (in[12] << 9) & 0x01ffffff;
                out[16] |=  (1 << 25) | ((in[13] >> 23));
                out[17] = (in[13] << 2) & 0x01ffffff;
                out[17] |=  (1 << 25) | ((in[14] >> 30));
                out[18] = (1 << 25) | ((in[14] >> 5) & 0x01ffffff);
                out[19] = (in[14] << 20) & 0x01ffffff;
                out[19] |=  (1 << 25) | ((in[15] >> 12));
                out[20] = (in[15] << 13) & 0x01ffffff;
                out[20] |=  (1 << 25) | ((in[16] >> 19));
                out[21] = (in[16] << 6) & 0x01ffffff;
                out[21] |=  (1 << 25) | ((in[17] >> 26));
                out[22] = (1 << 25) | ((in[17] >> 1) & 0x01ffffff);
                out[23] = (in[17] << 24) & 0x01ffffff;
                out[23] |=  (1 << 25) | ((in[18] >> 8));
                out[24] = (in[18] << 17) & 0x01ffffff;
                out[24] |=  (1 << 25) | ((in[19] >> 15));
                out[25] = (in[19] << 10) & 0x01ffffff;
                out[25] |=  (1 << 25) | ((in[20] >> 22));
                out[26] = (in[20] << 3) & 0x01ffffff;
                out[26] |=  (1 << 25) | ((in[21] >> 29));
                out[27] = (1 << 25) | ((in[21] >> 4) & 0x01ffffff);
                out[28] = (in[21] << 21) & 0x01ffffff;
                out[28] |=  (1 << 25) | ((in[22] >> 11));
                out[29] = (in[22] << 14) & 0x01ffffff;
                out[29] |=  (1 << 25) | ((in[23] >> 18));
                out[30] = (in[23] << 7) & 0x01ffffff;
                out[30] |=  (1 << 25) | ((in[24] >> 25));
                out[31] = (1 << 25) | (in[24] & 0x01ffffff);
        }
}

void
__vser_unpack26(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 26, out += 32, in += 26) {
                out[0] = (1 << 26) | ((in[0] >> 6));
                out[1] = (in[0] << 20) & 0x03ffffff;
                out[1] |=  (1 << 26) | ((in[1] >> 12));
                out[2] = (in[1] << 14) & 0x03ffffff;
                out[2] |=  (1 << 26) | ((in[2] >> 18));
                out[3] = (in[2] << 8) & 0x03ffffff;
                out[3] |=  (1 << 26) | ((in[3] >> 24));
                out[4] = (in[3] << 2) & 0x03ffffff;
                out[4] |=  (1 << 26) | ((in[4] >> 30));
                out[5] = (1 << 26) | ((in[4] >> 4) & 0x03ffffff);
                out[6] = (in[4] << 22) & 0x03ffffff;
                out[6] |=  (1 << 26) | ((in[5] >> 10));
                out[7] = (in[5] << 16) & 0x03ffffff;
                out[7] |=  (1 << 26) | ((in[6] >> 16));
                out[8] = (in[6] << 10) & 0x03ffffff;
                out[8] |=  (1 << 26) | ((in[7] >> 22));
                out[9] = (in[7] << 4) & 0x03ffffff;
                out[9] |=  (1 << 26) | ((in[8] >> 28));
                out[10] = (1 << 26) | ((in[8] >> 2) & 0x03ffffff);
                out[11] = (in[8] << 24) & 0x03ffffff;
                out[11] |=  (1 << 26) | ((in[9] >> 8));
                out[12] = (in[9] << 18) & 0x03ffffff;
                out[12] |=  (1 << 26) | ((in[10] >> 14));
                out[13] = (in[10] << 12) & 0x03ffffff;
                out[13] |=  (1 << 26) | ((in[11] >> 20));
                out[14] = (in[11] << 6) & 0x03ffffff;
                out[14] |=  (1 << 26) | ((in[12] >> 26));
                out[15] = (1 << 26) | (in[12] & 0x03ffffff);
                out[16] = (1 << 26) | ((in[13] >> 6));
                out[17] = (in[13] << 20) & 0x03ffffff;
                out[17] |=  (1 << 26) | ((in[14] >> 12));
                out[18] = (in[14] << 14) & 0x03ffffff;
                out[18] |=  (1 << 26) | ((in[15] >> 18));
                out[19] = (in[15] << 8) & 0x03ffffff;
                out[19] |=  (1 << 26) | ((in[16] >> 24));
                out[20] = (in[16] << 2) & 0x03ffffff;
                out[20] |=  (1 << 26) | ((in[17] >> 30));
                out[21] = (1 << 26) | ((in[17] >> 4) & 0x03ffffff);
                out[22] = (in[17] << 22) & 0x03ffffff;
                out[22] |=  (1 << 26) | ((in[18] >> 10));
                out[23] = (in[18] << 16) & 0x03ffffff;
                out[23] |=  (1 << 26) | ((in[19] >> 16));
                out[24] = (in[19] << 10) & 0x03ffffff;
                out[24] |=  (1 << 26) | ((in[20] >> 22));
                out[25] = (in[20] << 4) & 0x03ffffff;
                out[25] |=  (1 << 26) | ((in[21] >> 28));
                out[26] = (1 << 26) | ((in[21] >> 2) & 0x03ffffff);
                out[27] = (in[21] << 24) & 0x03ffffff;
                out[27] |=  (1 << 26) | ((in[22] >> 8));
                out[28] = (in[22] << 18) & 0x03ffffff;
                out[28] |=  (1 << 26) | ((in[23] >> 14));
                out[29] = (in[23] << 12) & 0x03ffffff;
                out[29] |=  (1 << 26) | ((in[24] >> 20));
                out[30] = (in[24] << 6) & 0x03ffffff;
                out[30] |=  (1 << 26) | ((in[25] >> 26));
                out[31] = (1 << 26) | (in[25] & 0x03ffffff);
        }
}

void
__vser_unpack27(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 27, out += 32, in += 27) {
                out[0] = (1 << 27) | ((in[0] >> 5));
                out[1] = (in[0] << 22) & 0x07ffffff;
                out[1] |=  (1 << 27) | ((in[1] >> 10));
                out[2] = (in[1] << 17) & 0x07ffffff;
                out[2] |=  (1 << 27) | ((in[2] >> 15));
                out[3] = (in[2] << 12) & 0x07ffffff;
                out[3] |=  (1 << 27) | ((in[3] >> 20));
                out[4] = (in[3] << 7) & 0x07ffffff;
                out[4] |=  (1 << 27) | ((in[4] >> 25));
                out[5] = (in[4] << 2) & 0x07ffffff;
                out[5] |=  (1 << 27) | ((in[5] >> 30));
                out[6] = (1 << 27) | ((in[5] >> 3) & 0x07ffffff);
                out[7] = (in[5] << 24) & 0x07ffffff;
                out[7] |=  (1 << 27) | ((in[6] >> 8));
                out[8] = (in[6] << 19) & 0x07ffffff;
                out[8] |=  (1 << 27) | ((in[7] >> 13));
                out[9] = (in[7] << 14) & 0x07ffffff;
                out[9] |=  (1 << 27) | ((in[8] >> 18));
                out[10] = (in[8] << 9) & 0x07ffffff;
                out[10] |=  (1 << 27) | ((in[9] >> 23));
                out[11] = (in[9] << 4) & 0x07ffffff;
                out[11] |=  (1 << 27) | ((in[10] >> 28));
                out[12] = (1 << 27) | ((in[10] >> 1) & 0x07ffffff);
                out[13] = (in[10] << 26) & 0x07ffffff;
                out[13] |=  (1 << 27) | ((in[11] >> 6));
                out[14] = (in[11] << 21) & 0x07ffffff;
                out[14] |=  (1 << 27) | ((in[12] >> 11));
                out[15] = (in[12] << 16) & 0x07ffffff;
                out[15] |=  (1 << 27) | ((in[13] >> 16));
                out[16] = (in[13] << 11) & 0x07ffffff;
                out[16] |=  (1 << 27) | ((in[14] >> 21));
                out[17] = (in[14] << 6) & 0x07ffffff;
                out[17] |=  (1 << 27) | ((in[15] >> 26));
                out[18] = (in[15] << 1) & 0x07ffffff;
                out[18] |=  (1 << 27) | ((in[16] >> 31));
                out[19] = (1 << 27) | ((in[16] >> 4) & 0x07ffffff);
                out[20] = (in[16] << 23) & 0x07ffffff;
                out[20] |=  (1 << 27) | ((in[17] >> 9));
                out[21] = (in[17] << 18) & 0x07ffffff;
                out[21] |=  (1 << 27) | ((in[18] >> 14));
                out[22] = (in[18] << 13) & 0x07ffffff;
                out[22] |=  (1 << 27) | ((in[19] >> 19));
                out[23] = (in[19] << 8) & 0x07ffffff;
                out[23] |=  (1 << 27) | ((in[20] >> 24));
                out[24] = (in[20] << 3) & 0x07ffffff;
                out[24] |=  (1 << 27) | ((in[21] >> 29));
                out[25] = (1 << 27) | ((in[21] >> 2) & 0x07ffffff);
                out[26] = (in[21] << 25) & 0x07ffffff;
                out[26] |=  (1 << 27) | ((in[22] >> 7));
                out[27] = (in[22] << 20) & 0x07ffffff;
                out[27] |=  (1 << 27) | ((in[23] >> 12));
                out[28] = (in[23] << 15) & 0x07ffffff;
                out[28] |=  (1 << 27) | ((in[24] >> 17));
                out[29] = (in[24] << 10) & 0x07ffffff;
                out[29] |=  (1 << 27) | ((in[25] >> 22));
                out[30] = (in[25] << 5) & 0x07ffffff;
                out[30] |=  (1 << 27) | ((in[26] >> 27));
                out[31] = (1 << 27) | (in[26] & 0x07ffffff);
        }
}

void
__vser_unpack28(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 28, out += 32, in += 28) {
                out[0] = (1 << 28) | ((in[0] >> 4));
                out[1] = (in[0] << 24) & 0x0fffffff;
                out[1] |=  (1 << 28) | ((in[1] >> 8));
                out[2] = (in[1] << 20) & 0x0fffffff;
                out[2] |=  (1 << 28) | ((in[2] >> 12));
                out[3] = (in[2] << 16) & 0x0fffffff;
                out[3] |=  (1 << 28) | ((in[3] >> 16));
                out[4] = (in[3] << 12) & 0x0fffffff;
                out[4] |=  (1 << 28) | ((in[4] >> 20));
                out[5] = (in[4] << 8) & 0x0fffffff;
                out[5] |=  (1 << 28) | ((in[5] >> 24));
                out[6] = (in[5] << 4) & 0x0fffffff;
                out[6] |=  (1 << 28) | ((in[6] >> 28));
                out[7] = (1 << 28) | (in[6] & 0x0fffffff);
                out[8] = (1 << 28) | ((in[7] >> 4));
                out[9] = (in[7] << 24) & 0x0fffffff;
                out[9] |=  (1 << 28) | ((in[8] >> 8));
                out[10] = (in[8] << 20) & 0x0fffffff;
                out[10] |=  (1 << 28) | ((in[9] >> 12));
                out[11] = (in[9] << 16) & 0x0fffffff;
                out[11] |=  (1 << 28) | ((in[10] >> 16));
                out[12] = (in[10] << 12) & 0x0fffffff;
                out[12] |=  (1 << 28) | ((in[11] >> 20));
                out[13] = (in[11] << 8) & 0x0fffffff;
                out[13] |=  (1 << 28) | ((in[12] >> 24));
                out[14] = (in[12] << 4) & 0x0fffffff;
                out[14] |=  (1 << 28) | ((in[13] >> 28));
                out[15] = (1 << 28) | (in[13] & 0x0fffffff);
                out[16] = (1 << 28) | ((in[14] >> 4));
                out[17] = (in[14] << 24) & 0x0fffffff;
                out[17] |=  (1 << 28) | ((in[15] >> 8));
                out[18] = (in[15] << 20) & 0x0fffffff;
                out[18] |=  (1 << 28) | ((in[16] >> 12));
                out[19] = (in[16] << 16) & 0x0fffffff;
                out[19] |=  (1 << 28) | ((in[17] >> 16));
                out[20] = (in[17] << 12) & 0x0fffffff;
                out[20] |=  (1 << 28) | ((in[18] >> 20));
                out[21] = (in[18] << 8) & 0x0fffffff;
                out[21] |=  (1 << 28) | ((in[19] >> 24));
                out[22] = (in[19] << 4) & 0x0fffffff;
                out[22] |=  (1 << 28) | ((in[20] >> 28));
                out[23] = (1 << 28) | (in[20] & 0x0fffffff);
                out[24] = (1 << 28) | ((in[21] >> 4));
                out[25] = (in[21] << 24) & 0x0fffffff;
                out[25] |=  (1 << 28) | ((in[22] >> 8));
                out[26] = (in[22] << 20) & 0x0fffffff;
                out[26] |=  (1 << 28) | ((in[23] >> 12));
                out[27] = (in[23] << 16) & 0x0fffffff;
                out[27] |=  (1 << 28) | ((in[24] >> 16));
                out[28] = (in[24] << 12) & 0x0fffffff;
                out[28] |=  (1 << 28) | ((in[25] >> 20));
                out[29] = (in[25] << 8) & 0x0fffffff;
                out[29] |=  (1 << 28) | ((in[26] >> 24));
                out[30] = (in[26] << 4) & 0x0fffffff;
                out[30] |=  (1 << 28) | ((in[27] >> 28));
                out[31] = (1 << 28) | (in[27] & 0x0fffffff);
        }
}

void
__vser_unpack29(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 29, out += 32, in += 29) {
                out[0] = (1 << 29) | ((in[0] >> 3));
                out[1] = (in[0] << 26) & 0x1fffffff;
                out[1] |=  (1 << 29) | ((in[1] >> 6));
                out[2] = (in[1] << 23) & 0x1fffffff;
                out[2] |=  (1 << 29) | ((in[2] >> 9));
                out[3] = (in[2] << 20) & 0x1fffffff;
                out[3] |=  (1 << 29) | ((in[3] >> 12));
                out[4] = (in[3] << 17) & 0x1fffffff;
                out[4] |=  (1 << 29) | ((in[4] >> 15));
                out[5] = (in[4] << 14) & 0x1fffffff;
                out[5] |=  (1 << 29) | ((in[5] >> 18));
                out[6] = (in[5] << 11) & 0x1fffffff;
                out[6] |=  (1 << 29) | ((in[6] >> 21));
                out[7] = (in[6] << 8) & 0x1fffffff;
                out[7] |=  (1 << 29) | ((in[7] >> 24));
                out[8] = (in[7] << 5) & 0x1fffffff;
                out[8] |=  (1 << 29) | ((in[8] >> 27));
                out[9] = (in[8] << 2) & 0x1fffffff;
                out[9] |=  (1 << 29) | ((in[9] >> 30));
                out[10] = (1 << 29) | ((in[9] >> 1) & 0x1fffffff);
                out[11] = (in[9] << 28) & 0x1fffffff;
                out[11] |=  (1 << 29) | ((in[10] >> 4));
                out[12] = (in[10] << 25) & 0x1fffffff;
                out[12] |=  (1 << 29) | ((in[11] >> 7));
                out[13] = (in[11] << 22) & 0x1fffffff;
                out[13] |=  (1 << 29) | ((in[12] >> 10));
                out[14] = (in[12] << 19) & 0x1fffffff;
                out[14] |=  (1 << 29) | ((in[13] >> 13));
                out[15] = (in[13] << 16) & 0x1fffffff;
                out[15] |=  (1 << 29) | ((in[14] >> 16));
                out[16] = (in[14] << 13) & 0x1fffffff;
                out[16] |=  (1 << 29) | ((in[15] >> 19));
                out[17] = (in[15] << 10) & 0x1fffffff;
                out[17] |=  (1 << 29) | ((in[16] >> 22));
                out[18] = (in[16] << 7) & 0x1fffffff;
                out[18] |=  (1 << 29) | ((in[17] >> 25));
                out[19] = (in[17] << 4) & 0x1fffffff;
                out[19] |=  (1 << 29) | ((in[18] >> 28));
                out[20] = (in[18] << 1) & 0x1fffffff;
                out[20] |=  (1 << 29) | ((in[19] >> 31));
                out[21] = (1 << 29) | ((in[19] >> 2) & 0x1fffffff);
                out[22] = (in[19] << 27) & 0x1fffffff;
                out[22] |=  (1 << 29) | ((in[20] >> 5));
                out[23] = (in[20] << 24) & 0x1fffffff;
                out[23] |=  (1 << 29) | ((in[21] >> 8));
                out[24] = (in[21] << 21) & 0x1fffffff;
                out[24] |=  (1 << 29) | ((in[22] >> 11));
                out[25] = (in[22] << 18) & 0x1fffffff;
                out[25] |=  (1 << 29) | ((in[23] >> 14));
                out[26] = (in[23] << 15) & 0x1fffffff;
                out[26] |=  (1 << 29) | ((in[24] >> 17));
                out[27] = (in[24] << 12) & 0x1fffffff;
                out[27] |=  (1 << 29) | ((in[25] >> 20));
                out[28] = (in[25] << 9) & 0x1fffffff;
                out[28] |=  (1 << 29) | ((in[26] >> 23));
                out[29] = (in[26] << 6) & 0x1fffffff;
                out[29] |=  (1 << 29) | ((in[27] >> 26));
                out[30] = (in[27] << 3) & 0x1fffffff;
                out[30] |=  (1 << 29) | ((in[28] >> 29));
                out[31] = (1 << 29) | (in[28] & 0x1fffffff);
        }
}

void
__vser_unpack30(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 30, out += 32, in += 30) {
                out[0] = (1 << 30) | ((in[0] >> 2));
                out[1] = (in[0] << 28) & 0x3fffffff;
                out[1] |=  (1 << 30) | ((in[1] >> 4));
                out[2] = (in[1] << 26) & 0x3fffffff;
                out[2] |=  (1 << 30) | ((in[2] >> 6));
                out[3] = (in[2] << 24) & 0x3fffffff;
                out[3] |=  (1 << 30) | ((in[3] >> 8));
                out[4] = (in[3] << 22) & 0x3fffffff;
                out[4] |=  (1 << 30) | ((in[4] >> 10));
                out[5] = (in[4] << 20) & 0x3fffffff;
                out[5] |=  (1 << 30) | ((in[5] >> 12));
                out[6] = (in[5] << 18) & 0x3fffffff;
                out[6] |=  (1 << 30) | ((in[6] >> 14));
                out[7] = (in[6] << 16) & 0x3fffffff;
                out[7] |=  (1 << 30) | ((in[7] >> 16));
                out[8] = (in[7] << 14) & 0x3fffffff;
                out[8] |=  (1 << 30) | ((in[8] >> 18));
                out[9] = (in[8] << 12) & 0x3fffffff;
                out[9] |=  (1 << 30) | ((in[9] >> 20));
                out[10] = (in[9] << 10) & 0x3fffffff;
                out[10] |=  (1 << 30) | ((in[10] >> 22));
                out[11] = (in[10] << 8) & 0x3fffffff;
                out[11] |=  (1 << 30) | ((in[11] >> 24));
                out[12] = (in[11] << 6) & 0x3fffffff;
                out[12] |=  (1 << 30) | ((in[12] >> 26));
                out[13] = (in[12] << 4) & 0x3fffffff;
                out[13] |=  (1 << 30) | ((in[13] >> 28));
                out[14] = (in[13] << 2) & 0x3fffffff;
                out[14] |=  (1 << 30) | ((in[14] >> 30));
                out[15] = (1 << 30) | (in[14] & 0x3fffffff);
                out[16] = (1 << 30) | ((in[15] >> 2));
                out[17] = (in[15] << 28) & 0x3fffffff;
                out[17] |=  (1 << 30) | ((in[16] >> 4));
                out[18] = (in[16] << 26) & 0x3fffffff;
                out[18] |=  (1 << 30) | ((in[17] >> 6));
                out[19] = (in[17] << 24) & 0x3fffffff;
                out[19] |=  (1 << 30) | ((in[18] >> 8));
                out[20] = (in[18] << 22) & 0x3fffffff;
                out[20] |=  (1 << 30) | ((in[19] >> 10));
                out[21] = (in[19] << 20) & 0x3fffffff;
                out[21] |=  (1 << 30) | ((in[20] >> 12));
                out[22] = (in[20] << 18) & 0x3fffffff;
                out[22] |=  (1 << 30) | ((in[21] >> 14));
                out[23] = (in[21] << 16) & 0x3fffffff;
                out[23] |=  (1 << 30) | ((in[22] >> 16));
                out[24] = (in[22] << 14) & 0x3fffffff;
                out[24] |=  (1 << 30) | ((in[23] >> 18));
                out[25] = (in[23] << 12) & 0x3fffffff;
                out[25] |=  (1 << 30) | ((in[24] >> 20));
                out[26] = (in[24] << 10) & 0x3fffffff;
                out[26] |=  (1 << 30) | ((in[25] >> 22));
                out[27] = (in[25] << 8) & 0x3fffffff;
                out[27] |=  (1 << 30) | ((in[26] >> 24));
                out[28] = (in[26] << 6) & 0x3fffffff;
                out[28] |=  (1 << 30) | ((in[27] >> 26));
                out[29] = (in[27] << 4) & 0x3fffffff;
                out[29] |=  (1 << 30) | ((in[28] >> 28));
                out[30] = (in[28] << 2) & 0x3fffffff;
                out[30] |=  (1 << 30) | ((in[29] >> 30));
                out[31] = (1 << 30) | (in[29] & 0x3fffffff);
        }
}

void
__vser_unpack31(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 31, out += 32, in += 31) {
                out[0] = (1 << 31) | ((in[0] >> 1));
                out[1] = (in[0] << 30) & 0x7fffffff;
                out[1] |=  (1 << 31) | ((in[1] >> 2));
                out[2] = (in[1] << 29) & 0x7fffffff;
                out[2] |=  (1 << 31) | ((in[2] >> 3));
                out[3] = (in[2] << 28) & 0x7fffffff;
                out[3] |=  (1 << 31) | ((in[3] >> 4));
                out[4] = (in[3] << 27) & 0x7fffffff;
                out[4] |=  (1 << 31) | ((in[4] >> 5));
                out[5] = (in[4] << 26) & 0x7fffffff;
                out[5] |=  (1 << 31) | ((in[5] >> 6));
                out[6] = (in[5] << 25) & 0x7fffffff;
                out[6] |=  (1 << 31) | ((in[6] >> 7));
                out[7] = (in[6] << 24) & 0x7fffffff;
                out[7] |=  (1 << 31) | ((in[7] >> 8));
                out[8] = (in[7] << 23) & 0x7fffffff;
                out[8] |=  (1 << 31) | ((in[8] >> 9));
                out[9] = (in[8] << 22) & 0x7fffffff;
                out[9] |=  (1 << 31) | ((in[9] >> 10));
                out[10] = (in[9] << 21) & 0x7fffffff;
                out[10] |=  (1 << 31) | ((in[10] >> 11));
                out[11] = (in[10] << 20) & 0x7fffffff;
                out[11] |=  (1 << 31) | ((in[11] >> 12));
                out[12] = (in[11] << 19) & 0x7fffffff;
                out[12] |=  (1 << 31) | ((in[12] >> 13));
                out[13] = (in[12] << 18) & 0x7fffffff;
                out[13] |=  (1 << 31) | ((in[13] >> 14));
                out[14] = (in[13] << 17) & 0x7fffffff;
                out[14] |=  (1 << 31) | ((in[14] >> 15));
                out[15] = (in[14] << 16) & 0x7fffffff;
                out[15] |=  (1 << 31) | ((in[15] >> 16));
                out[16] = (in[15] << 15) & 0x7fffffff;
                out[16] |=  (1 << 31) | ((in[16] >> 17));
                out[17] = (in[16] << 14) & 0x7fffffff;
                out[17] |=  (1 << 31) | ((in[17] >> 18));
                out[18] = (in[17] << 13) & 0x7fffffff;
                out[18] |=  (1 << 31) | ((in[18] >> 19));
                out[19] = (in[18] << 12) & 0x7fffffff;
                out[19] |=  (1 << 31) | ((in[19] >> 20));
                out[20] = (in[19] << 11) & 0x7fffffff;
                out[20] |=  (1 << 31) | ((in[20] >> 21));
                out[21] = (in[20] << 10) & 0x7fffffff;
                out[21] |=  (1 << 31) | ((in[21] >> 22));
                out[22] = (in[21] << 9) & 0x7fffffff;
                out[22] |=  (1 << 31) | ((in[22] >> 23));
                out[23] = (in[22] << 8) & 0x7fffffff;
                out[23] |=  (1 << 31) | ((in[23] >> 24));
                out[24] = (in[23] << 7) & 0x7fffffff;
                out[24] |=  (1 << 31) | ((in[24] >> 25));
                out[25] = (in[24] << 6) & 0x7fffffff;
                out[25] |=  (1 << 31) | ((in[25] >> 26));
                out[26] = (in[25] << 5) & 0x7fffffff;
                out[26] |=  (1 << 31) | ((in[26] >> 27));
                out[27] = (in[26] << 4) & 0x7fffffff;
                out[27] |=  (1 << 31) | ((in[27] >> 28));
                out[28] = (in[27] << 3) & 0x7fffffff;
                out[28] |=  (1 << 31) | ((in[28] >> 29));
                out[29] = (in[28] << 2) & 0x7fffffff;
                out[29] |=  (1 << 31) | ((in[29] >> 30));
                out[30] = (in[29] << 1) & 0x7fffffff;
                out[30] |=  (1 << 31) | ((in[30] >> 31));
                out[31] = (1 << 31) | (in[30] & 0x7fffffff);
        }
}

void
__vser_unpack32(uint32_t *out, uint32_t *in, uint32_t bs)
{
        uint32_t        i;

        for (i = 0; i < bs; i += 32, out += 32, in += 32) {
                out[0] = in[0];
                out[1] = in[1];
                out[2] = in[2];
                out[3] = in[3];
                out[4] = in[4];
                out[5] = in[5];
                out[6] = in[6];
                out[7] = in[7];
                out[8] = in[8];
                out[9] = in[9];
                out[10] = in[10];
                out[11] = in[11];
                out[12] = in[12];
                out[13] = in[13];
                out[14] = in[14];
                out[15] = in[15];
                out[16] = in[16];
                out[17] = in[17];
                out[18] = in[18];
                out[19] = in[19];
                out[20] = in[20];
                out[21] = in[21];
                out[22] = in[22];
                out[23] = in[23];
                out[24] = in[24];
                out[25] = in[25];
                out[26] = in[26];
                out[27] = in[27];
                out[28] = in[28];
                out[29] = in[29];
                out[30] = in[30];
                out[31] = in[31];
        }
}
