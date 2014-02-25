#include <util/compression/simd-compression/simdbitpackinghelpers.h>

namespace SIMDCompression
{

void SIMD_nullunpacker32(const __m128i   *__restrict__ , uint32_t   *__restrict__  out) {
    memset(out, 0, 32 * 4 * 4);
}
void uSIMD_nullunpacker32(const __m128i   *__restrict__ , uint32_t   *__restrict__  out) {
    memset(out, 0, 32 * 4 * 4);
}

void simdunpack(const __m128i   *__restrict__ in, uint32_t   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: SIMD_nullunpacker32(in, out); return;
    case 1: __SIMD_fastunpack1(in, out); return;
    case 2: __SIMD_fastunpack2(in, out); return;
    case 3: __SIMD_fastunpack3(in, out); return;
    case 4: __SIMD_fastunpack4(in, out); return;
    case 5: __SIMD_fastunpack5(in, out); return;
    case 6: __SIMD_fastunpack6(in, out); return;
    case 7: __SIMD_fastunpack7(in, out); return;
    case 8: __SIMD_fastunpack8(in, out); return;
    case 9: __SIMD_fastunpack9(in, out); return;
    case 10: __SIMD_fastunpack10(in, out); return;
    case 11: __SIMD_fastunpack11(in, out); return;
    case 12: __SIMD_fastunpack12(in, out); return;
    case 13: __SIMD_fastunpack13(in, out); return;
    case 14: __SIMD_fastunpack14(in, out); return;
    case 15: __SIMD_fastunpack15(in, out); return;
    case 16: __SIMD_fastunpack16(in, out); return;
    case 17: __SIMD_fastunpack17(in, out); return;
    case 18: __SIMD_fastunpack18(in, out); return;
    case 19: __SIMD_fastunpack19(in, out); return;
    case 20: __SIMD_fastunpack20(in, out); return;
    case 21: __SIMD_fastunpack21(in, out); return;
    case 22: __SIMD_fastunpack22(in, out); return;
    case 23: __SIMD_fastunpack23(in, out); return;
    case 24: __SIMD_fastunpack24(in, out); return;
    case 25: __SIMD_fastunpack25(in, out); return;
    case 26: __SIMD_fastunpack26(in, out); return;
    case 27: __SIMD_fastunpack27(in, out); return;
    case 28: __SIMD_fastunpack28(in, out); return;
    case 29: __SIMD_fastunpack29(in, out); return;
    case 30: __SIMD_fastunpack30(in, out); return;
    case 31: __SIMD_fastunpack31(in, out); return;
    case 32: __SIMD_fastunpack32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

/*assumes that integers fit in the prescribed number of bits*/
void simdpackwithoutmask(const uint32_t   *__restrict__ in, __m128i   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: return;
    case 1: __SIMD_fastpackwithoutmask1(in, out); return;
    case 2: __SIMD_fastpackwithoutmask2(in, out); return;
    case 3: __SIMD_fastpackwithoutmask3(in, out); return;
    case 4: __SIMD_fastpackwithoutmask4(in, out); return;
    case 5: __SIMD_fastpackwithoutmask5(in, out); return;
    case 6: __SIMD_fastpackwithoutmask6(in, out); return;
    case 7: __SIMD_fastpackwithoutmask7(in, out); return;
    case 8: __SIMD_fastpackwithoutmask8(in, out); return;
    case 9: __SIMD_fastpackwithoutmask9(in, out); return;
    case 10: __SIMD_fastpackwithoutmask10(in, out); return;
    case 11: __SIMD_fastpackwithoutmask11(in, out); return;
    case 12: __SIMD_fastpackwithoutmask12(in, out); return;
    case 13: __SIMD_fastpackwithoutmask13(in, out); return;
    case 14: __SIMD_fastpackwithoutmask14(in, out); return;
    case 15: __SIMD_fastpackwithoutmask15(in, out); return;
    case 16: __SIMD_fastpackwithoutmask16(in, out); return;
    case 17: __SIMD_fastpackwithoutmask17(in, out); return;
    case 18: __SIMD_fastpackwithoutmask18(in, out); return;
    case 19: __SIMD_fastpackwithoutmask19(in, out); return;
    case 20: __SIMD_fastpackwithoutmask20(in, out); return;
    case 21: __SIMD_fastpackwithoutmask21(in, out); return;
    case 22: __SIMD_fastpackwithoutmask22(in, out); return;
    case 23: __SIMD_fastpackwithoutmask23(in, out); return;
    case 24: __SIMD_fastpackwithoutmask24(in, out); return;
    case 25: __SIMD_fastpackwithoutmask25(in, out); return;
    case 26: __SIMD_fastpackwithoutmask26(in, out); return;
    case 27: __SIMD_fastpackwithoutmask27(in, out); return;
    case 28: __SIMD_fastpackwithoutmask28(in, out); return;
    case 29: __SIMD_fastpackwithoutmask29(in, out); return;
    case 30: __SIMD_fastpackwithoutmask30(in, out); return;
    case 31: __SIMD_fastpackwithoutmask31(in, out); return;
    case 32: __SIMD_fastpackwithoutmask32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

/*assumes that integers fit in the prescribed number of bits*/
void simdpack(const uint32_t   *__restrict__ in, __m128i   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: return;
    case 1: __SIMD_fastpack1(in, out); return;
    case 2: __SIMD_fastpack2(in, out); return;
    case 3: __SIMD_fastpack3(in, out); return;
    case 4: __SIMD_fastpack4(in, out); return;
    case 5: __SIMD_fastpack5(in, out); return;
    case 6: __SIMD_fastpack6(in, out); return;
    case 7: __SIMD_fastpack7(in, out); return;
    case 8: __SIMD_fastpack8(in, out); return;
    case 9: __SIMD_fastpack9(in, out); return;
    case 10: __SIMD_fastpack10(in, out); return;
    case 11: __SIMD_fastpack11(in, out); return;
    case 12: __SIMD_fastpack12(in, out); return;
    case 13: __SIMD_fastpack13(in, out); return;
    case 14: __SIMD_fastpack14(in, out); return;
    case 15: __SIMD_fastpack15(in, out); return;
    case 16: __SIMD_fastpack16(in, out); return;
    case 17: __SIMD_fastpack17(in, out); return;
    case 18: __SIMD_fastpack18(in, out); return;
    case 19: __SIMD_fastpack19(in, out); return;
    case 20: __SIMD_fastpack20(in, out); return;
    case 21: __SIMD_fastpack21(in, out); return;
    case 22: __SIMD_fastpack22(in, out); return;
    case 23: __SIMD_fastpack23(in, out); return;
    case 24: __SIMD_fastpack24(in, out); return;
    case 25: __SIMD_fastpack25(in, out); return;
    case 26: __SIMD_fastpack26(in, out); return;
    case 27: __SIMD_fastpack27(in, out); return;
    case 28: __SIMD_fastpack28(in, out); return;
    case 29: __SIMD_fastpack29(in, out); return;
    case 30: __SIMD_fastpack30(in, out); return;
    case 31: __SIMD_fastpack31(in, out); return;
    case 32: __SIMD_fastpack32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

void usimdunpack(const __m128i   *__restrict__ in, uint32_t   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: uSIMD_nullunpacker32(in, out); return;
    case 1: __uSIMD_fastunpack1(in, out); return;
    case 2: __uSIMD_fastunpack2(in, out); return;
    case 3: __uSIMD_fastunpack3(in, out); return;
    case 4: __uSIMD_fastunpack4(in, out); return;
    case 5: __uSIMD_fastunpack5(in, out); return;
    case 6: __uSIMD_fastunpack6(in, out); return;
    case 7: __uSIMD_fastunpack7(in, out); return;
    case 8: __uSIMD_fastunpack8(in, out); return;
    case 9: __uSIMD_fastunpack9(in, out); return;
    case 10: __uSIMD_fastunpack10(in, out); return;
    case 11: __uSIMD_fastunpack11(in, out); return;
    case 12: __uSIMD_fastunpack12(in, out); return;
    case 13: __uSIMD_fastunpack13(in, out); return;
    case 14: __uSIMD_fastunpack14(in, out); return;
    case 15: __uSIMD_fastunpack15(in, out); return;
    case 16: __uSIMD_fastunpack16(in, out); return;
    case 17: __uSIMD_fastunpack17(in, out); return;
    case 18: __uSIMD_fastunpack18(in, out); return;
    case 19: __uSIMD_fastunpack19(in, out); return;
    case 20: __uSIMD_fastunpack20(in, out); return;
    case 21: __uSIMD_fastunpack21(in, out); return;
    case 22: __uSIMD_fastunpack22(in, out); return;
    case 23: __uSIMD_fastunpack23(in, out); return;
    case 24: __uSIMD_fastunpack24(in, out); return;
    case 25: __uSIMD_fastunpack25(in, out); return;
    case 26: __uSIMD_fastunpack26(in, out); return;
    case 27: __uSIMD_fastunpack27(in, out); return;
    case 28: __uSIMD_fastunpack28(in, out); return;
    case 29: __uSIMD_fastunpack29(in, out); return;
    case 30: __uSIMD_fastunpack30(in, out); return;
    case 31: __uSIMD_fastunpack31(in, out); return;
    case 32: __uSIMD_fastunpack32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

/*assumes that integers fit in the prescribed number of bits*/
void usimdpackwithoutmask(const uint32_t   *__restrict__ in, __m128i   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: return;
    case 1: __uSIMD_fastpackwithoutmask1(in, out); return;
    case 2: __uSIMD_fastpackwithoutmask2(in, out); return;
    case 3: __uSIMD_fastpackwithoutmask3(in, out); return;
    case 4: __uSIMD_fastpackwithoutmask4(in, out); return;
    case 5: __uSIMD_fastpackwithoutmask5(in, out); return;
    case 6: __uSIMD_fastpackwithoutmask6(in, out); return;
    case 7: __uSIMD_fastpackwithoutmask7(in, out); return;
    case 8: __uSIMD_fastpackwithoutmask8(in, out); return;
    case 9: __uSIMD_fastpackwithoutmask9(in, out); return;
    case 10: __uSIMD_fastpackwithoutmask10(in, out); return;
    case 11: __uSIMD_fastpackwithoutmask11(in, out); return;
    case 12: __uSIMD_fastpackwithoutmask12(in, out); return;
    case 13: __uSIMD_fastpackwithoutmask13(in, out); return;
    case 14: __uSIMD_fastpackwithoutmask14(in, out); return;
    case 15: __uSIMD_fastpackwithoutmask15(in, out); return;
    case 16: __uSIMD_fastpackwithoutmask16(in, out); return;
    case 17: __uSIMD_fastpackwithoutmask17(in, out); return;
    case 18: __uSIMD_fastpackwithoutmask18(in, out); return;
    case 19: __uSIMD_fastpackwithoutmask19(in, out); return;
    case 20: __uSIMD_fastpackwithoutmask20(in, out); return;
    case 21: __uSIMD_fastpackwithoutmask21(in, out); return;
    case 22: __uSIMD_fastpackwithoutmask22(in, out); return;
    case 23: __uSIMD_fastpackwithoutmask23(in, out); return;
    case 24: __uSIMD_fastpackwithoutmask24(in, out); return;
    case 25: __uSIMD_fastpackwithoutmask25(in, out); return;
    case 26: __uSIMD_fastpackwithoutmask26(in, out); return;
    case 27: __uSIMD_fastpackwithoutmask27(in, out); return;
    case 28: __uSIMD_fastpackwithoutmask28(in, out); return;
    case 29: __uSIMD_fastpackwithoutmask29(in, out); return;
    case 30: __uSIMD_fastpackwithoutmask30(in, out); return;
    case 31: __uSIMD_fastpackwithoutmask31(in, out); return;
    case 32: __uSIMD_fastpackwithoutmask32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

void usimdpack(const uint32_t   *__restrict__ in, __m128i   *__restrict__  out, const uint32_t bit) {
    switch (bit) {
    case 0: return;
    case 1: __uSIMD_fastpack1(in, out); return;
    case 2: __uSIMD_fastpack2(in, out); return;
    case 3: __uSIMD_fastpack3(in, out); return;
    case 4: __uSIMD_fastpack4(in, out); return;
    case 5: __uSIMD_fastpack5(in, out); return;
    case 6: __uSIMD_fastpack6(in, out); return;
    case 7: __uSIMD_fastpack7(in, out); return;
    case 8: __uSIMD_fastpack8(in, out); return;
    case 9: __uSIMD_fastpack9(in, out); return;
    case 10: __uSIMD_fastpack10(in, out); return;
    case 11: __uSIMD_fastpack11(in, out); return;
    case 12: __uSIMD_fastpack12(in, out); return;
    case 13: __uSIMD_fastpack13(in, out); return;
    case 14: __uSIMD_fastpack14(in, out); return;
    case 15: __uSIMD_fastpack15(in, out); return;
    case 16: __uSIMD_fastpack16(in, out); return;
    case 17: __uSIMD_fastpack17(in, out); return;
    case 18: __uSIMD_fastpack18(in, out); return;
    case 19: __uSIMD_fastpack19(in, out); return;
    case 20: __uSIMD_fastpack20(in, out); return;
    case 21: __uSIMD_fastpack21(in, out); return;
    case 22: __uSIMD_fastpack22(in, out); return;
    case 23: __uSIMD_fastpack23(in, out); return;
    case 24: __uSIMD_fastpack24(in, out); return;
    case 25: __uSIMD_fastpack25(in, out); return;
    case 26: __uSIMD_fastpack26(in, out); return;
    case 27: __uSIMD_fastpack27(in, out); return;
    case 28: __uSIMD_fastpack28(in, out); return;
    case 29: __uSIMD_fastpack29(in, out); return;
    case 30: __uSIMD_fastpack30(in, out); return;
    case 31: __uSIMD_fastpack31(in, out); return;
    case 32: __uSIMD_fastpack32(in, out); return;
    default: break;
    }
    throw std::logic_error("number of bits is unsupported");
}

}
