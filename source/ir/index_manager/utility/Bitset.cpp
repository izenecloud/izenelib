#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <am/bitmap/ewah.h>
#include <am/succinct/utils.hpp>

#include <boost/detail/endian.hpp>
#include <immintrin.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

Bitset::Bitset()
    : size_(0)
    , capacity_(INIT_SIZE / sizeof(uint64_t))
    , bits_(cachealign_alloc<uint64_t>(capacity_), cachealign_deleter())
{
    memset(bits_.get(), 0, capacity_ * sizeof(uint64_t));
}

Bitset::Bitset(size_t size)
    : size_(size)
    , capacity_(block_num(size_))
    , bits_(cachealign_alloc<uint64_t>(capacity_), cachealign_deleter())
{
    memset(bits_.get(), 0, capacity_ * sizeof(uint64_t));
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
        bits_.reset(cachealign_alloc<uint64_t>(capacity_), cachealign_deleter());
        memset(bits_.get(), 0, capacity_ * sizeof(uint64_t));
        memcpy(bits_.get(), other.bits_.get(), (size_ + 7) / 8);
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
    uint64_t* new_bits = cachealign_alloc<uint64_t>(capacity_);
    memset(new_bits, 0, capacity_ * sizeof(uint64_t));
    memcpy(new_bits, bits_.get(), (size_ + 7) / 8);
    bits_.reset(new_bits, cachealign_deleter());

    return *this;
}


bool Bitset::test(size_t pos) const
{
    if (pos >= size_) return false;
    return bits_[pos / 64] & (uint64_t(1) << pos % 64);
}

bool Bitset::any() const
{
    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first < last - 3; first += 4)
    {
        if (first[0] || first[1] || first[2] || first[3]) return true;
    }

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

    const uint64_t* first = bits_.get() + start / 64;
    const uint64_t* last = bits_.get() + end / 64;

    if (first == last)
    {
        return *first & startMask & endMask;
    }

    if (*first++ & startMask) return true;

    for (; first < last - 3; first += 4)
    {
        if (first[0] || first[1] || first[2] || first[3]) return true;
    }

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

    for (; first < last - 3; first += 4)
    {
        count += _mm_popcnt_u64(first[0]);
        count += _mm_popcnt_u64(first[1]);
        count += _mm_popcnt_u64(first[2]);
        count += _mm_popcnt_u64(first[3]);
    }

    while (first != last)
    {
        count += _mm_popcnt_u64(*first++);
    }

    return count;
}

size_t Bitset::count(size_t start, size_t end) const
{
    end = std::min(end, size_);
    if (start >= end) return 0;

    uint64_t startMask = ~uint64_t(0) << (start % 64);
    uint64_t endMask = ~(~uint64_t(0) << (end % 64));

    const uint64_t* first = bits_.get() + start / 64;
    const uint64_t* last = bits_.get() + end / 64;

    if (first == last)
    {
        return _mm_popcnt_u64(*first & startMask & endMask);
    }

    size_t count = _mm_popcnt_u64(*first++ & startMask);

    for (; first < last - 3; first += 4)
    {
        count += _mm_popcnt_u64(first[0]);
        count += _mm_popcnt_u64(first[1]);
        count += _mm_popcnt_u64(first[2]);
        count += _mm_popcnt_u64(first[3]);
    }

    while (first != last)
    {
        count += _mm_popcnt_u64(*first++);
    }

    return count + (endMask ? _mm_popcnt_u64(*last & endMask) : 0);
}

void Bitset::set(size_t pos)
{
    grow(pos + 1);
    bits_[pos / 64] |= uint64_t(1) << (pos % 64);
}

void Bitset::set(size_t start, size_t end)
{
    if (start >= end) return;
    grow(end);

    uint64_t startMask = ~uint64_t(0) << (start % 64);
    uint64_t endMask = ~(~uint64_t(0) << (end % 64));

    uint64_t* first = bits_.get() + start / 64;
    uint64_t* last = bits_.get() + end / 64;

    if (first == last)
    {
        *first |= startMask & endMask;
        return;
    }

    *first++ |= startMask;

    memset(first, 0xFF, (char*)last - (char*)first);

    if (endMask) *last |= endMask;
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

void Bitset::reset(size_t start, size_t end)
{
    if (start >= end) return;
    grow(end);

    uint64_t startMask = ~(~uint64_t(0) << (start % 64));
    uint64_t endMask = ~uint64_t(0) << (end % 64);

    uint64_t* first = bits_.get() + start / 64;
    uint64_t* last = bits_.get() + end / 64;

    if (first == last)
    {
        *first &= startMask | endMask;
        return;
    }

    *first++ &= startMask;

    memset(first, 0, (char*)last - (char*)first);

    if (~endMask) *last &= endMask;
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

void Bitset::flip(size_t start, size_t end)
{
    if (start >= end) return;
    grow(end);

    uint64_t startMask = ~uint64_t(0) << (start % 64);
    uint64_t endMask = ~(~uint64_t(0) << (end % 64));

    uint64_t* first = bits_.get() + start / 64;
    uint64_t* last = bits_.get() + end / 64;

    if (first == last)
    {
        *first ^= startMask & endMask;
        return;
    }

    *first++ ^= startMask;

    for (; first < last - 3; first += 4)
    {
        first[0] = ~first[0];
        first[1] = ~first[1];
        first[2] = ~first[2];
        first[3] = ~first[3];
    }

    for (; first != last; ++first)
    {
        first[0] = ~first[0];
    }

    if (endMask) *last ^= endMask;
}

void Bitset::flip()
{
    if (size_ == 0) return;

    uint64_t* first = bits_.get();
    uint64_t* last = bits_.get() + block_num(size_);

    for (; first < last - 3; first += 4)
    {
        first[0] = ~first[0];
        first[1] = ~first[1];
        first[2] = ~first[2];
        first[3] = ~first[3];
    }

    for (; first != last; ++first)
    {
        first[0] = ~first[0];
    }

    clear_dirty_bits();
}

void Bitset::copy_segment(const void* src, size_t start, size_t len)
{
    if (len == 0) return;
    grow(start + len);

    uint64_t startMask = ~(~uint64_t(0) << (start % 64));
    uint64_t endMask = ~uint64_t(0) << ((start + len) % 64);

    uint64_t* first = bits_.get() + start / 64;
    uint64_t* last = bits_.get() + (start + len) / 64;

    size_t offset0 = start % 64;
    size_t offset1 = 64 - offset0;

    if (first == last)
    {
        first[0] = (first[0] & (startMask | endMask)) | (tail_bits(src, len) << offset0);
        return;
    }

    if (start % 8 == 0)
    {
        char* dest = (char*)bits_.get() + start / 8;
        memcpy(dest, src, (char*)last - dest);
        if (~endMask)
        {
            last[0] = (last[0] & endMask) | tail_bits(src, len);
        }
        return;
    }

    const uint64_t* data = (const uint64_t*)src;

    first[0] = (first[0] & startMask) | (data[0] << offset0);

    for (++first; first < last - 3; first += 4, data += 4)
    {
        first[0] = (data[0] >> offset1) | (data[1] << offset0);
        first[1] = (data[1] >> offset1) | (data[2] << offset0);
        first[2] = (data[2] >> offset1) | (data[3] << offset0);
        first[3] = (data[3] >> offset1) | (data[4] << offset0);
    }

    for (; first != last; ++data)
    {
        *first++ = (data[0] >> offset1) | (data[1] << offset0);
    }

    if (~endMask)
    {
        last[0] = (last[0] & endMask) | (data[0] >> offset1);
        if ((start + len) % 64 > offset0)
        {
            last[0] |= tail_bits(src, len) << offset0;
        }
    }
}

size_t Bitset::find_first() const
{
    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    for (; first < last - 3; first += 4)
    {
        if (first[0]) return (first - bits_.get()) * 64 + __builtin_ctzll(first[0]);
        if (first[1]) return (first - bits_.get() + 1) * 64 + __builtin_ctzll(first[1]);
        if (first[2]) return (first - bits_.get() + 2) * 64 + __builtin_ctzll(first[2]);
        if (first[3]) return (first - bits_.get() + 3) * 64 + __builtin_ctzll(first[3]);
    }

    for (; first != last; ++first)
    {
        if (first[0]) return (first - bits_.get()) * 64 + __builtin_ctzll(first[0]);
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

    for (; first < last - 3; first += 4)
    {
        if (first[0]) return (first - bits_.get()) * 64 + __builtin_ctzll(first[0]);
        if (first[1]) return (first - bits_.get() + 1) * 64 + __builtin_ctzll(first[1]);
        if (first[2]) return (first - bits_.get() + 2) * 64 + __builtin_ctzll(first[2]);
        if (first[3]) return (first - bits_.get() + 3) * 64 + __builtin_ctzll(first[3]);
    }

    for (; first != last; ++first)
    {
        if (first[0]) return (first - bits_.get()) * 64 + __builtin_ctzll(first[0]);
    }

    return npos;
}

size_t Bitset::find_last() const
{
    const uint64_t* first = bits_.get() + block_num(size_) - 1;
    const uint64_t* last = bits_.get() - 1;

    for (; first > last + 3; first -= 4)
    {
        if (first[0]) return (first - bits_.get()) * 64 + 63 - __builtin_clzll(first[0]);
        if (first[-1]) return (first - bits_.get()) * 64 - 1 - __builtin_clzll(first[-1]);
        if (first[-2]) return (first - bits_.get()) * 64 - 65 - __builtin_clzll(first[-2]);
        if (first[-3]) return (first - bits_.get()) * 64 - 129 - __builtin_clzll(first[-3]);
    }

    for (; first != last; --first)
    {
        if (first[0]) return (first - bits_.get()) * 64 + 63 - __builtin_clzll(first[0]);
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

    for (; first > last + 3; first -= 4)
    {
        if (first[0]) return (first - bits_.get()) * 64 + 63 - __builtin_clzll(first[0]);
        if (first[-1]) return (first - bits_.get()) * 64 - 1 - __builtin_clzll(first[-1]);
        if (first[-2]) return (first - bits_.get()) * 64 - 65 - __builtin_clzll(first[-2]);
        if (first[-3]) return (first - bits_.get()) * 64 - 129 - __builtin_clzll(first[-3]);
    }

    for (; first != last; --first)
    {
        if (first[0]) return (first - bits_.get()) * 64 + 63 - __builtin_clzll(first[0]);
    }

    return npos;
}

size_t Bitset::select(size_t ind) const
{
    using izenelib::am::succinct::SuccinctUtils;

    const uint64_t* first = bits_.get();
    const uint64_t* last = bits_.get() + block_num(size_);

    size_t bcount = 0;

    for (; first < last - 3; first += 4)
    {
        if ((bcount = _mm_popcnt_u64(first[0])) > ind)
        {
            return (first - bits_.get()) * 64 + SuccinctUtils::selectBlock(first[0], ind);
        }
        ind -= bcount;

        if ((bcount = _mm_popcnt_u64(*(first + 1))) > ind)
        {
            return (first - bits_.get() + 1) * 64 + SuccinctUtils::selectBlock(first[1], ind);
        }
        ind -= bcount;

        if ((bcount = _mm_popcnt_u64(*(first + 2))) > ind)
        {
            return (first - bits_.get() + 2) * 64 + SuccinctUtils::selectBlock(first[2], ind);
        }
        ind -= bcount;

        if ((bcount = _mm_popcnt_u64(*(first + 3))) > ind)
        {
            return (first - bits_.get() + 3) * 64 + SuccinctUtils::selectBlock(first[3], ind);
        }
        ind -= bcount;
    }

    for (; first != last; ++first)
    {
        if ((bcount = _mm_popcnt_u64(first[0])) > ind)
        {
            return (first - bits_.get()) * 64 + SuccinctUtils::selectBlock(first[0], ind);
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

    for (; first < last - 3; first += 4, first1 += 4)
    {
        if (first[0] != first1[0]) return false;
        if (first[1] != first1[1]) return false;
        if (first[2] != first1[2]) return false;
        if (first[3] != first1[3]) return false;
    }

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

    for (; first < last - 3; first += 4, first1 += 4)
    {
        first[0] |= first1[0];
        first[1] |= first1[1];
        first[2] |= first1[2];
        first[3] |= first1[3];
    }

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

    for (; first < last - 3; first += 4, first1 += 4)
    {
        first[0] ^= first1[0];
        first[1] ^= first1[1];
        first[2] ^= first1[2];
        first[3] ^= first1[3];
    }

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

    for (; first < last - 3; first += 4, first1 += 4)
    {
        first[0] &= ~first1[0];
        first[1] &= ~first1[1];
        first[2] &= ~first1[2];
        first[3] &= ~first1[3];
    }

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

uint64_t Bitset::tail_bits(const void* data, size_t len) const
{
    uint64_t val = 0;
    size_t bits = len % 64;
    size_t bytes = (bits + 7) / 8;
    memcpy(&val, (const uint64_t*)data + len / 64, bytes);
    val &= ~(~uint64_t(0) << bits);

    return val;
}

void Bitset::grow(size_t size)
{
    if (size <= size_) return;

    const size_t newBlockNum = block_num(size);
    if (newBlockNum <= capacity_)
    {
        size_ = size;
        return;
    }

    size_t capacity = std::max(size_t(1), capacity_);

    while (capacity < newBlockNum)
        capacity *= 2;

    uint64_t* new_data = cachealign_alloc<uint64_t>(capacity);
    memset(new_data, 0, capacity * sizeof(uint64_t));
    if (size_) memcpy(new_data, bits_.get(), (size_ + 7) / 8);
    bits_.reset(new_data, cachealign_deleter());
    capacity_ = capacity;
    size_ = size;
}

}

NS_IZENELIB_IR_END
