/*-----------------------------------------------------------------------------
 *  Simple16.cpp - A implementation of Simple16.
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

#include "util/compression/int/Simple16.hpp"

#define SIMPLE16_LOGDESC        4
#define SIMPLE16_LEN            (1 << SIMPLE16_LOGDESC)

#define SIMPLE16_DESC_FUNC1(num1, log1) \
        bool                            \
        Simple16::try##num1##_##log1##bit(uint32_t *n, uint32_t len)    \
        {                                       \
                uint32_t        i;              \
                uint32_t        min;            \
\
                min = (len < num1)? len : num1; \
\
                for (i = 0; i < min; i++) {     \
                        if (int_utils::get_msb(n[i]) > log1 - 1)        \
                                return false;   \
                }               \
\
                return true;    \
        }

#define SIMPLE16_DESC_FUNC2(num1, log1, num2, log2)     \
        bool                                    \
        Simple16::try##num1##_##log1##bit_##num2##_##log2##bit(uint32_t *n, uint32_t len)       \
        {                                       \
                uint32_t        i;              \
                uint32_t        base;           \
                uint32_t        min;            \
\
                min = (len < num1)? len : num1; \
\
                for (i = 0; i < min; i++) {     \
                        if (int_utils::get_msb(n[i]) > log1 - 1)        \
                                return false;   \
                }               \
\
                base = min;     \
                len -= min;     \
\
                min = (len < num2)? len: num2;                          \
\
                for (i = base; i < base + min; i++) {                   \
                        if (int_utils::get_msb(n[i]) > log2 - 1)        \
                                return false;                           \
                }               \
\
                return true;    \
        }

#define SIMPLE16_DESC_FUNC3(num1, log1, num2, log2, num3, log3) \
        bool                                    \
        Simple16::try##num1##_##log1##bit_##num2##_##log2##bit_##num3##_##log3##bit(uint32_t *n, uint32_t len)    \
        {                                       \
                uint32_t        i;              \
                uint32_t        base;           \
                uint32_t        min;            \
\
                min = (len < num1)? len : num1; \
\
                for (i = 0; i < min; i++) {     \
                        if (int_utils::get_msb(n[i]) > log1 - 1)        \
                                return false;   \
                }               \
\
                base = min;     \
                len -= min;     \
\
                min = (len < num2)? len: num2;                          \
\
                for (i = base; i < base + min; i++) {                   \
                        if (int_utils::get_msb(n[i]) > log2 - 1)        \
                                return false;                           \
                }               \
\
                base += min;    \
                len -= min;     \
\
                min = (len < num3)? len: num3;                          \
\
                for (i = base; i < base + min; i++) {                   \
                        if (int_utils::get_msb(n[i]) > log3 - 1)        \
                                return false;                           \
                }               \
\
                return true;    \
        }

/* Fuction difinition by macros in a trying order */
SIMPLE16_DESC_FUNC1(28, 1);
SIMPLE16_DESC_FUNC2(7, 2, 14, 1);
SIMPLE16_DESC_FUNC3(7, 1, 7, 2, 7, 1);
SIMPLE16_DESC_FUNC2(14, 1, 7, 2);
SIMPLE16_DESC_FUNC1(14, 2);
SIMPLE16_DESC_FUNC2(1, 4, 8, 3);
SIMPLE16_DESC_FUNC3(1, 3, 4, 4, 3, 3);
SIMPLE16_DESC_FUNC1(7, 4);
SIMPLE16_DESC_FUNC2(4, 5, 2, 4);
SIMPLE16_DESC_FUNC2(2, 4, 4, 5);
SIMPLE16_DESC_FUNC2(3, 6, 2, 5);
SIMPLE16_DESC_FUNC2(2, 5, 3, 6);
SIMPLE16_DESC_FUNC1(4, 7);
SIMPLE16_DESC_FUNC2(1, 10, 2, 9);
SIMPLE16_DESC_FUNC1(2, 14);

/* A set of unpacking functions */
static inline void __simple16_unpack1_28(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack2_7_1_14(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack1_7_2_7_1_7(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack1_14_2_7(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack2_14(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack4_1_3_8(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack3_1_4_4_3_3(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack4_7(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack5_4_4_2(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack4_2_5_4(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack6_3_5_2(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack5_2_6_3(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack7_4(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack10_1_9_2(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack14_2(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));
static inline void __simple16_unpack28_1(uint32_t **out, uint32_t **in)
        __attribute__((always_inline));

/* A interface of unpacking functions above */
typedef void (*__simple16_unpacker)(uint32_t **out, uint32_t **in);

static __simple16_unpacker       __simple16_unpack[SIMPLE16_LEN] = {
        __simple16_unpack1_28, __simple16_unpack2_7_1_14,
        __simple16_unpack1_7_2_7_1_7, __simple16_unpack1_14_2_7,
        __simple16_unpack2_14, __simple16_unpack4_1_3_8,
        __simple16_unpack3_1_4_4_3_3, __simple16_unpack4_7,
        __simple16_unpack5_4_4_2, __simple16_unpack4_2_5_4,
        __simple16_unpack6_3_5_2, __simple16_unpack5_2_6_3,
        __simple16_unpack7_4, __simple16_unpack10_1_9_2,
        __simple16_unpack14_2, __simple16_unpack28_1
};

void
Simple16::encodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t &nvalue)
{
        uint32_t        i;
        uint32_t        base;
        uint32_t        min;
        BitsWriter      *wt;

        wt = new BitsWriter(out);

        while (len > 0) {
                if (Simple16::try28_1bit(in, len)) {
                        /* Descripter Number: 0 */
                        wt->bit_writer(0, 4);

                        min = (len < 28)? len : 28;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 1);
                } else if (Simple16::try7_2bit_14_1bit(in, len)) {
                        /* Descripter Number: 1 */
                        wt->bit_writer(1, 4);

                        min = (len < 7)? len : 7;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 2);

                        base = min;
                        min = ((len - base) < 14)? len - base : 14;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 1);

                        min += base;
                } else if (Simple16::try7_1bit_7_2bit_7_1bit(in, len)) {
                        /* Descripter Number: 2 */
                        wt->bit_writer(2, 4);

                        min = (len < 7)? len : 7;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 1);

                        base = min;
                        min = (len - base < 7)? len - base : 7;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 2);

                        base += min;
                        min = (len - base < 7)? len - base : 7;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 1);

                        min += base;
                } else if (Simple16::try14_1bit_7_2bit(in, len)) {
                        /* Descripter Number: 3 */
                        wt->bit_writer(3, 4);

                        min = (len < 14)? len : 14;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 1);

                        base = min;
                        min = ((len - base) < 7)? len - base : 7;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 2);

                        min += base;
                } else if (Simple16::try14_2bit(in, len)) {
                        /* Descripter Number: 4 */
                        wt->bit_writer(4, 4);

                        min = (len < 14)? len : 14;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 2);
                } else if (Simple16::try1_4bit_8_3bit(in, len)) {
                        /* Descripter Number: 5 */
                        wt->bit_writer(5, 4);

                        min = (len < 1)? len : 1;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 4);

                        base = min;
                        min = ((len - base) < 8)? len - base : 8;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 3);

                        min += base;
                } else if (Simple16::try1_3bit_4_4bit_3_3bit(in, len)) {
                        /* Descripter Number: 6 */
                        wt->bit_writer(6, 4);

                        min = (len < 1)? len : 1;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 3);

                        base = min;
                        min = (len - base < 4)? len - base : 4;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 4);

                        base += min;
                        min = (len - base < 3)? len - base : 3;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 3);

                        min += base;
                } else if (Simple16::try7_4bit(in, len)) {
                        /* Descripter Number: 7 */
                        wt->bit_writer(7, 4);

                        min = (len < 7)? len : 7;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 4);
                } else if (Simple16::try4_5bit_2_4bit(in, len)) {
                        /* Descripter Number: 8 */
                        wt->bit_writer(8, 4);

                        min = (len < 4)? len : 4;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 5);

                        base = min;
                        min = ((len - base) < 2)? len - base : 2;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 4);

                        min += base;
                } else if (Simple16::try2_4bit_4_5bit(in, len)) {
                        /* Descripter Number: 9 */
                        wt->bit_writer(9, 4);

                        min = (len < 2)? len : 2;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 4);

                        base = min;
                        min = ((len - base) < 4)? len - base : 4;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 5);

                        min += base;
                } else if (Simple16::try3_6bit_2_5bit(in, len)) {
                        /* Descripter Number: 10 */
                        wt->bit_writer(10, 4);

                        min = (len < 3)? len : 3;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 6);

                        base = min;
                        min = ((len - base) < 2)? len - base : 2;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 5);

                        min += base;
                } else if (Simple16::try2_5bit_3_6bit(in, len)) {
                        /* Descripter Number: 11 */
                        wt->bit_writer(11, 4);

                        min = (len < 2)? len : 2;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 5);

                        base = min;
                        min = ((len - base) < 3)? len - base : 3;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 6);

                        min += base;
                } else if (Simple16::try4_7bit(in, len)) {
                        /* Descripter Number: 12 */
                        wt->bit_writer(12, 4);

                        min = (len < 4)? len : 4;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 7);
                } else if (Simple16::try1_10bit_2_9bit(in, len)) {
                        /* Descripter Number: 13 */
                        wt->bit_writer(13, 4);

                        min = (len < 1)? len : 1;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 10);

                        base = min;
                        min = ((len - base) < 2)? len - base : 2;
                        for (i = base; i < base + min; i++)
                                wt->bit_writer(*in++, 9);

                        min += base;
                } else if (Simple16::try2_14bit(in, len)) {
                        /* Descripter Number: 14 */
                        wt->bit_writer(14, 4);

                        min = (len < 2)? len : 2;
                        for (i = 0; i < min; i++)
                                wt->bit_writer(*in++, 14);
                } else {
                        if ((*in >> 28) > 0)
                                eoutput("Input's out of range: %u", *in);

                        /* Descripter Number: 15 */
                        wt->bit_writer(15, 4);

                        min = 1;
                        wt->bit_writer(*in++, 28);
                }

                /* Align to 32-bit */
                wt->bit_flush();

                len -= min;
        }

        nvalue = wt->written;

        delete wt;
}

void
Simple16::decodeArray(uint32_t *in, uint32_t len,
                uint32_t *out, uint32_t nvalue)
{
        uint32_t        *end;

        end = out + nvalue;

        while (end > out) {
                (__simple16_unpack[*in >>
                 (32 - SIMPLE16_LOGDESC)])(&out, &in);
        }
}

/* --- Intra functions below --- */

void
__simple16_unpack1_28(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 27) & 0x01;
        pout[1] = (pin[0] >> 26) & 0x01;
        pout[2] = (pin[0] >> 25) & 0x01;
        pout[3] = (pin[0] >> 24) & 0x01;
        pout[4] = (pin[0] >> 23) & 0x01;
        pout[5] = (pin[0] >> 22) & 0x01;
        pout[6] = (pin[0] >> 21) & 0x01;
        pout[7] = (pin[0] >> 20) & 0x01;
        pout[8] = (pin[0] >> 19) & 0x01;
        pout[9] = (pin[0] >> 18) & 0x01;
        pout[10] = (pin[0] >> 17) & 0x01;
        pout[11] = (pin[0] >> 16) & 0x01;
        pout[12] = (pin[0] >> 15) & 0x01;
        pout[13] = (pin[0] >> 14) & 0x01;
        pout[14] = (pin[0] >> 13) & 0x01;
        pout[15] = (pin[0] >> 12) & 0x01;
        pout[16] = (pin[0] >> 11) & 0x01;
        pout[17] = (pin[0] >> 10) & 0x01;
        pout[18] = (pin[0] >> 9) & 0x01;
        pout[19] = (pin[0] >> 8) & 0x01;
        pout[20] = (pin[0] >> 7) & 0x01;
        pout[21] = (pin[0] >> 6) & 0x01;
        pout[22] = (pin[0] >> 5) & 0x01;
        pout[23] = (pin[0] >> 4) & 0x01;
        pout[24] = (pin[0] >> 3) & 0x01;
        pout[25] = (pin[0] >> 2) & 0x01;
        pout[26] = (pin[0] >> 1) & 0x01;
        pout[27] = pin[0] & 0x01;

        *in = pin + 1;
        *out = pout + 28;
}

void
__simple16_unpack2_7_1_14(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 26) & 0x03;
        pout[1] = (pin[0] >> 24) & 0x03;
        pout[2] = (pin[0] >> 22) & 0x03;
        pout[3] = (pin[0] >> 20) & 0x03;
        pout[4] = (pin[0] >> 18) & 0x03;
        pout[5] = (pin[0] >> 16) & 0x03;
        pout[6] = (pin[0] >> 14) & 0x03;

        pout[7] = (pin[0] >> 13) & 0x01;
        pout[8] = (pin[0] >> 12) & 0x01;
        pout[9] = (pin[0] >> 11) & 0x01;
        pout[10] = (pin[0] >> 10) & 0x01;
        pout[11] = (pin[0] >> 9) & 0x01;
        pout[12] = (pin[0] >> 8) & 0x01;
        pout[13] = (pin[0] >> 7) & 0x01;
        pout[14] = (pin[0] >> 6) & 0x01;
        pout[15] = (pin[0] >> 5) & 0x01;
        pout[16] = (pin[0] >> 4) & 0x01;
        pout[17] = (pin[0] >> 3) & 0x01;
        pout[18] = (pin[0] >> 2) & 0x01;
        pout[19] = (pin[0] >> 1) & 0x01;
        pout[20] = pin[0] & 0x01;

        *in = pin + 1;
        *out = pout + 21;
}

void
__simple16_unpack1_7_2_7_1_7(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 27) & 0x01;
        pout[1] = (pin[0] >> 26) & 0x01;
        pout[2] = (pin[0] >> 25) & 0x01;
        pout[3] = (pin[0] >> 24) & 0x01;
        pout[4] = (pin[0] >> 23) & 0x01;
        pout[5] = (pin[0] >> 22) & 0x01;
        pout[6] = (pin[0] >> 21) & 0x01;

        pout[7] = (pin[0] >> 19) & 0x03;
        pout[8] = (pin[0] >> 17) & 0x03;
        pout[9] = (pin[0] >> 15) & 0x03;
        pout[10] = (pin[0] >> 13) & 0x03;
        pout[11] = (pin[0] >> 11) & 0x03;
        pout[12] = (pin[0] >> 9) & 0x03;
        pout[13] = (pin[0] >> 7) & 0x03;

        pout[14] = (pin[0] >> 6) & 0x01;
        pout[15] = (pin[0] >> 5) & 0x01;
        pout[16] = (pin[0] >> 4) & 0x01;
        pout[17] = (pin[0] >> 3) & 0x01;
        pout[18] = (pin[0] >> 2) & 0x01;
        pout[19] = (pin[0] >> 1) & 0x01;
        pout[20] = pin[0] & 0x01;

        *in = pin + 1;
        *out = pout + 21;
}

void
__simple16_unpack1_14_2_7(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 27) & 0x01;
        pout[1] = (pin[0] >> 26) & 0x01;
        pout[2] = (pin[0] >> 25) & 0x01;
        pout[3] = (pin[0] >> 24) & 0x01;
        pout[4] = (pin[0] >> 23) & 0x01;
        pout[5] = (pin[0] >> 22) & 0x01;
        pout[6] = (pin[0] >> 21) & 0x01;
        pout[7] = (pin[0] >> 20) & 0x01;
        pout[8] = (pin[0] >> 19) & 0x01;
        pout[9] = (pin[0] >> 18) & 0x01;
        pout[10] = (pin[0] >> 17) & 0x01;
        pout[11] = (pin[0] >> 16) & 0x01;
        pout[12] = (pin[0] >> 15) & 0x01;
        pout[13] = (pin[0] >> 14) & 0x01;

        pout[14] = (pin[0] >> 12) & 0x03;
        pout[15] = (pin[0] >> 10) & 0x03;
        pout[16] = (pin[0] >> 8) & 0x03;
        pout[17] = (pin[0] >> 6) & 0x03;
        pout[18] = (pin[0] >> 4) & 0x03;
        pout[19] = (pin[0] >> 2) & 0x03;
        pout[20] = pin[0] & 0x03;

        *in = pin + 1;
        *out = pout + 21;
}

void
__simple16_unpack2_14(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 26) & 0x03;
        pout[1] = (pin[0] >> 24) & 0x03;
        pout[2] = (pin[0] >> 22) & 0x03;
        pout[3] = (pin[0] >> 20) & 0x03;
        pout[4] = (pin[0] >> 18) & 0x03;
        pout[5] = (pin[0] >> 16) & 0x03;
        pout[6] = (pin[0] >> 14) & 0x03;
        pout[7] = (pin[0] >> 12) & 0x03;
        pout[8] = (pin[0] >> 10) & 0x03;
        pout[9] = (pin[0] >> 8) & 0x03;
        pout[10] = (pin[0] >> 6) & 0x03;
        pout[11] = (pin[0] >> 4) & 0x03;
        pout[12] = (pin[0] >> 2) & 0x03;
        pout[13] = pin[0] & 0x03;

        *in = pin + 1;
        *out = pout + 14;
}

void
__simple16_unpack4_1_3_8(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 24) & 0x0f;

        pout[1] = (pin[0] >> 21) & 0x07;
        pout[2] = (pin[0] >> 18) & 0x07;
        pout[3] = (pin[0] >> 15) & 0x07;
        pout[4] = (pin[0] >> 12) & 0x07;
        pout[5] = (pin[0] >> 9) & 0x07;
        pout[6] = (pin[0] >> 6) & 0x07;
        pout[7] = (pin[0] >> 3) & 0x07;
        pout[8] = pin[0] & 0x07;

        *in = pin + 1;
        *out = pout + 9;
}
void
__simple16_unpack3_1_4_4_3_3(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 25) & 0x07;

        pout[1] = (pin[0] >> 21) & 0x0f;
        pout[2] = (pin[0] >> 17) & 0x0f;
        pout[3] = (pin[0] >> 13) & 0x0f;
        pout[4] = (pin[0] >> 9) & 0x0f;

        pout[5] = (pin[0] >> 6) & 0x07;
        pout[6] = (pin[0] >> 3) & 0x07;
        pout[7] = pin[0] & 0x07;

        *in = pin + 1;
        *out = pout + 8;
}

void
__simple16_unpack4_7(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 24) & 0x0f;
        pout[1] = (pin[0] >> 20) & 0x0f;
        pout[2] = (pin[0] >> 16) & 0x0f;
        pout[3] = (pin[0] >> 12) & 0x0f;
        pout[4] = (pin[0] >> 8) & 0x0f;
        pout[5] = (pin[0] >> 4) & 0x0f;
        pout[6] = pin[0] & 0x0f;

        *in = pin + 1;
        *out = pout + 7;
}

void
__simple16_unpack5_4_4_2(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 23) & 0x1f;
        pout[1] = (pin[0] >> 18) & 0x1f;
        pout[2] = (pin[0] >> 13) & 0x1f;
        pout[3] = (pin[0] >> 8) & 0x1f;

        pout[4] = (pin[0] >> 4) & 0x0f;
        pout[5] = pin[0] & 0x0f;

        *in = pin + 1;
        *out = pout + 6;
}

void
__simple16_unpack4_2_5_4(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 24) & 0x0f;
        pout[1] = (pin[0] >> 20) & 0x0f;

        pout[2] = (pin[0] >> 15) & 0x1f;
        pout[3] = (pin[0] >> 10) & 0x1f;
        pout[4] = (pin[0] >> 5) & 0x1f;
        pout[5] = pin[0] & 0x1f;

        *in = pin + 1;
        *out = pout + 6;
}

void
__simple16_unpack6_3_5_2(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 22) & 0x3f;
        pout[1] = (pin[0] >> 16) & 0x3f;
        pout[2] = (pin[0] >> 10) & 0x3f;

        pout[3] = (pin[0] >> 5) & 0x1f;
        pout[4] = pin[0] & 0x1f;

        *in = pin + 1;
        *out = pout + 5;
}

void
__simple16_unpack5_2_6_3(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 23) & 0x1f;
        pout[1] = (pin[0] >> 18) & 0x1f;

        pout[2] = (pin[0] >> 12) & 0x3f;
        pout[3] = (pin[0] >> 6) & 0x3f;
        pout[4] = pin[0] & 0x3f;

        *in = pin + 1;
        *out = pout + 5;
}

void
__simple16_unpack7_4(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 21) & 0x7f;
        pout[1] = (pin[0] >> 14) & 0x7f;
        pout[2] = (pin[0] >> 7) & 0x7f;
        pout[3] = pin[0] & 0x7f;

        *in = pin + 1;
        *out = pout + 4;
}

void
__simple16_unpack10_1_9_2(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 18) & 0x03ff;

        pout[1] = (pin[0] >> 9) & 0x01ff;
        pout[2] = pin[0] & 0x01ff;

        *in = pin + 1;
        *out = pout + 3;
}

void
__simple16_unpack14_2(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = (pin[0] >> 14) & 0x3fff;
        pout[1] = pin[0] & 0x3fff;

        *in = pin + 1;
        *out = pout + 2;
}

void
__simple16_unpack28_1(uint32_t **out, uint32_t **in)
{
        uint32_t        *pout;
        uint32_t        *pin;

        pout = *out;
        pin = *in;

        pout[0] = pin[0] & 0x0fffffff;

        *in = pin + 1;
        *out = pout + 1;
}
