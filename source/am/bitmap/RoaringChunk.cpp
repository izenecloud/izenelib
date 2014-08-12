#include <am/bitmap/RoaringChunk.h>

#include <immintrin.h>

NS_IZENELIB_AM_BEGIN

RoaringChunk::RoaringChunk()
    : key_(0)
    , cardinality_(0)
    , updatable_(false)
{
}

RoaringChunk::RoaringChunk(uint32_t key, uint32_t value)
    : key_(key)
    , cardinality_(1)
    , updatable_(true)
{
    chunk_.reset(cachealign_alloc<uint32_t>(INIT_ARRAY_SIZE / 2 + 4), cachealign_deleter());
    //memset(&chunk_[4], 0, 16);
    chunk_[0] = ARRAY;
    chunk_[1] = INIT_ARRAY_SIZE;
    chunk_[2] = INIT_ARRAY_SIZE / 2 + 4;
    chunk_[4] = value;
}

RoaringChunk::RoaringChunk(uint32_t key, Type type, uint32_t capacity)
    : key_(key)
    , cardinality_(0)
    , updatable_(false)
{
    switch (type)
    {
    case ARRAY:
        capacity = (capacity + 1) / 2;
        chunk_.reset(cachealign_alloc<uint32_t>(capacity + 4), cachealign_deleter());
        //memset(&chunk_[4], 0, capacity * 4);
        chunk_[0] = ARRAY;
        chunk_[1] = capacity * 2;
        chunk_[2] = capacity + 4;
        break;

    case BITMAP:
        chunk_.reset(cachealign_alloc<uint32_t>(BITMAP_SIZE + 4), cachealign_deleter());
        memset(&chunk_[4], 0, BITMAP_SIZE * 4);
        chunk_[0] = BITMAP;
        chunk_[1] = MAX_CHUNK_CAPACITY;
        chunk_[2] = BITMAP_SIZE + 4;
        break;

    case FULL:
        chunk_.reset(new uint32_t[4]());
        chunk_[0] = FULL;
        chunk_[1] = MAX_CHUNK_CAPACITY;
        chunk_[2] = 4;
        cardinality_ = MAX_CHUNK_CAPACITY;
        break;

    default: break;
    }
}

RoaringChunk::~RoaringChunk()
{
}

boost::shared_array<uint32_t> RoaringChunk::getChunk() const
{
    if (!updatable_) return chunk_;

    while (flag_.test_and_set());
    boost::shared_array<uint32_t> tmp_chunk(chunk_);
    flag_.clear();

    return tmp_chunk;
}

RoaringChunk::RoaringChunk(const RoaringChunk& b, bool clone)
    : key_(b.key_)
    , cardinality_(b.cardinality_)
    , updatable_(false)
{
    const boost::shared_array<uint32_t>& old_chunk = b.getChunk();
    if (clone)
    {
        chunk_.reset(cachealign_alloc<uint32_t>(old_chunk[2]), cachealign_deleter());
        memcpy(&chunk_[0], &old_chunk[0], old_chunk[2] * 4);
    }
    else
    {
        chunk_ = old_chunk;
    }
}

RoaringChunk& RoaringChunk::operator=(const RoaringChunk& b)
{
    key_ = b.key_;
    cardinality_ = b.cardinality_;
    updatable_ = false;
    chunk_ = b.getChunk();

    return *this;
}

bool RoaringChunk::operator==(const RoaringChunk& b) const
{
    return key_ == b.key_
        && cardinality_ == b.cardinality_
        && updatable_ == b.updatable_
        && chunk_ == b.chunk_;
}

void RoaringChunk::add(uint32_t x)
{
    switch (chunk_[0])
    {
    case BITMAP:
        assert(x >= cardinality_);
        if (++cardinality_ < MAX_CHUNK_CAPACITY)
        {
            chunk_[x / 64] |= 1ULL << x;
        }
        else
        {
            boost::shared_array<uint32_t> new_chunk(new uint32_t[4]());
            new_chunk[0] = FULL;
            new_chunk[1] = MAX_CHUNK_CAPACITY;
            new_chunk[2] = 4;

            chunk_.swap(new_chunk);
        }
        break;

    case ARRAY:
        if (cardinality_ < chunk_[1])
        {
            uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
            data[cardinality_++] = x;
        }
        else
        {
            uint32_t new_capacity = chunk_[1] < 64 ? chunk_[1]
                : chunk_[1] < 1024 ? (chunk_[1] * 3 + 2) / 4
                : (chunk_[1] * 5 + 4) / 8;

            if (new_capacity < BITMAP_SIZE)
            {
                boost::shared_array<uint32_t> new_chunk(cachealign_alloc<uint32_t>(new_capacity + 4), cachealign_deleter());
                //memset(&new_chunk[4], 0, new_capacity * 4);
                new_chunk[0] = ARRAY;
                new_chunk[1] = new_capacity * 2;
                new_chunk[2] = new_capacity + 4;

                memcpy(&new_chunk[4], &chunk_[4], cardinality_ * 2);
                uint16_t* data = reinterpret_cast<uint16_t*>(&new_chunk[4]);
                data[cardinality_++] = x;

                chunk_.swap(new_chunk);
            }
            else
            {
                boost::shared_array<uint32_t> new_chunk(cachealign_alloc<uint32_t>(BITMAP_SIZE + 4), cachealign_deleter());
                memset(&new_chunk[4], 0, BITMAP_SIZE * 4);
                new_chunk[0] = BITMAP;
                new_chunk[1] = MAX_CHUNK_CAPACITY;
                new_chunk[2] = BITMAP_SIZE + 4;

                uint64_t* data = reinterpret_cast<uint64_t*>(&new_chunk[4]);
                const uint16_t* old_data = reinterpret_cast<uint16_t*>(&chunk_[4]);
                for (uint32_t i = 0; i < chunk_[1]; ++i)
                {
                    data[old_data[i] / 64] |= 1ULL << old_data[i];
                }
                new_chunk[x / 64] |= 1ULL << x;
                ++cardinality_;

                chunk_.swap(new_chunk);
            }
        }
        break;

    default: break;
    }
}

RoaringChunk RoaringChunk::operator~() const
{
    uint32_t new_cardinality = MAX_CHUNK_CAPACITY - cardinality_;
    if (new_cardinality == 0)
    {
        return RoaringChunk();
    }
    else if (new_cardinality < MAX_ARRAY_SIZE)
    {
        RoaringChunk answer(key_, ARRAY, new_cardinality);

        uint16_t* new_data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
        const uint64_t* old_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
        for (uint32_t k = 0, pos = 0; k < 1024 && pos < new_cardinality; ++k)
        {
            uint64_t block = ~old_data[k];
            while (block)
            {
                new_data[pos++] = uint16_t(k * 64 + __builtin_ctzll(block));
                block &= ~-block;
            }
        }

        answer.cardinality_ = new_cardinality;

        return answer;
    }
    else if (cardinality_ > 0)
    {
        RoaringChunk answer(key_, BITMAP, new_cardinality);

        if (cardinality_ < 4096)
        {
            uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
            const uint16_t* old_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);

            memset(data, 0xff, BITMAP_SIZE * 4);
            for (uint32_t i = 0; i < cardinality_; ++i)
            {
                data[old_data[i] / 64] ^= 1ULL << old_data[i];
            }
        }
        else
        {
            uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
            const uint64_t* old_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);

            for (uint32_t i = 0; i < 1024; ++i)
            {
                data[i] = ~old_data[i];
            }
        }

        answer.cardinality_ = new_cardinality;

        return answer;
    }
    else
    {
        return RoaringChunk(key_, FULL, 0);
    }
}

RoaringChunk RoaringChunk::operator&(const RoaringChunk& b) const
{
    if (!cardinality_ | !b.cardinality_) return RoaringChunk();

    uint32_t type = chunk_[0] * 3 + b.chunk_[0];
    switch (type)
    {
    case 0: return and_ArrayArray(b);
    case 1: return b.and_BitmapArray(*this);
    case 2: return *this;
    case 3: return and_BitmapArray(b);
    case 4: return and_BitmapBitmap(b);
    case 5: return *this;
    case 6:
    case 7:
    case 8: return b;
    }

    assert(false);
    return *this;
}

RoaringChunk RoaringChunk::operator|(const RoaringChunk& b) const
{
    if (!cardinality_) return b;
    if (!b.cardinality_) return *this;

    uint32_t type = chunk_[0] * 3 + b.chunk_[0];
    switch (type)
    {
    case 0: return or_ArrayArray(b);
    case 1: return b.or_BitmapArray(*this);
    case 2: return b;
    case 3: return or_BitmapArray(b);
    case 4: return or_BitmapBitmap(b);
    case 5: return b;
    case 6:
    case 7:
    case 8: return *this;
    }

    assert(false);
    return *this;
}

RoaringChunk RoaringChunk::operator^(const RoaringChunk& b) const
{
    if (!cardinality_) return b;
    if (!b.cardinality_) return *this;

    uint32_t type = chunk_[0] * 3 + b.chunk_[0];
    switch (type)
    {
    case 0: return xor_ArrayArray(b);
    case 1: return b.xor_BitmapArray(*this);
    case 2: return ~*this;
    case 3: return xor_BitmapArray(b);
    case 4: return xor_BitmapBitmap(b);
    case 5: return ~*this;
    case 6:
    case 7:
    case 8: return ~b;
    }

    assert(false);
    return *this;
}

RoaringChunk RoaringChunk::operator-(const RoaringChunk& b) const
{
    if (!cardinality_) return RoaringChunk();
    if (!b.cardinality_) return *this;

    uint32_t type = chunk_[0] * 3 + b.chunk_[0];
    switch (type)
    {
    case 0: return andNot_ArrayArray(b);
    case 1: return andNot_ArrayBitmap(b);
    case 2: return RoaringChunk();
    case 3: return andNot_BitmapArray(b);
    case 4: return andNot_BitmapBitmap(b);
    case 5: return RoaringChunk();
    case 6:
    case 7:
    case 8: return ~b;
    }

    assert(false);
    return *this;
}

void RoaringChunk::convertFromBitmapToArray()
{
    uint32_t new_capacity = (cardinality_ + 1) / 2;
    boost::shared_array<uint32_t> new_chunk(cachealign_alloc<uint32_t>(new_capacity + 4), cachealign_deleter());
    //memset(&new_chunk[4], 0, new_capacity * 4);
    new_chunk[0] = ARRAY;
    new_chunk[1] = new_capacity * 2;
    new_chunk[2] = new_capacity + 4;

    uint16_t* new_data = reinterpret_cast<uint16_t*>(&new_chunk[4]);
    const uint64_t* old_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    for (uint32_t k = 0, pos = 0; k < 1024 && pos < cardinality_; ++k)
    {
        uint64_t block = old_data[k];
        while (block)
        {
            new_data[pos++] = uint16_t(k * 64 + __builtin_ctzll(block));
            block &= ~-block;
        }
    }

    chunk_.swap(new_chunk);
}

RoaringChunk RoaringChunk::and_BitmapBitmap(const RoaringChunk& b) const
{
    RoaringChunk answer(key_, BITMAP, 0);

    uint32_t cardinality = 0;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < 1024; ++i)
    {
        data[i] = a_data[i] & b_data[i];
        cardinality += _mm_popcnt_u64(data[i]);
    }

    answer.cardinality_ = cardinality;
    if (cardinality < 4096) answer.convertFromBitmapToArray();

    return answer;
}

RoaringChunk RoaringChunk::and_BitmapArray(const RoaringChunk& b) const
{
    uint32_t b_size = b.cardinality_;

    RoaringChunk answer(key_, ARRAY, b_size);

    uint32_t pos = 0;

    uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < b_size; ++i)
    {
        uint16_t val = b_data[i];
        if (a_data[val / 64] & 1ULL << val)
            data[pos++] = val;
    }

    answer.cardinality_ = pos;

    return answer;
}

RoaringChunk RoaringChunk::and_ArrayArray(const RoaringChunk& b) const
{
    uint32_t a_size = cardinality_, b_size = b.cardinality_;

    RoaringChunk answer(key_, ARRAY, std::min(a_size, b_size));

    uint32_t pos = 0, a_pos = 0, b_pos = 0;

    uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    if (a_pos < a_size && b_pos < b_size)
    {
        while (true)
        {
            if (a_data[a_pos] < b_data[b_pos])
            {
                if (++a_pos == a_size) break;
            }
            else if (a_data[a_pos] > b_data[b_pos])
            {
                if (++b_pos == b_size) break;
            }
            else
            {
                data[pos++] = a_data[a_pos];

                if (++a_pos == a_size || ++b_pos == b_size) break;
            }
        }
    }

    answer.cardinality_ = pos;

    return answer;
}

RoaringChunk RoaringChunk::or_BitmapBitmap(const RoaringChunk& b) const
{
    RoaringChunk answer(key_, BITMAP, 0);

    uint32_t cardinality = 0;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < 1024; ++i)
    {
        data[i] = a_data[i] | b_data[i];
        cardinality += _mm_popcnt_u64(data[i]);
    }

    if (cardinality == 65536) return RoaringChunk(key_, FULL, 0);

    answer.cardinality_ = cardinality;

    return answer;
}

RoaringChunk RoaringChunk::or_BitmapArray(const RoaringChunk& b) const
{
    RoaringChunk answer(*this, true);

    uint32_t cardinality = b.cardinality_;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < cardinality; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] |= 1ULL << val;
    }

    for (uint32_t i = 0; i < 1024; ++i)
    {
        cardinality += _mm_popcnt_u64(data[i]);
    }

    if (cardinality == 65536) return RoaringChunk(key_, FULL, 0);

    answer.cardinality_ = cardinality;

    return answer;
}

RoaringChunk RoaringChunk::or_ArrayArray(const RoaringChunk& b) const
{
    uint32_t a_size = cardinality_, b_size = b.cardinality_;
    uint32_t size = a_size + b_size;

    if (size < 4096)
    {
        RoaringChunk answer(key_, ARRAY, size);

        uint32_t pos = 0, a_pos = 0, b_pos = 0;

        uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

        if (a_pos < a_size && b_pos < b_size)
        {
            while (true)
            {
                if (a_data[a_pos] < b_data[b_pos])
                {
                    do
                    {
                        data[pos++] = a_data[a_pos];
                        if (++a_pos == a_size) break;
                    } while (a_data[a_pos] < b_data[b_pos]);
                    if (++a_pos == a_size) break;
                }
                else if (a_data[a_pos] > b_data[b_pos])
                {
                    do
                    {
                        data[pos++] = b_data[b_pos];
                        if (++b_pos == b_size) break;
                    } while (a_data[a_pos] > b_data[b_pos]);
                    if (++b_pos == b_size) break;
                }
                else
                {
                    data[pos++] = a_data[a_pos];

                    if (++a_pos == a_size || ++b_pos == b_size) break;
                }
            }
        }

        answer.cardinality_ = pos;

        return answer;
    }
    else
    {
        RoaringChunk answer(key_, BITMAP, 0);

        uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

        for (uint32_t i = 0; i < a_size; ++i)
        {
            uint16_t val = a_data[i];
            data[val / 64] |= 1ULL << val;
        }

        for (uint32_t i = 0; i < b_size; ++i)
        {
            uint16_t val = b_data[i];
            data[val / 64] |= 1ULL << val;
        }

        uint32_t cardinality = 0;
        for (uint32_t i = 0; i < 1024; ++i)
        {
            cardinality += _mm_popcnt_u64(data[i]);
        }

        answer.cardinality_ = cardinality;
        if (cardinality < 4096) answer.convertFromBitmapToArray();

        return answer;
    }
}

RoaringChunk RoaringChunk::xor_BitmapBitmap(const RoaringChunk& b) const
{
    RoaringChunk answer(key_, BITMAP, 0);

    uint32_t cardinality = 0;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < 1024; ++i)
    {
        data[i] = a_data[i] & b_data[i];
        cardinality += _mm_popcnt_u64(data[i]);
    }

    if (cardinality == 65536) return RoaringChunk(key_, FULL, 0);

    answer.cardinality_ = cardinality;
    if (cardinality < 4096) answer.convertFromBitmapToArray();

    return answer;
}

RoaringChunk RoaringChunk::xor_BitmapArray(const RoaringChunk& b) const
{
    RoaringChunk answer(*this, true);

    uint32_t cardinality = b.cardinality_;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < cardinality; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] ^= 1ULL << val;
    }

    for (uint32_t i = 0; i < 1024; ++i)
    {
        cardinality += _mm_popcnt_u64(data[i]);
    }

    answer.cardinality_ = cardinality;
    if (cardinality < 4096) answer.convertFromBitmapToArray();

    return answer;
}

RoaringChunk RoaringChunk::xor_ArrayArray(const RoaringChunk& b) const
{
    uint32_t a_size = cardinality_, b_size = b.cardinality_;
    uint32_t size = a_size + b_size;

    if (size < 4096)
    {
        RoaringChunk answer(key_, ARRAY, size);

        uint32_t pos = 0, a_pos = 0, b_pos = 0;

        uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

        if (a_pos < a_size && b_pos < b_size)
        {
            while (true)
            {
                if (a_data[a_pos] < b_data[b_pos])
                {
                    do
                    {
                        data[pos++] = a_data[a_pos];
                        if (++a_pos == a_size) break;
                    } while (a_data[a_pos] < b_data[b_pos]);
                    if (++a_pos == a_size) break;
                }
                else if (a_data[a_pos] > b_data[b_pos])
                {
                    do
                    {
                        data[pos++] = b_data[b_pos];
                        if (++b_pos == b_size) break;
                    } while (a_data[a_pos] > b_data[b_pos]);
                    if (++b_pos == b_size) break;
                }
                else
                {
                    if (++a_pos == a_size || ++b_pos == b_size) break;
                }
            }
        }

        answer.cardinality_ = pos;

        return answer;
    }
    else
    {
        RoaringChunk answer(key_, BITMAP, 0);

        uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

        for (uint32_t i = 0; i < a_size; ++i)
        {
            uint16_t val = a_data[i];
            data[val / 64] ^= 1ULL << val;
        }

        for (uint32_t i = 0; i < b_size; ++i)
        {
            uint16_t val = b_data[i];
            data[val / 64] ^= 1ULL << val;
        }

        uint32_t cardinality = 0;
        for (uint32_t i = 0; i < 1024; ++i)
        {
            cardinality += _mm_popcnt_u64(data[i]);
        }

        answer.cardinality_ = cardinality;
        if (cardinality < 4096) answer.convertFromBitmapToArray();

        return answer;
    }
}

RoaringChunk RoaringChunk::andNot_BitmapBitmap(const RoaringChunk& b) const
{
    RoaringChunk answer(key_, BITMAP, 0);

    uint32_t cardinality = 0;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < 1024; ++i)
    {
        data[i] = a_data[i] & ~b_data[i];
        cardinality += _mm_popcnt_u64(data[i]);
    }

    answer.cardinality_ = cardinality;
    if (cardinality < 4096) answer.convertFromBitmapToArray();

    return answer;
}

RoaringChunk RoaringChunk::andNot_BitmapArray(const RoaringChunk& b) const
{
    RoaringChunk answer(*this, true);

    uint32_t cardinality = b.cardinality_;

    uint64_t* data = reinterpret_cast<uint64_t*>(&answer.chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    for (uint32_t i = 0; i < cardinality; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] &= ~(1ULL << val);
    }

    for (uint32_t i = 0; i < 1024; ++i)
    {
        cardinality += _mm_popcnt_u64(data[i]);
    }

    answer.cardinality_ = cardinality;
    if (cardinality < 4096) answer.convertFromBitmapToArray();

    return answer;
}

RoaringChunk RoaringChunk::andNot_ArrayBitmap(const RoaringChunk& b) const
{
    uint32_t cardinality = cardinality_;

    RoaringChunk answer(key_, ARRAY, cardinality);

    uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b.chunk_[4]);

    uint32_t pos = 0;
    for (uint32_t i = 0; i < cardinality; ++i)
    {
        uint16_t val = a_data[i];
        if ((b_data[val / 64] & 1ULL << val) == 0)
            data[pos++] = val;
    }

    return answer;
}

RoaringChunk RoaringChunk::andNot_ArrayArray(const RoaringChunk& b) const
{
    uint32_t a_size = cardinality_, b_size = b.cardinality_;

    RoaringChunk answer(key_, ARRAY, a_size);

    uint32_t pos = 0, a_pos = 0, b_pos = 0;

    uint16_t* data = reinterpret_cast<uint16_t*>(&answer.chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b.chunk_[4]);

    if (a_pos < a_size && b_pos < b_size)
    {
        while (true)
        {
            if (a_data[a_pos] < b_data[b_pos])
            {
                do
                {
                    data[pos++] = a_data[a_pos];
                    if (++a_pos == a_size) break;
                } while (a_data[a_pos] < b_data[b_pos]);
                if (++a_pos == a_size) break;
            }
            else if (a_data[a_pos] > b_data[b_pos])
            {
                do
                {
                    if (++b_pos == b_size) break;
                } while (a_data[a_pos] > b_data[b_pos]);
                if (++b_pos == b_size) break;
            }
            else
            {
                if (++a_pos == a_size || ++b_pos == b_size) break;
            }
        }
    }

    answer.cardinality_ = pos;

    return answer;
}

NS_IZENELIB_AM_END
