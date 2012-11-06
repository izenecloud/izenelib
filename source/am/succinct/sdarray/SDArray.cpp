/*
 *  Copyright (c) 2010 Daisuke Okanohara
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <am/succinct/sdarray/SDArray.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace sdarray
{

const size_t SDArray::BLOCK_SIZE = 64;

inline size_t log2(size_t x)
{
    return 64 - __builtin_clzl(x);
}

inline size_t popCount(size_t x)
{
    return __builtin_popcountl(x);
}

size_t select(size_t x, size_t r)
{
    assert(r != 0);
    size_t x1 = x - ((x >> 1) & 0x5555555555555555LLU);
    size_t x2 = (x1 & 0x3333333333333333LLU) + ((x1 >> 2) & 0x3333333333333333LLU);
    size_t x3 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0FLLU;

    size_t pos = 0;
    for (;;  pos += 8)
    {
        size_t b = (x3 >> pos) & 0xFFLLU;
        if (r <= b) break;
        r -= b;
    }

    size_t v2 = (x2 >> pos) & 0xFLLU;
    if (r > v2)
    {
        r -= v2;
        pos += 4;
    }

    size_t v1 = (x1 >> pos) & 0x3LLU;
    if (r > v1)
    {
        r -= v1;
        pos += 2;
    }

    size_t v0  = (x >> pos) & 0x1LLU;
    if (r > v0)
    {
        pos += 1;
    }

    return pos;
}

inline size_t getBits(size_t x, size_t beg, size_t num)
{
    return (x >> beg) & ((1LLU << num) - 1);
}

SDArray::SDArray() : size_(), sum_()
{
}

SDArray::~SDArray()
{
}

void SDArray::add(size_t val)
{
    vals_.push_back(val);
    ++size_;

    if (vals_.size() == BLOCK_SIZE)
        build();
}

void SDArray::build()
{
    assert(vals_.size() <= BLOCK_SIZE);
    if (vals_.empty()) return;

    for (size_t i = 1; i < vals_.size(); ++i)
    {
        vals_[i] += vals_[i - 1];
    }

    size_t begPos = B_.size();

    Ltable_.push_back(sum_);
    sum_ += vals_.back();

    // header
    // |-- begPos   (48) --|
    // |-- allZero  ( 1) --|
    // |-- width    ( 7) --|
    // |-- firstSum ( 8) --|
    assert(begPos < 1LLU << 48);
    size_t header = begPos; // use first 48 bit only

    if (vals_.back() == 0)
    {
        header |= (1LLU << 48);
    }
    else
    {
        size_t width = log2(vals_.back() / vals_.size());
        assert(width < (1LLU << 7));

        // All zero special case
        B_.resize(begPos + 2 + (vals_.size() * width + 63) / 64);
        packHighs_(begPos, width);
        packLows_(begPos, width);

        header |= (width << 49);
        size_t firstSum_ = popCount(B_[begPos]);
        assert(firstSum_ < (1LLU << 8));

        header |= firstSum_ << 56;
    }

    Ltable_.push_back(header);
    vals_.clear();
}

void SDArray::clear()
{
    std::vector<size_t>().swap(Ltable_);
    std::vector<size_t>().swap(B_);
    size_ = 0;
    sum_  = 0;
}

size_t SDArray::prefixSum(size_t pos) const
{
    size_t bpos   = pos / BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    size_t sum    = Ltable_[bpos * 2];

    if (offset == 0)
    {
        return sum;
    }
    else
    {
        return sum + selectBlock_(offset, Ltable_[bpos * 2 + 1]);
    }
}

size_t SDArray::prefixSumLookup(size_t pos, size_t& val) const
{
    size_t bpos   = pos / BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    size_t sum    = Ltable_[bpos * 2];
    size_t prev   = 0;

    if (offset == 0)
    {
        prev = 0;
    }
    else
    {
        prev = selectBlock_(offset, Ltable_[bpos * 2 + 1]);
    }

    val = selectBlock_(offset + 1, Ltable_[bpos * 2 + 1]) - prev;

    return sum + prev;
}

size_t SDArray::getVal(size_t pos) const
{
    size_t bpos   = pos / BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    size_t header = Ltable_[bpos * 2 + 1];

    if (offset == 0)
    {
        return selectBlock_(offset + 1, header);
    }
    else
    {
        return selectBlock_(offset + 1, header) - selectBlock_(offset, header);
    }
}

size_t SDArray::find(size_t val) const
{
    if (sum_ == 0 || sum_ < val)
    {
#ifndef NDEBUG
        std::cerr << "come0" << std::endl;
#endif
        return NOTFOUND;
    }

    size_t low  = 0;
    size_t high = Ltable_.size() / 2;
    size_t mid;
    while (low < high)
    {
        mid = low + (high - low) / 2;
        if (val < Ltable_[mid * 2])
            high = mid;
        else
            low = mid + 1;
    }

    if (low * 2 > Ltable_.size())
    {
#ifndef NDEBUG
        std::cerr << "come1" << std::endl;
#endif
        return NOTFOUND;
    }
    if (low == 0) return 0;

    size_t bpos = low - 1;
    assert(Ltable_[bpos * 2] <= val);
    assert((bpos + 1) * 2 >= Ltable_.size() || val < Ltable_[(bpos + 1) * 2]);

    return bpos * BLOCK_SIZE + rankBlock_(val - Ltable_[bpos * 2], Ltable_[bpos * 2 + 1]);
}

size_t SDArray::getSum() const
{
    return sum_;
}

size_t SDArray::size() const
{
    return size_;
}

size_t SDArray::allocSize() const
{
    return sizeof(SDArray) + sizeof(size_t) * (Ltable_.size() + B_.size());
}

void SDArray::save(std::ostream& os) const
{
    os.write((const char *)&size_, sizeof(size_));
    os.write((const char *)&sum_, sizeof(sum_));

    os.write((const char *)&Ltable_[0],  sizeof(Ltable_[0]) * Ltable_.size());

    size_t Bsize = B_.size();
    os.write((const char *)&Bsize, sizeof(Bsize));
    os.write((const char *)&B_[0],  sizeof(B_[0]) * B_.size());
}

void SDArray::load(std::istream& is)
{
    clear();

    is.read((char *)&size_, sizeof(size_));
    is.read((char *)&sum_, sizeof(sum_));

    size_t Lsize = (size_ + BLOCK_SIZE - 1) / BLOCK_SIZE * 2;
    Ltable_.resize(Lsize);
    is.read((char *)&Ltable_[0], sizeof(Ltable_[0]) * Ltable_.size());

    size_t Bsize = 0;
    is.read((char *)&Bsize, sizeof(Bsize));
    B_.resize(Bsize);
    is.read((char *)&B_[0], sizeof(B_[0]) * B_.size());
}

void SDArray::packHighs_(size_t begPos, size_t width)
{
    size_t pos;
    for (size_t i = 0; i < vals_.size(); ++i)
    {
        pos = (vals_[i] >> width) + i;
        B_[begPos + pos / BLOCK_SIZE] |= 1LLU << (pos % BLOCK_SIZE);
    }
}

void SDArray::packLows_(size_t begPos, size_t width)
{
    if (width == 0) return;

    begPos += 2;
    size_t mask = (1LLU << width) - 1;
    size_t val, pos, bpos, offset;
    for (size_t i = 0; i < vals_.size(); ++i)
    {
        val    = vals_[i] & mask;
        pos    = i * width;
        bpos   = pos / BLOCK_SIZE;
        offset = pos % BLOCK_SIZE;

        B_[begPos + bpos] |= val << offset;
        if (offset + width > BLOCK_SIZE)
        {
            B_[begPos + bpos + 1] |= val >> (BLOCK_SIZE - offset);
        }
    }
}

size_t SDArray::selectBlock_(size_t offset, size_t header) const
{
    if (getBits(header, 48, 1))
    {
        // all zero
        return 0;
    }

    size_t begPos   = getBits(header,  0, 48);
    size_t width    = getBits(header, 49,  7);
    size_t firstSum = getBits(header, 56,  8);

    size_t high;
    if (offset <= firstSum)
    {
        high = (select(B_[begPos], offset) + 1 - offset) << width;
    }
    else
    {
        high = (select(B_[begPos + 1], offset - firstSum) + 1 - offset + BLOCK_SIZE) << width;
    }

    return high + getLow_(begPos, offset - 1, width);
}

size_t SDArray::rankBlock_(size_t val, size_t header) const
{
    if (getBits(header, 48, 1))
    {
        // all zero
        return BLOCK_SIZE - 1;
    }

    size_t begPos      = getBits(header,  0, 48);
    size_t width       = getBits(header, 49,  7);
    size_t firstOneSum = getBits(header, 56,  8);

    size_t high = val >> width;
    size_t low  = getBits(val, 0, width);

    size_t firstZeroSum = BLOCK_SIZE - firstOneSum;
    size_t valNum = 0;
    size_t highPos = begPos * BLOCK_SIZE;

    if (high > firstZeroSum)
    {
        valNum += firstOneSum;
        high -= firstZeroSum;
        highPos += BLOCK_SIZE;
    }
    if (high > 0)
    {
        size_t skipNum = select(~B_[highPos / BLOCK_SIZE], high) + 1;
        highPos += skipNum;
        assert(skipNum >= high);
        valNum += skipNum - high;
    }

    size_t cur;
    for (;; ++highPos, ++valNum)
    {
        if (highPos >= (begPos + 2) * BLOCK_SIZE)
        {
            return valNum;
        }
        if (!getBit_(highPos))
        {
            return valNum;
        }

        cur = getLow_(begPos, valNum, width);
        if (cur == low)
        {
            return valNum + 1;
        }
        else if (low < cur)
        {
            return valNum;
        }
    }

    return valNum;
}

size_t SDArray::getLow_(size_t begPos, size_t num, size_t width) const
{
    return getBits_((begPos + 2) * BLOCK_SIZE + num * width, width);
}

bool SDArray::getBit_(size_t pos) const
{
    return (B_[pos / BLOCK_SIZE] >> (pos % BLOCK_SIZE)) & 1LLU;
}

size_t SDArray::getBits_(size_t pos, size_t num) const
{
    size_t bpos   = pos / BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    size_t mask   = (1LLU << num) - 1;

    if (offset + num <= BLOCK_SIZE)
    {
        return (B_[bpos] >> pos) & mask;
    }
    else
    {
        return ((B_[bpos] >> pos) + (B_[bpos + 1] << (BLOCK_SIZE - offset))) & mask;
    }
}

}
}

NS_IZENELIB_AM_END
