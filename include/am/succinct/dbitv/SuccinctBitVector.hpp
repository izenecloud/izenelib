/*-----------------------------------------------------------------------------
 *  SuccinctBitVector.hpp - A x86/64 optimized rank/select dictionary for dense bit-arrays 
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2012 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 * =================
 *
 * Overview
 *-----------
 * This code is orignated from Okanohara's implimentation below:

 * http://codezine.jp/article/detail/260

 * Some techniques to harness x86/64 platforms are put into the code.
 * Aligned pre-computed values with cache-lines
 * Striped bit-vectors with these values
 * [Pending] Branch-free processing with SIMD instructions

 *More information can be found in
 *http://www.slideshare.net/maropu0804/a-x86optimized-rankselect-dictionary-for-bit-sequences

 *-----------------------------------------------------------------------------
 */

#ifndef __IZENELIB_AM_DBITV_SUCCINCTBITVECTOR_HPP__
#define __IZENELIB_AM_DBITV_SUCCINCTBITVECTOR_HPP__

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <vector>
#include <memory>
#include <types.h>

#include <glog/logging.h>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dense
{

#if  defined(__GNUC__) && __GNUC_PREREQ(2, 2)
#define  __USE_POSIX_MEMALIGN__
#endif

/* Defined by BSIZE */
typedef uint32_t  block_t;

#define BYTE2DWORD(x)     ((x) >> 2)
#define DWORD2BYTE(x)     ((x) << 2)
#define LOCATE_BPOS(pos)  ((pos + BSIZE - 1) / BSIZE)

/* Block size (32-bit environment) */
const size_t BSIZE = 32;
const size_t CACHELINE_SZ = 16;
const size_t SIMD_ALIGN = 4;

const size_t LEVEL1_NUM = 256;
const size_t LEVEL2_NUM = BSIZE;

static uint32_t popcount(block_t b)
{
    return __builtin_popcount(b);
}

static uint8_t selectPos8(uint32_t d, int r)
{
    CHECK(r < 8);

    if (d == 0 && r == 0)
        return 0;

    uint32_t ret = 0;

    /* NOTE: A input for bsf MUST NOT be 0 */
    for (int i = 0; i < r + 1; i++, d ^= 1 << ret)
        __asm__("bsf %1, %0;" :"=r"(ret) :"r"(d));

    return ret;
}

static uint32_t selectPos(block_t blk, uint32_t r)
{
    CHECK(r < 32);

    uint32_t nblock = 0;
    uint32_t cnt = 0;

    while (nblock < 4)
    {
        cnt = popcount((blk >> nblock * 8) & 0xff);

        if (r < cnt)
            break;

        r -= cnt;
        nblock++;
    }

    return nblock * 8 +
           selectPos8((blk >> (nblock * 8)) & 0xff, r);
}

#ifdef __USE_POSIX_MEMALIGN__
static uint32_t *cachealign_alloc(uint64_t size)
{
    CHECK(size != 0);
    uint32_t  *p;
    /* NOTE: *lev2 is 64B-aligned so as to avoid cache-misses */
    uint32_t ret = posix_memalign((void **)&p,
                                  DWORD2BYTE(CACHELINE_SZ), DWORD2BYTE(size));
    return (ret == 0)? p : NULL;
}

static void cachealign_free(uint32_t *p)
{
    if (p == NULL) return;
    free(p);
}
#else
static uint32_t *cachealign_alloc(uint64_t size)
{
    CHECK(size != 0);
    /* FIXME: *lev2 NEEDS to be 64B-aligned. */
    uint32_t *p = new uint32_t[size + CACHELINE_SZ];
    return p;
}

static void cachealign_free(uint32_t *p)
{
    if (p == NULL) return;
    delete[] p;
}
#endif /* __USE_POSIX_MEMALIGN__ */

class BitVector
{
public:
    BitVector() : size_(0), none_(0) {}
    ~BitVector() throw() {}

    void init(uint64_t len)
    {
        CHECK(len != 0);

        size_ = len;

        B_.resize(LOCATE_BPOS(size_));
        for (uint64_t i = 0; i < B_.size(); i++)
            B_[i] = 0;
    }

    void set_bit(uint8_t bit, uint64_t pos)
    {
        CHECK(pos < size_);

        if (bit)
            B_[pos / BSIZE] |= 1U << (pos % BSIZE);
        else
            B_[pos / BSIZE] &= (~(1U << (pos % BSIZE)));

        none_++;
    }

    bool lookup(uint64_t pos) const
    {
        CHECK(pos < size_);
        return (B_[pos / BSIZE] & (0x01 << (pos % BSIZE))) > 0;
    }

    block_t get_block(uint64_t pos) const
    {
        CHECK(pos < size_);
        return B_[pos];
    }

    uint64_t length() const
    {
        return size_;
    }

    uint64_t get_none() const
    {
        return none_;
    }

private:
    uint64_t  size_;
    uint64_t  none_;
    std::vector<block_t>  B_;
}; /* BitVector */

class SuccinctRank;
class SuccinctSelect;

typedef boost::shared_ptr<SuccinctRank>   RankPtr;
typedef boost::shared_ptr<SuccinctSelect> SelectPtr;

class SuccinctRank
{
public:
    SuccinctRank() : size_(0) {};
    explicit SuccinctRank(const BitVector& bv) :
        size_(bv.length()), mem_(cachealign_alloc(
                                     CACHELINE_SZ * ((bv.length() / LEVEL1_NUM) + 1)),
                                 cachealign_free)
    {
        init(bv);
    };
    ~SuccinctRank() throw() {};

    uint32_t rank(uint32_t pos, uint32_t bit) const
    {
        CHECK(pos < size_);

        pos++;

        if (bit)
            return rank1(pos);
        else
            return pos - rank1(pos);
    }

    boost::shared_ptr<uint32_t> get_mem() const
    {
        return  mem_;
    }

private:
    /*--- Private functions below ---*/
    void init(const BitVector& bv)
    {
        uint32_t  *lev1p = NULL;
        uint8_t   *lev2p = NULL;
        block_t   *lev3p = NULL;

        uint32_t idx = 0, nbits = 0;

        do
        {
            if (idx % LEVEL1_NUM == 0)
            {
                lev1p = mem_.get() + CACHELINE_SZ * (idx / LEVEL1_NUM);
                lev2p = reinterpret_cast<uint8_t *>(lev1p + SIMD_ALIGN);
                lev3p = reinterpret_cast<block_t *>(
                            lev1p + SIMD_ALIGN + BYTE2DWORD(LEVEL1_NUM / LEVEL2_NUM));

                *lev1p = nbits;
            }

            if (idx % LEVEL2_NUM == 0)
            {
                CHECK(nbits - *lev1p <= UINT8_MAX);

                *lev2p++ = static_cast<uint8_t>(nbits - *lev1p);
                block_t blk = bv.get_block(idx / BSIZE);
                memcpy(lev3p++, &blk, sizeof(block_t));
            }

            if (idx % BSIZE == 0)
                nbits += popcount(bv.get_block(idx / BSIZE));
        }
        while (idx++ <= size_);

        /* Put some tricky code here for SIMD instructions */
        for (uint32_t i = idx; i % LEVEL1_NUM != 0; i++)
        {
            if (i % LEVEL2_NUM == 0)
                *lev2p++ = static_cast<uint8_t>(UINT8_MAX);
        }
    }

    uint32_t rank1(uint32_t pos) const
    {
        CHECK(pos <= size_);

        uint32_t *lev1p = mem_.get() + CACHELINE_SZ * (pos / LEVEL1_NUM);
        uint8_t *lev2p = reinterpret_cast<uint8_t *>(lev1p + SIMD_ALIGN);

        uint32_t offset = (pos / LEVEL2_NUM) % (LEVEL1_NUM /LEVEL2_NUM);

        CHECK(offset < LEVEL1_NUM / LEVEL2_NUM);

        uint32_t r = *lev1p + *(lev2p + offset);

        block_t *blk = reinterpret_cast<block_t *>(lev1p + SIMD_ALIGN +
                       BYTE2DWORD(LEVEL1_NUM / LEVEL2_NUM)) + offset;
        uint32_t rem = (pos % LEVEL2_NUM) % BSIZE;

        r += popcount((*blk) & ((1ULL << rem) - 1));

        return r;
    }

    uint32_t  size_;
    boost::shared_ptr<uint32_t> mem_;
}; /* SuccinctRank */

class SuccinctSelect
{
public:
    SuccinctSelect() : size_(0) {};
    explicit SuccinctSelect(const BitVector& bv,
                            uint8_t bit, RankPtr& rk) :
        size_(0), bit_(bit), rk_(rk)
    {
        init(bv);
    };
    ~SuccinctSelect() throw() {};

    uint32_t select(uint32_t pos) const
    {
        CHECK(pos < size_);

        /* Search the position on LEVEL1 */
        uint32_t lev1pos = rkQ_->rank(pos, 1) - 1;

        uint32_t *lev1p = rk_->get_mem().get() + CACHELINE_SZ * lev1pos;

        CHECK(pos >= cumltv(*lev1p, lev1pos * LEVEL1_NUM, bit_));

        uint8_t lpos = pos - cumltv(*lev1p, lev1pos * LEVEL1_NUM, bit_);

        /* Search the position on LEVEL2 */
        uint8_t *lev2p = reinterpret_cast<uint8_t *>(lev1p + SIMD_ALIGN);

        uint32_t lev2pos = 0;

        do
        {
            if (cumltv(*(lev2p + lev2pos + 1),
                       (lev2pos + 1) * LEVEL2_NUM, bit_) > lpos) break;
        }
        while (++lev2pos < LEVEL1_NUM / LEVEL2_NUM - 1);

        /* Count the left bits */
        CHECK(lpos >= cumltv(*(lev2p + lev2pos), lev2pos * LEVEL2_NUM, bit_));

        uint32_t rem = lpos - cumltv(*(lev2p + lev2pos),
                                     lev2pos * LEVEL2_NUM, bit_);

        block_t *blk = static_cast<block_t *>(
                           lev1p + SIMD_ALIGN + BYTE2DWORD(LEVEL1_NUM / LEVEL2_NUM)) + lev2pos;

        return lev1pos * LEVEL1_NUM + lev2pos * LEVEL2_NUM +
               selectPos(block(*blk, bit_), rem);
    }

private:
    /*--- Private functions below ---*/
    void init(const BitVector& bv)
    {
        block_t   blk;
        BitVector Q;

        uint64_t sz = bv.length();
        uint32_t bsize = (sz + BSIZE - 1) / BSIZE;

        /* Calculate sz in advance */
        for (uint32_t i = 0; i < bsize; i++)
        {
            blk = block(bv.get_block(i), bit_);

            if ((i + 1) * BSIZE > sz)
            {
                uint32_t rem = sz - i * BSIZE;
                blk &= ((1 << rem) - 1);
            }

            size_ += popcount(blk);
        }

        Q.init(size_);

        uint32_t qcount = 0;
        for (uint32_t i = 0; i < bsize; i++)
        {
            if (i % (LEVEL1_NUM / BSIZE) == 0)
                Q.set_bit(1, qcount);

            blk = block(bv.get_block(i), bit_);
            qcount += popcount(blk);
        }

        RankPtr rkQ(new SuccinctRank(Q));
        rkQ_ = rkQ;
    }

    uint32_t cumltv(uint32_t val, uint32_t pos,
                    uint8_t bit) const
    {
        if (bit)
            return val;
        else
            return pos - val;
    }

    block_t block(block_t b, uint8_t bit) const
    {
        if (bit)
            return b;
        else
            return ~b;
    }

    uint32_t  size_;
    uint32_t  bit_;
    RankPtr   rkQ_;

    /*
    * A reference to the rank dictionary
    * of the orignal bit-vector.
    */
    RankPtr   rk_;
}; /* SuccinctSelect */

class SuccinctBitVector
{
public:
    SuccinctBitVector() : rk_((SuccinctRank *)0),
        st0_((SuccinctSelect *)0), st1_((SuccinctSelect *)0) {};
    ~SuccinctBitVector() throw() {};

    /* Functions to initialize */
    void init(uint64_t size)
    {
        bv_.init(size);
    }

    void build()
    {
        if (bv_.length() == 0)
            throw "Not initialized yet: bv_";

        RankPtr rk(new SuccinctRank(bv_));
        SelectPtr st0(new SuccinctSelect(bv_, 0, rk));
        SelectPtr st1(new SuccinctSelect(bv_, 1, rk));

        rk_ = rk, st0_ = st0, st1_ = st1;
    }

    void set_bit(uint8_t bit, uint64_t pos)
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";
        if (bit > 1)
            throw "Invalid input: bit";

        bv_.set_bit(bit, pos);
    }

    bool lookup(uint64_t pos) const
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        return bv_.lookup(pos);
    }

    /* Rank & Select operations */
    uint32_t rank(uint32_t pos, uint8_t bit) const
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";
        if (bit > 1)
            throw "Invalid input: bit";

        return rk_->rank(pos, bit);
    }

    uint32_t select(uint32_t pos, uint8_t bit) const
    {
        if (pos >= bv_.get_none())
            throw "Invalid input: pos";
        if (bit > 1)
            throw "Invalid input: bit";

        if (bit)
            return st1_->select(pos);
        else
            return st0_->select(pos);
    }

private:
    /* A sequence of bit-array */
    BitVector bv_;

    /* A rank/select dictionary for dense */
    RankPtr rk_;
    SelectPtr st0_;
    SelectPtr st1_;
}; /* SuccinctBitVector */

} /* dense */
} /* succinct */

NS_IZENELIB_AM_END

#endif /* __IZENELIB_AM_DBITV_SUCCINCTBITVECTOR_HPP__ */
