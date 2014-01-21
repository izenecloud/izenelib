#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <am/bitmap/ewah.h>
#include <am/succinct/utils.hpp>
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

Bitset::Bitset(size_t size)
    : size_(size)
    , capacity_(block_num(size_))
    , bits_(getAlignedArray<uint64_t>(capacity_))
{
}

Bitset::Bitset(size_t size, size_t capacity, const boost::shared_array<uint64_t>& bits)
    : size_(size)
    , capacity_(capacity)
    , bits_(bits)
{
}

Bitset::Bitset(const Bitset& other, bool dup)
    : size_(other.size_)
    , capacity_(other.capacity_)
{
    if (dup)
    {
        bits_.reset(getAlignedArray<uint64_t>(capacity_));
        memcpy(bits_.get(), bits_.get(), (size_ + 7) / 8);
    }
    else
    {
        bits_ = other.bits_;
    }
}

Bitset::~Bitset()
{
}

const Bitset& Bitset::dup()
{
    uint64_t* new_bits = getAlignedArray<uint64_t>(capacity_);
    memcpy(new_bits, bits_.get(), (size_ + 7) / 8);
    bits_.reset(new_bits);

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

    while (first != last)
    {
        if (*first++) return true;
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

    const uint64_t* first = bits_.get() + start_blk;
    const uint64_t* last = bits_.get() + end_blk;

    if (start_blk == end_blk)
    {
        return *first & startMask & endMask;
    }

    if (*first++ & startMask) return true;

    while (first != last)
    {
        if (*first++) return true;
    }

    return endMask && *last & endMask;
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

    while (first != last)
    {
        count += __builtin_popcountll(*first++);
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

    const uint64_t* first = bits_.get() + start_blk;
    const uint64_t* last = bits_.get() + end_blk;

    if (start_blk == end_blk)
    {
        return __builtin_popcountll(*first & startMask & endMask);
    }

    size_t count = __builtin_popcountll(*first++ & startMask);

    while (first != last)
    {
        count += __builtin_popcountll(*first++);
    }

    return count + (endMask ? __builtin_popcountll(*last & endMask) : 0);
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

size_t Bitset::select(size_t ind) const
{
    using izenelib::am::succinct::SuccinctUtils;

    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    size_t bcount = 0;

    for (; first != last; ++first)
    {
        if ((bcount = __builtin_popcountll(*first)) > ind)
        {
            return (first - bits_.get()) * 64 + SuccinctUtils::selectBlock(*first, ind);
        }
        ind -= bcount;
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
