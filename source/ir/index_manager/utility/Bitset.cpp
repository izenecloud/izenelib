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
    , capacity_(INIT_SIZE / sizeof(uint64_t))
    , bits_(getAlignedArray<uint64_t>(capacity_))
{
}

Bitset::Bitset(size_t len)
    : size_(len)
    , capacity_(block_num(size_))
    , bits_(getAlignedArray<uint64_t>(capacity_))
{
}

Bitset::~Bitset()
{
}

Bitset::Bitset(const Bitset& other)
    : size_(other.size_)
    , capacity_(other.capacity_)
    , bits_(getAlignedArray<uint64_t>(capacity_))
{
    memcpy(bits_.get(), other.bits_.get(), (size_ + 7) / 8);
}

Bitset& Bitset::operator=(const Bitset& other)
{
    if (this != &other)
    {
        size_ = other.size_;
        if (!other.bits_)
        {
            capacity_ = other.capacity_;
            bits_.reset();
        }
        else if (other.capacity_ != capacity_ || !bits_)
        {
            uint64_t* new_data = getAlignedArray<uint64_t>(other.capacity_);
            memcpy(new_data, other.bits_.get(), (other.size_ + 7) / 8);
            bits_.reset(new_data);
            capacity_ = other.capacity_;
        }
        else
        {
            memcpy(bits_.get(), other.bits_.get(), (other.size_ + 7) / 8);
        }
    }

    return *this;
}

bool Bitset::test(size_t pos) const
{
    if (pos >= size_) return false;
    return bits_[pos / 64] & (1 << pos % 64);
}

bool Bitset::any() const
{
    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first != last; ++first)
    {
        if (*first) return true;
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

    const uint64_t* first = bits_.get() + start_blk + 1;
    const uint64_t* last = bits_.get() + end_blk;

    for (; first != last; ++first)
    {
        if (*first) return true;
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

    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first != last; ++first)
    {
        count += __builtin_popcountll(*first);
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

    const uint64_t* first = bits_.get() + start_blk + 1;
    const uint64_t* last = bits_.get() + end_blk;

    for (; first != last; ++first)
    {
        count += __builtin_popcountll(*first);
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
    memset(bits_.get(), 0xFF, block_num(size_) * sizeof(uint64_t));
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
    memset(bits_.get(), 0, block_num(size_) * sizeof(uint64_t));
}

void Bitset::flip(size_t pos)
{
    grow(pos + 1);
    bits_[pos / 64] ^= uint64_t(1) << (pos % 64);
}

void Bitset::flip()
{
    if (size_ == 0) return;

    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(size_);

    for (; first != last; ++first)
    {
        *first = ~*first;
    }

    clear_dirty_bits();
}

size_t Bitset::find_first() const
{
    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first != last; ++first)
    {
        if (*first)
        {
            return (first - bits_.get()) * 64 + __builtin_ctzll(*first);
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

    const uint64_t* first = bits_.get() + i + 1;
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first != last; ++first)
    {
        if (*first)
        {
            return (first - bits_.get()) * 64 + __builtin_ctzll(*first);
        }
    }

    return npos;
}

size_t Bitset::find_last() const
{
    const uint64_t* first = bits_.get() + block_num(size_) - 1;
    const uint64_t* last = bits_.get() - 1;

    for (; first != last; --first)
    {
        if (*first)
        {
            return (first - bits_.get()) * 64 - 1 - __builtin_clzll(*first);
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

    const uint64_t* first = bits_.get() + i - 1;
    const uint64_t* last = bits_.get() - 1;

    for (; first != last; --first)
    {
        if (*first)
        {
            return (first - bits_.get()) * 64 - 1 - __builtin_clzll(*first);
        }
    }

    return npos;
}

bool Bitset::operator==(const Bitset& b) const
{
    if (size_ != b.size_) return false;

    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);
    const uint64_t* first1 = b.bits_.get();

    while (first != last)
    {
        if (*first++ != *first1++) return false;
    }

    return true;
}

bool Bitset::equal_ignore_size(const Bitset& b) const
{
    const size_t blk = block_num(size_);
    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + blk;

    const size_t blk1 = block_num(b.size_);
    const uint64_t* first1 = b.bits_.get();
    const uint64_t* last1 = b.bits_.get() + blk1;

    const size_t min_blk = std::min(blk, blk1);

    for (size_t i = 0; i < min_blk; ++i)
    {
        if (*first++ != *first1++) return false;
    }

    while (first != last)
    {
        if (*first++) return false;
    }

    while (first1 != last1)
    {
        if (*first1++) return false;
    }

    return true;
}

Bitset& Bitset::operator&=(const Bitset& b)
{
    grow(b.size_);

    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(b.size_);
    const uint64_t* first1 = b.bits_.get();

    while (first != last)
    {
        *first++ &= *first1++;
    }

    last = bits_.get() + block_num(size_);
    while (first != last)
    {
        *first++ = 0;
    }

    return *this;
}

Bitset& Bitset::operator|=(const Bitset& b)
{
    grow(b.size_);

    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(b.size_);
    const uint64_t* first1 = b.bits_.get();

    while (first != last)
    {
        *first++ |= *first1++;
    }

    return *this;
}

Bitset& Bitset::operator^=(const Bitset& b)
{
    grow(b.size_);

    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(b.size_);
    const uint64_t* first1 = b.bits_.get();

    while (first != last)
    {
        *first++ ^= *first1++;
    }

    return *this;
}

Bitset& Bitset::operator-=(const Bitset& b)
{
    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(std::min(size_, b.size_));
    const uint64_t* first1 = b.bits_.get();

    while (first != last)
    {
        *first++ &= ~*first1++;
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
    std::swap(capacity_, b.capacity_);
    bits_.swap(b.bits_);
}

void Bitset::read(Directory* pDirectory, const char* name)
{
    IndexInput* pInput = pDirectory->openInput(name);
    grow((size_t)pInput->readInt());
    pInput->read((char*)bits_.get(), block_num(size_) * sizeof(uint64_t));
    delete pInput;
}

void Bitset::write(Directory* pDirectory, const char* name) const
{
    IndexOutput* pOutput = pDirectory->createOutput(name);
    pOutput->writeInt((int32_t)size_);
    pOutput->write((const char*)bits_.get(), block_num(size_) * sizeof(uint64_t));
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
    size_ = size;
    const size_t newBlockNum = block_num(size);
    if (newBlockNum <= capacity_) return;

    size_t capacity = capacity_;
    while (capacity < newBlockNum)
        capacity *= 2;

    uint64_t* new_data = getAlignedArray<uint64_t>(capacity);
    memcpy(new_data, bits_.get(), (size + 7) / 8);
    bits_.reset(new_data);
    capacity_ = capacity;
}

}

NS_IZENELIB_IR_END
