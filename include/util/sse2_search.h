#ifndef HAVE_SSE2_DEFINES_SEARCH_H
#define HAVE_SSE2_DEFINES_SEARCH_H

typedef int v4si __attribute__ ((vector_size (16)));
typedef short v8hi __attribute__ ((vector_size (16)));
typedef char v16qi __attribute__ ((vector_size (16)));


// arr is ASC
inline unsigned int simd_liner_search_Nobranch(
                    const unsigned int *arr,
                    unsigned int n,
                    unsigned int key,
                    unsigned int index = 0)
{
    v4si *in_data = (v4si*)arr;
    unsigned int i = index;
    i >>= 2;
    unsigned int res;
    v4si key4 = { (int)key, (int)key, (int)key, (int)key };
    for (;;) {
        v4si cmp0 = __builtin_ia32_pcmpgtd128 (key4, in_data [i + 0]);
        v4si cmp1 = __builtin_ia32_pcmpgtd128 (key4, in_data [i + 1]);
        v4si cmp2 = __builtin_ia32_pcmpgtd128 (key4, in_data [i + 2]);
        v4si cmp3 = __builtin_ia32_pcmpgtd128 (key4, in_data [i + 3]);

        v8hi pack01 = __builtin_ia32_packssdw128 (cmp0, cmp1);
        v8hi pack23 = __builtin_ia32_packssdw128 (cmp2, cmp3);
        v16qi pack0123 = __builtin_ia32_packsswb128 (pack01, pack23);

        res = __builtin_ia32_pmovmskb128 (pack0123);

        if (res != 0xffff)
        break;

        i += 4;
    }

    return i * 4 + __builtin_ctz (~res);
}

//arr is DESC
inline unsigned int simd_liner_search_Nobranch_rev(
                    const unsigned int *arr,
                    unsigned int n,
                    unsigned int key,
                    unsigned int index = 0)
{
    v4si *in_data = (v4si*)arr;
    unsigned int i = index;
    i >>= 2;
    unsigned int res;
    v4si key4 = { (int)key, (int)key, (int)key, (int)key };
    for (;;) {
        v4si cmp0 = __builtin_ia32_pcmpgtd128 (in_data [i + 0], key4);
        v4si cmp1 = __builtin_ia32_pcmpgtd128 (in_data [i + 1], key4);
        v4si cmp2 = __builtin_ia32_pcmpgtd128 (in_data [i + 2], key4);
        v4si cmp3 = __builtin_ia32_pcmpgtd128 (in_data [i + 3], key4);

        v8hi pack01 = __builtin_ia32_packssdw128 (cmp0, cmp1);
        v8hi pack23 = __builtin_ia32_packssdw128 (cmp2, cmp3);
        v16qi pack0123 = __builtin_ia32_packsswb128 (pack01, pack23);

        res = __builtin_ia32_pmovmskb128 (pack0123);

        if (res != 0xffff)
        break;

        i += 4;
    }

    return i * 4 + __builtin_ctz (~res);
}

#endif
