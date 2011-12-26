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

#include <types.h>

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
    RabinFingerprint()
        : p_lcr(0x000000000000001BLL) //Represents t^64 + t^4 + t^3 + t + 1.
        , tableA_(), tableB_(), tableC_(), tableD_(), tableE_(), tableF_(), tableG_(), tableH_()
    {
        this->InitTables();
    }

    explicit RabinFingerprint(uint64_t p)
        : p_lcr(p)
        , tableA_(), tableB_(), tableC_(), tableD_(), tableE_(), tableF_(), tableG_(), tableH_()
    {
        this->InitTables();
    }

    uint64_t Compute(int64_t const* A, std::size_t size) const
    {
        uint64_t w = 0LL;
        for (std::size_t s = 0; s < size; ++s)
        {
            w = this->ComputeTablesSum(w) ^ A[s];
        }
        return w;
    }

    uint64_t Compute(int32_t const* A, std::size_t size) const
    {
        uint64_t w = 0LL;

        //If there's an odd number of elements in vector A, the first element should be accumulate in w.
        std::size_t s = 0;
        if ((size % 2) != 0)
        {
            w = A[0];
            ++s;
        }

        for (; s < size; s += 2)
        {
            w = this->ComputeTablesSum(w) ^
                (static_cast<uint64_t>(A[s]) << 32) ^
                (static_cast<uint64_t>(A[s + 1]));
        }
        return w;
    }

    uint64_t Compute(int16_t const* A, std::size_t size) const
    {
        uint64_t w = 0LL;

        //Accumulate in w elements so the rest of the vector can be split every four elements.
        std::size_t s = 0;
        std::size_t remainder = size % 4;
        for (; s < remainder; ++s)
        {
            w = (w << 16) ^ A[s];
        }

        for (; s < size; s += 4)
        {
            w = this->ComputeTablesSum(w) ^
                (static_cast<uint64_t>(A[s]) << 48) ^
                (static_cast<uint64_t>(A[s + 1]) << 32) ^
                (static_cast<uint64_t>(A[s + 2]) << 14) ^
                (static_cast<uint64_t>(A[s + 3]));
        }
        return w;
    }

    uint64_t Compute(int8_t const* A, std::size_t size) const
    {
        uint64_t w = 0LL;

        //Accumulate in w elements so the rest of the vector can be split every eight elements.
        std::size_t s = 0;
        std::size_t remainder = size % 8;
        for (; s < remainder; ++s)
        {
            w = (w << 8) ^ A[s];
        }

        for (; s < size; s += 8)
        {
            w = this->ComputeTablesSum(w) ^
                (static_cast<uint64_t>(A[s]) << 56) ^
                (static_cast<uint64_t>(A[s + 1]) << 48) ^
                (static_cast<uint64_t>(A[s + 2]) << 40) ^
                (static_cast<uint64_t>(A[s + 3]) << 32) ^
                (static_cast<uint64_t>(A[s + 4]) << 24) ^
                (static_cast<uint64_t>(A[s + 5]) << 16) ^
                (static_cast<uint64_t>(A[s + 6]) << 8) ^
                (static_cast<uint64_t>(A[s + 7]));
        }
        return w;
    }

    uint64_t Compute(char const* A, std::size_t size) const
    {
        //This method assumes an 8-bit char implementation (same size as int8_t). Notice that the C++
        //Standard guarantees that the size of a char is 1 byte. It is implementation-defined, however,
        //how many bits a byte has.

        int8_t const* iA = reinterpret_cast<int8_t const*>(A);
        return this->Compute(iA, size);
    }

private:
    //Disable.
    RabinFingerprint(RabinFingerprint const&);
    RabinFingerprint const& operator=(RabinFingerprint const&);

    void InitTables()
    {
        //This represents t^(k + i) mod P, where i is the index of the array.
        //It will be used to compute the tables.
        uint64_t mods[K];

        //Remember t^k mod P is equivalent to p_lcr.
        mods[0] = p_lcr;
        for (int i = 1; i < K; ++i)
        {
            //By property: t^i mod P = t(t^(i - 1)) mod P.
            mods[i] = mods[i - 1] << 1;

            //If mods[i - 1] had a term at k-1, mods[i] would have had the term k, which is not represented.
            //The term k would account for exactly one more division by P. Then, the effect is the same
            //as adding p_lcr to the mod.
            if ((mods[i - 1] & T_K_minus_1) != 0)
                mods[i] ^= p_lcr;
        }

        //Compute tables. A control variable is used to indicate whether the current bit should be
        //considered.
        for (int i = 0; i < 256; ++i)
        {
            int control = i;
            for (int j = 0; j < 8 && control > 0; ++j)
            {
                if (control & 1) //Ok, consider bit.
                {
                    tableA_[i] ^= mods[j + 56];
                    tableB_[i] ^= mods[j + 48];
                    tableC_[i] ^= mods[j + 40];
                    tableD_[i] ^= mods[j + 32];
                    tableE_[i] ^= mods[j + 24];
                    tableF_[i] ^= mods[j + 16];
                    tableG_[i] ^= mods[j + 8];
                    tableH_[i] ^= mods[j];
                }
                control >>= 1;
            }
        }
    }

    uint64_t & ComputeTablesSum(uint64_t & w) const
    {
        w = tableH_[((w) & 0xFF)] ^
            tableG_[((w >> 8) & 0xFF)] ^
            tableF_[((w >> 16) & 0xFF)] ^
            tableE_[((w >> 24) & 0xFF)] ^
            tableD_[((w >> 32) & 0xFF)] ^
            tableC_[((w >> 40) & 0xFF)] ^
            tableB_[((w >> 48) & 0xFF)] ^
            tableA_[((w >> 56) & 0xFF)];
        return w; //Pass by reference to return the same w. (Convenience and efficiency.)
    }

private:
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
