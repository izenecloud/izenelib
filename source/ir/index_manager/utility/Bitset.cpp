#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <am/bitmap/ewah.h>
#include <boost/detail/endian.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

Bitset::Bitset()
    : size_(0)
    , bits_(INIT_SIZE / sizeof(uint64_t))
{
}

Bitset::Bitset(const Bitset& other)
    : size_(other.size_)
    , bits_(other.bits_)
{
}

Bitset::Bitset(size_t len)
    : size_(len)
    , bits_(block_num(size_))
{
}

Bitset::~Bitset()
{
}

bool Bitset::test(size_t pos) const
{
    if (pos >= size_) return false;
    return bits_[pos / 64] & (1 << pos % 64);
}

bool Bitset::any() const
{
    const size_t blk = block_num(size_);
    for (size_t i = 0; i < blk; ++i)
    {
        if (bits_[i]) return true;
    }

    return false;
}

bool Bitset::any(size_t start, size_t end) const
{
    end = std::min(end, size_);
    if (start >= end) return false;

    uint64_t startMask = ~uint64_t(0) << (start % 64);
    uint64_t endMask = ~(~uint64_t(0) << (end % 64));
    size_t start_blk = start / 64;
    size_t end_blk = end / 64;

    if (start_blk == end_blk)
    {
        return bits_[start_blk] & startMask & endMask;
    }

    if (bits_[start_blk] & startMask) return true;

    for (size_t i = start_blk + 1; i < end_blk; ++i)
    {
        if (bits_[i]) return true;
    }

    if (endMask && bits_[end_blk] & endMask) return true;

    return false;
}

bool Bitset::none() const
{
    return !any();
}

size_t Bitset::count() const
{
    size_t count = 0;

    const size_t blk = block_num(size_);
    for (size_t i = 0; i < blk; ++i)
    {
        count += __builtin_popcountll(bits_[i]);
    }

    return count;
}

size_t Bitset::count(size_t start, size_t end) const
{
    end = std::min(end, size_);
    if (start >= end) return 0;

    uint64_t startMask = ~uint64_t(0) << (start % 64);
    uint64_t endMask = ~(~uint64_t(0) << (end % 64));
    size_t start_blk = start / 64;
    size_t end_blk = end / 64;

    if (start_blk == end_blk)
    {
        return __builtin_popcountll(bits_[start_blk] & startMask & endMask);
    }

    size_t count = __builtin_popcountll(bits_[start_blk] & startMask);

    for (size_t i = start_blk + 1; i < end_blk; ++i)
    {
        count += __builtin_popcountll(bits_[i]);
    }

    if (endMask)
    {
        count += __builtin_popcountll(bits_[end_blk] & endMask);
    }

    return count;
}

void Bitset::set(size_t pos)
{
    grow(pos + 1);
    bits_[pos / 64] |= uint64_t(1) << (pos % 64);
}

void Bitset::set()
{
    if (size_ == 0) return;
    memset(&bits_[0], 0xFF, block_num(size_) * sizeof(uint64_t));
    clear_dirty_bits();
}

void Bitset::reset(size_t pos)
{
    grow(pos + 1);
    bits_[pos / 64] &= ~(uint64_t(1) << (pos % 64));
}

void Bitset::reset()
{
    if (size_ == 0) return;
    memset(&bits_[0], 0, block_num(size_) * sizeof(uint64_t));
}

void Bitset::flip()
{
    if (size_ == 0) return;

    const size_t blk = block_num(size_);
    for (size_t i = 0; i < blk; ++i)
    {
        bits_[i] = ~bits_[i];
    }

    clear_dirty_bits();
}

size_t Bitset::find_first() const
{
    const size_t blk = block_num(size_);
    for (size_t i = 0; i < blk; ++i)
    {
        if (bits_[i])
        {
            return i * 64 + __builtin_ctzll(bits_[i]);
        }
    }

    return npos;
}


size_t Bitset::find_next(size_t pos) const
{

    if (++pos >= size_) return npos;

    size_t i = pos / 64;
    const uint64_t fore = bits_[i] >> (pos % 64);

    if (fore) return pos + __builtin_ctzll(fore);

    const size_t blk = block_num(size_);
    for (++i; i < blk; ++i)
    {
        if (bits_[i])
        {
            return i * 64 + __builtin_ctzll(bits_[i]);
        }
    }

    return npos;
}

size_t Bitset::find_last() const
{
    const size_t blk = block_num(size_);
    for (size_t i = blk; i > 0; --i)
    {
        if (bits_[i - 1])
        {
            return i * 64 - 1 - __builtin_clzll(bits_[i - 1]);
        }
    }

    return npos;
}


size_t Bitset::find_prev(size_t pos) const
{

    if (pos == 0) return npos;
    if (--pos >= size_) return find_last();

    size_t i = pos / 64;
    const uint64_t fore = bits_[i] << (63 - pos % 64);

    if (fore) return pos - __builtin_clzll(fore);

    for (; i > 0; --i)
    {
        if (bits_[i - 1])
        {
            return i * 64 - 1 - __builtin_clzll(bits_[i - 1]);
        }
    }

    return npos;
}

bool Bitset::operator!=(const Bitset& b) const
{
    if (size_ != b.size_) return false;
    const size_t blk = block_num(size_);
    for (size_t i = 0; i < blk; ++i)
        if (bits_[i] != b.bits_[i]) return false;

    return true;
}

bool Bitset::equal_ignore_size(const Bitset& b) const
{
    const size_t blk = block_num(std::min(size_, b.size_));
    for (size_t i = 0; i < blk; ++i)
        if (bits_[i] != b.bits_[i]) return false;

    if (size_ > b.size_)
    {
        const size_t max_blk = block_num(size_);
        for (size_t i = blk + 1; i < max_blk; ++i)
            if (bits_[i]) return false;
    }
    else
    {
        const size_t max_blk = block_num(b.size_);
        for (size_t i = blk + 1; i < max_blk; ++i)
            if (b.bits_[i]) return false;
    }

    return true;
}

Bitset& Bitset::operator&=(const Bitset& b)
{
    grow(b.size_);

    const size_t blk = block_num(b.size_);
    for (size_t i = 0; i < blk; ++i)
    {
        bits_[i] &= b.bits_[i];
    }

    return *this;
}

Bitset& Bitset::operator|=(const Bitset& b)
{
    grow(b.size_);

    const size_t blk = block_num(b.size_);
    for (size_t i = 0; i < blk; ++i)
    {
        bits_[i] |= b.bits_[i];
    }

    return *this;
}

Bitset& Bitset::operator^=(const Bitset& b)
{
    grow(b.size_);

    const size_t blk = block_num(b.size_);
    for (size_t i = 0; i < blk; ++i)
    {
        bits_[i] ^= b.bits_[i];
    }

    return *this;
}

Bitset& Bitset::operator-=(const Bitset& b)
{
    size_t min_size = std::min(size_, b.size_);

    const size_t blk = block_num(min_size);
    for (size_t i = 0; i < blk; ++i)
    {
        bits_[i] &= ~b.bits_[i];
    }

    return *this;
}

size_t Bitset::size() const
{
    return size_;
}

void Bitset::swap(Bitset& b)
{
    std::swap(size_, b.size_);
    bits_.swap(b.bits_);
}

void Bitset::read(Directory* pDirectory, const char* name)
{
    IndexInput* pInput = pDirectory->openInput(name);
    grow((size_t)pInput->readInt());
    memset(&bits_[0], 0 , bits_.size() * sizeof(uint64_t));
    pInput->read((char*)&bits_[0], block_num(size_) * sizeof(uint64_t));
    delete pInput;
}

void Bitset::write(Directory* pDirectory, const char* name) const
{
    IndexOutput* pOutput = pDirectory->createOutput(name);
    pOutput->writeInt((int32_t)size_);
    pOutput->write((const char*)&bits_[0], block_num(size_) * sizeof(uint64_t));
    delete pOutput;
}

void Bitset::clear_dirty_bits()
{
    if (size_ % 64)
        bits_[size_ / 64] &= ~(~uint64_t(0) << (size_ % 64));
}

void Bitset::grow(size_t size)
{
    if (size <= size_) return;
    const size_t newBlockNum = block_num(size);
    if (newBlockNum <= bits_.size()) return;

    size_t capacity = bits_.size();
    while (capacity < newBlockNum)
        capacity <<= 1;

    bits_.resize(capacity);
    size_ = size;
}

}

NS_IZENELIB_IR_END
