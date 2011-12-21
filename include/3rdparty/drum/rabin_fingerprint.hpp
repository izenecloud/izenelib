/*****************************************************************************
 The MIT License

 Copyright (c) 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef IZENELIB_RABIN_FINGERPRINT_HPP
#define IZENELIB_RABIN_FINGERPRINT_HPP

#include <boost/cstdint.hpp>
#include <stdint.h>

/*
 * Rabin Fingerprint - Implementation as suggested by Broder
 * (http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.53.6172)
 *
 * - This is based on http://sourceforge.net/projects/webcat/ and
 *   http://sourceforge.net/projects/rabinhash/. However, I thinks there's a bug in the current
 *   webcat version (16-03-2009).
 * - Make sure your compiler is standard complaint regarding array initialization. When it's in the
 *   initialization list of the constructor, default-initialization should be provided. This means
 *   zero-initialization for int64_t.
 */

class RabinFingerprint
{
public:
    RabinFingerprint();
    explicit RabinFingerprint(uint64_t);

    uint64_t Compute(int64_t const* A, std::size_t size) const;
    uint64_t Compute(int32_t const* A, std::size_t size) const;
    uint64_t Compute(int16_t const* A, std::size_t size) const;
    uint64_t Compute(int8_t const* A, std::size_t size) const;
    uint64_t Compute(char const* A, std::size_t size) const;

private:
    //Disable.
    RabinFingerprint(RabinFingerprint const&);
    RabinFingerprint const& operator=(RabinFingerprint const&);

    void InitTables();
    uint64_t & ComputeTablesSum(uint64_t & w) const;

    //It's not necessary to provide definitions for such integral constant variables as long as their
    //addresses are not taken.
    static int const K = 64; //Degree of the polynomial P.
    static int64_t const T_K_minus_1 = 1LL << (K - 1); //Represents t^(K-1).

    //Bit 64 of the polynomial P is always 1 and not treated directly. This is the polynomial
    //with the leading coefficient removed (lcr).
    uint64_t const p_lcr;

    //Broder's paper presents four pre-computed tables because words are considered to be 32-bit.
    //In this implementation W is a 64-bit integral type. Eight tables are used.
    //Table A is i1^127 + i2^126 + ... + i8^120.
    //Table B is i1^119 + i2^118 + ... + i8^112.
    //Table C, D, ..
    //Table E is i1^95  + i2^94  + ... + i8^88. (This is table A in the paper.)
    //Table F, G, H.
    int64_t tableA_[256]; //Assuming byte size 8.
    int64_t tableB_[256];
    int64_t tableC_[256];
    int64_t tableD_[256];
    int64_t tableE_[256];
    int64_t tableF_[256];
    int64_t tableG_[256];
    int64_t tableH_[256];
};

#endif //IZENELIB_RABIN_FINGERPRINT_HPP
