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

#include <am/succinct/utils.hpp>

#include <vector>
#include <memory>
#include <iostream>

#include <immintrin.h>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dense
{

/*
 * FIXME: rBlock has 32-byte eachs so that its factor
 * is easily aligned to cache-lines. The container needs
 * to align data implicitly(e.g., AlignedVector).
 */
struct rBlock
{
    block_t b0;
    block_t b1;
    size_t rk;
    size_t b0sum;
};

class BitVector
{
public:
    BitVector() : size_(0), none_(0) {}
    ~BitVector() throw() {}

    void init(size_t len)
    {
        __assert(len != 0);
        size_ = len;
        B_.resize((size_ + BSIZE - 1) / BSIZE);
    }

    void set_bit(size_t pos)
    {
        __assert(pos < size_);
        B_[pos / BSIZE] |= block_t(1) << (pos % BSIZE);
        ++none_;
    }

    void unset_bit(size_t pos)
    {
        __assert(pos < size_);
        B_[pos / BSIZE] &= ~(block_t(1) << (pos % BSIZE));
    }

    void change_bit(size_t pos, bool bit)
    {
        if (bit) set_bit(pos);
        else unset_bit(pos);
    }

    bool lookup(size_t pos) const
    {
        __assert(pos < size_);
        return B_[pos / BSIZE] & (block_t(1) << (pos % BSIZE));
    }

    block_t get_block(size_t pos) const
    {
        __assert(pos < B_.size());
        return B_[pos];
    }

    size_t length() const
    {
        return size_;
    }

    size_t bsize() const
    {
        return B_.size();
    }

    size_t get_none() const
    {
        return none_;
    }

    size_t get_nzero() const
    {
        return size_ - none_;
    }

//  void save(std::ostream& ostr) const
//  {
//      ostr.write((const char*)&size_, sizeof(size_));
//      ostr.write((const char*)&none_, sizeof(none_));
//      ostr.write((const char*)&B_[0], sizeof(B_[0]) * B_.size());
//  }

//  void load(std::istream& istr)
//  {
//      istr.read((char*)&size_, sizeof(size_));
//      istr.read((char*)&none_, sizeof(none_));
//      B_.resize((size_ + BSIZE - 1) / BSIZE);
//      istr.read((char*)&B_[0], sizeof(B_[0]) * B_.size());
//  }

private:
    size_t size_;
    size_t none_;
    std::vector<block_t> B_;
}; /* BitVector */

class SuccinctRank;
class SuccinctSelect;

typedef boost::shared_ptr<SuccinctRank>   RankPtr;
typedef boost::shared_ptr<SuccinctSelect> SelectPtr;

class SuccinctRank
{
public:
    SuccinctRank() : size_(0) {}
    explicit SuccinctRank(const BitVector& bv)
        : size_(bv.length())
    {
        init(bv);
    }
    ~SuccinctRank() throw() {}

    size_t rank1(size_t pos) const
    {
        __assert(pos <= size_);

        const rBlock& rblk = rblk_[pos / PRESUM_SZ];

        block_t mask = (block_t(1) << (pos % 64)) - 1;

        /*
         * FIXME: gcc seems to generates a conditional jump, so
         * the code below needs to be replaced with __asm__().
         */
        return rblk.rk + ((pos & 64) ? rblk.b0sum + SuccinctUtils::popcount(rblk.b1 & mask) : SuccinctUtils::popcount(rblk.b0 & mask));
    }

    size_t rank0(size_t pos) const
    {
        return pos - rank1(pos);
    }

    size_t rank(size_t pos, bool bit) const
    {
        return bit ? rank1(pos) : rank0(pos);
    }

    const rBlock& get_rblock(size_t idx) const
    {
        return rblk_[idx];
    }

    rBlock& get_rblock(size_t idx)
    {
        return rblk_[idx];
    }

//  void save(std::ostream& ostr) const
//  {
//      ostr.write((const char*)&size_, sizeof(size_));
//      ostr.write((const char*)&rblk_[0], sizeof(rblk_[0]) * rblk_.size());
//  }

//  void load(std::istream& istr)
//  {
//      istr.read((char*)&size_, sizeof(size_));
//      rblk_.resize(size_ / PRESUM_SZ + 1);
//      istr.read((char*)&rblk_[0], sizeof(rblk_[0]) * rblk_.size());
//  }

private:
    /*--- Private functions below ---*/
    void init(const BitVector& bv)
    {
        size_t bnum = bv.length() / PRESUM_SZ + 1;
        rblk_.resize(bnum);

        size_t r = 0;
        size_t pos = 0;
        for (size_t i = 0; i < bnum; ++i, pos += 2)
        {
            rblk_[i].b0 = pos < bv.bsize() ? bv.get_block(pos) : 0;
            rblk_[i].b1 = pos + 1 < bv.bsize() ? bv.get_block(pos + 1) : 0;
            rblk_[i].rk = r;

            /* b0sum used for select() */
            rblk_[i].b0sum = SuccinctUtils::popcount(rblk_[i].b0);

            r += rblk_[i].b0sum + SuccinctUtils::popcount(rblk_[i].b1);
        }
    }

    size_t size_;
    std::vector<rBlock> rblk_;
}; /* SuccinctRank */

class SuccinctSelect
{
public:
    SuccinctSelect() : bit_(1), size_(0) {}
    SuccinctSelect(const BitVector& bv, RankPtr& rk, bool bit)
        : bit_(bit), size_(0), rk_(rk)
    {
        init(bv);
    }
    ~SuccinctSelect() throw() {}

    size_t select(size_t pos) const
    {
        __assert(pos < size_);

        size_t rpos = rkQ_->rank1(pos) - 1;
        const rBlock& rblk = rk_->get_rblock(rpos);

        size_t rem = pos - cumltv(rblk.rk, rpos * PRESUM_SZ);

        size_t rb = 0;
        block_t blk = 0;

        if (cumltv(rblk.b0sum, BSIZE) > rem)
        {
            blk = block(rblk.b0);
            rb = 0;
        }
        else
        {
            blk = block(rblk.b1);
            rb = BSIZE;
            rem -= cumltv(rblk.b0sum, BSIZE);
        }

        return rpos * PRESUM_SZ + rb + SuccinctUtils::selectBlock(blk, rem);
    }

//  void save(std::ostream& ostr) const
//  {
//      ostr.write((const char*)&bit_, sizeof(bit_));
//      ostr.write((const char*)&size_, sizeof(size_));
//      if (rkQ_) rkQ_->save(ostr);
//  }

//  void load(std::istream& istr)
//  {
//      istr.read((char*)&bit_, sizeof(bit_));
//      istr.read((char*)&size_, sizeof(size_));
//      rkQ_->load(istr);
//  }

private:
    /*--- Private functions below ---*/
    void init(const BitVector& bv)
    {
        block_t   blk;
        BitVector Q;

        size_t sz = bv.length();
        size_t bsize = (sz + BSIZE - 1) / BSIZE;

        /* Calculate sz in advance */
        for (size_t i = 0; i < bsize - 1; ++i)
        {
            blk = block(bv.get_block(i));
            size_ += SuccinctUtils::popcount(blk);
        }

        blk = block(bv.get_block(bsize - 1));
        if (bsize * BSIZE > sz)
        {
            blk &= (block_t(1) << (sz - (bsize - 1) * BSIZE)) - 1;
        }
        size_ += SuccinctUtils::popcount(blk);

        Q.init(size_);

        size_t qcount = 0;
        for (size_t i = 0; i < bsize; ++i)
        {
            if (i % (PRESUM_SZ / BSIZE) == 0)
                Q.set_bit(qcount);

            blk = block(bv.get_block(i));
            qcount += SuccinctUtils::popcount(blk);
        }

        rkQ_.reset(new SuccinctRank(Q));
    }

    inline size_t cumltv(size_t val, size_t pos) const
    {
        return bit_ ? val : pos - val;
    }

    inline block_t block(block_t b) const
    {
        return bit_ ? b : ~b;
    }

    bool    bit_;
    size_t  size_;
    RankPtr rkQ_;

    /*
     * A reference to the rank dictionary
     * of the orignal bit-vector.
     */
    RankPtr rk_;
}; /* SuccinctSelect */

class SuccinctBitVector
{
public:
    SuccinctBitVector() {}
    ~SuccinctBitVector() throw() {}

    /* Functions to initialize */
    void init(size_t size)
    {
        bv_.init(size);
    }

    void build()
    {
        if (bv_.length() == 0)
            throw "Not initialized yet: bv_";

        rk_.reset(new SuccinctRank(bv_));
        st0_.reset(new SuccinctSelect(bv_, rk_, false));
        st1_.reset(new SuccinctSelect(bv_, rk_, true));
    }

    void set_bit(size_t pos)
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        bv_.set_bit(pos);
    }

    void unset_bit(size_t pos)
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        bv_.unset_bit(pos);
    }

    void change_bit(size_t pos, bool bit)
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        bv_.change_bit(bit, pos);
    }

    bool lookup(size_t pos) const
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        return bv_.lookup(pos);
    }

    bool lookup(size_t pos, size_t& rank) const
    {
        if (pos >= bv_.length())
            throw "Invalid input: pos";

        bool bit = bv_.lookup(pos);
        rank = rk_->rank(pos, bit);
        return bit;
    }

    /* Rank & Select operations */
    size_t rank1(size_t pos) const
    {
        if (pos > bv_.length())
            throw "Invalid input: pos";

        return rk_->rank1(pos);
    }

    size_t rank0(size_t pos) const
    {
        if (pos > bv_.length())
            throw "Invalid input: pos";

        return rk_->rank0(pos);
    }

    size_t rank(size_t pos, bool bit) const
    {
        if (pos > bv_.length())
            throw "Invalid input: pos";

        return rk_->rank(pos, bit);
    }

    size_t select1(size_t pos) const
    {
        if (pos >= bv_.get_none())
            throw "Invalid input: pos";

        return st1_->select(pos);
    }

    size_t select0(size_t pos) const
    {
        if (pos >= bv_.get_nzero())
            throw "Invalid input: pos";

        return st0_->select(pos);
    }

    size_t select(size_t pos, bool bit) const
    {
        return bit ? select1(pos) : select0(pos);
    }

    size_t length() const
    {
        return bv_.length();
    }

//  void save(std::ostream& ostr) const
//  {
//      bv_.save(ostr);
//      if (bv_.length() == 0) return;

//      rk_->save(ostr);
//      st0_->save(ostr);
//      st1_->save(ostr);
//  }

//  void load(std::istream& istr)
//  {
//      bv_.load(istr);
//      if (bv_.length() == 0) return;

//      rk_.reset(new SuccinctRank(bv_));
//      st0_.reset(new SuccinctSelect(bv_, rk_, false));
//      st1_.reset(new SuccinctSelect(bv_, rk_, true));
//      rk_->load(istr);
//      st0_->load(istr);
//      st1_->load(istr);
//  }

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
