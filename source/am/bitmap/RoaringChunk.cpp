#include <am/bitmap/RoaringChunk.h>

#include <immintrin.h>

NS_IZENELIB_AM_BEGIN

RoaringChunk::RoaringChunk(uint32_t key)
    : key_(key)
    , updatable_(false)
{
}

RoaringChunk::~RoaringChunk()
{
}

boost::shared_array<uint32_t> RoaringChunk::getChunk() const
{
    if (!updatable_) return chunk_;

    while (flag_.test_and_set());
    data_type tmp_chunk(chunk_);
    flag_.clear();

    return tmp_chunk;
}

RoaringChunk::RoaringChunk(const self_type& b)
    : key_(b.key_)
    , updatable_(b.updatable_)
    , chunk_(b.chunk_)
{
}

RoaringChunk& RoaringChunk::operator=(const self_type& b)
{
    key_ = b.key_;
    updatable_ = b.updatable_;
    chunk_ = b.chunk_;

    return *this;
}

bool RoaringChunk::operator==(const self_type& b) const
{
    return key_ == b.key_
        && updatable_ == b.updatable_
        && chunk_ == b.chunk_;
}

void RoaringChunk::resetChunk(Type type, uint32_t capacity)
{
    switch (type)
    {
    case ARRAY:
        capacity = (capacity + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;
        chunk_.reset(cachealign_alloc<uint32_t>(capacity / 2 + 4), cachealign_deleter());
        chunk_[0] = ARRAY;
        chunk_[1] = 0;
        chunk_[2] = capacity;
        chunk_[3] = capacity / 2 + 4;
        break;

    case BITMAP:
        chunk_.reset(cachealign_alloc<uint32_t>(MAX_CHUNK_CAPACITY / 32 + 4), cachealign_deleter());
        chunk_[0] = BITMAP;
        chunk_[1] = 0;
        chunk_[2] = MAX_CHUNK_CAPACITY;
        chunk_[3] = MAX_CHUNK_CAPACITY / 32 + 4;
        memset(&chunk_[4], 0, MAX_CHUNK_CAPACITY / 8);
        break;

    case FULL:
        chunk_.reset(cachealign_alloc<uint32_t>(4), cachealign_deleter());
        chunk_[0] = FULL;
        chunk_[1] = MAX_CHUNK_CAPACITY;
        chunk_[2] = MAX_CHUNK_CAPACITY;
        chunk_[3] = 4;
        break;

    default: break;
    }
}

void RoaringChunk::init(uint32_t key, uint32_t value)
{
    key_ = key;
    updatable_ = true;

    chunk_.reset(cachealign_alloc<uint32_t>(INIT_ARRAY_SIZE / 2 + 4), cachealign_deleter());
    chunk_[0] = ARRAY;
    chunk_[1] = 1;
    chunk_[2] = INIT_ARRAY_SIZE;
    chunk_[3] = INIT_ARRAY_SIZE / 2 + 4;
    chunk_[4] = value;
}

void RoaringChunk::add(uint32_t x)
{
    switch (chunk_[0])
    {
    case BITMAP:
        assert(x >= chunk_[1]);
        if (chunk_[1] < MAX_CHUNK_CAPACITY - 1)
        {
            ++chunk_[1];
            uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
            data[x / 64] |= 1ULL << x;
        }
        else
        {
            data_type new_chunk(cachealign_alloc<uint32_t>(4), cachealign_deleter());
            new_chunk[0] = FULL;
            new_chunk[1] = MAX_CHUNK_CAPACITY;
            new_chunk[2] = MAX_CHUNK_CAPACITY;
            new_chunk[3] = 4;

            while (flag_.test_and_set());
            chunk_.swap(new_chunk);
            flag_.clear();
        }
        break;

    case ARRAY:
        if (chunk_[1] < chunk_[2])
        {
            uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
            data[chunk_[1]++] = x;
        }
        else
        {
            uint32_t capacity = chunk_[2] < 64 ? chunk_[2] * 2 : chunk_[2] < 1024 ? chunk_[2] * 3 / 2 : chunk_[2] * 5 / 4;
            capacity = (capacity + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;

            if (capacity < MAX_ARRAY_SIZE)
            {
                data_type new_chunk(cachealign_alloc<uint32_t>(capacity / 2 + 4), cachealign_deleter());
                new_chunk[0] = ARRAY;
                new_chunk[1] = chunk_[1] + 1;
                new_chunk[2] = capacity;
                new_chunk[3] = capacity / 2 + 4;
                memcpy(&new_chunk[4], &chunk_[4], chunk_[1] * 2);

                uint16_t* data = reinterpret_cast<uint16_t*>(&new_chunk[4]);
                data[chunk_[1]] = x;

                while (flag_.test_and_set());
                chunk_.swap(new_chunk);
                flag_.clear();
            }
            else
            {
                data_type new_chunk(cachealign_alloc<uint32_t>(MAX_CHUNK_CAPACITY / 32 + 4), cachealign_deleter());
                new_chunk[0] = BITMAP;
                new_chunk[1] = chunk_[1] + 1;
                new_chunk[2] = MAX_CHUNK_CAPACITY;
                new_chunk[3] = MAX_CHUNK_CAPACITY / 32 + 4;
                memset(&new_chunk[4], 0, MAX_CHUNK_CAPACITY / 8);

                uint64_t* data = reinterpret_cast<uint64_t*>(&new_chunk[4]);
                const uint16_t* old_data = reinterpret_cast<uint16_t*>(&chunk_[4]);

                uint16_t size = chunk_[1];
                for (uint32_t i = 0; i < size; ++i)
                {
                    data[old_data[i] / 64] |= 1ULL << old_data[i];
                }
                data[x / 64] |= 1ULL << x;

                while (flag_.test_and_set());
                chunk_.swap(new_chunk);
                flag_.clear();
            }
        }
        break;

    default: break;
    }
}

void RoaringChunk::trim()
{
    if (!chunk_) return;

    if (chunk_[0] == ARRAY)
    {
        uint32_t capacity = (chunk_[1] + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;

        if (capacity < chunk_[2])
        {
            data_type new_chunk(cachealign_alloc<uint32_t>(capacity / 2 + 4), cachealign_deleter());
            new_chunk[0] = ARRAY;
            new_chunk[1] = chunk_[1];
            new_chunk[2] = capacity;
            new_chunk[3] = capacity / 2 + 4;
            memcpy(&new_chunk[4], &chunk_[4], chunk_[1] * 2);

            while (flag_.test_and_set());
            chunk_.swap(new_chunk);
            flag_.clear();
        }
    }

    updatable_ = false;
}

void RoaringChunk::cloneChunk(const data_type& chunk)
{
    chunk_.reset(cachealign_alloc<uint32_t>(chunk[3]), cachealign_deleter());
    memcpy(&chunk_[0], &chunk[0], chunk[3] * 4);
}

void RoaringChunk::copyOnDemand(const self_type& b)
{
    key_ = b.key_;

    if (b.updatable_) cloneChunk(b.getChunk());
    else chunk_ = b.chunk_;
}

RoaringChunk RoaringChunk::operator~() const
{
    self_type answer(key_);

    data_type chunk = getChunk();

    switch (chunk[0])
    {
    case 0: answer.not_Array(chunk);
    case 1: answer.not_Bitmap(chunk);
    default: break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator&(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = a_chunk[0] * 3 + b_chunk[0];
    switch (type)
    {
    case 0: answer.and_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.and_BitmapArray(b_chunk, a_chunk); break;
    case 3: answer.and_BitmapArray(a_chunk, b_chunk); break;
    case 4: answer.and_BitmapBitmap(a_chunk, b_chunk); break;
    case 2:
    case 5: answer.copyOnDemand(*this); break;
    default: answer.copyOnDemand(b); break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator|(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = a_chunk[0] * 3 + b_chunk[0];
    switch (type)
    {
    case 0: answer.or_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.or_BitmapArray(b_chunk, a_chunk); break;
    case 3: answer.or_BitmapArray(a_chunk, b_chunk); break;
    case 4: answer.or_BitmapBitmap(a_chunk, b_chunk); break;
    default: answer.resetChunk(FULL, 0); break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator^(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = a_chunk[0] * 3 + b_chunk[0];
    switch (type)
    {
    case 0: answer.xor_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.xor_BitmapArray(b_chunk, a_chunk); break;
    case 2: answer.not_Array(a_chunk); break;
    case 3: answer.xor_BitmapArray(a_chunk, b_chunk); break;
    case 4: answer.xor_BitmapBitmap(a_chunk, b_chunk); break;
    case 5: answer.not_Bitmap(a_chunk); break;
    case 6: answer.not_Array(b_chunk); break;
    case 7: answer.not_Bitmap(b_chunk); break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator-(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = a_chunk[0] * 3 + b_chunk[0];
    switch (type)
    {
    case 0: answer.andNot_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.andNot_ArrayBitmap(a_chunk, b_chunk); break;
    case 3: answer.andNot_BitmapArray(a_chunk, b_chunk); break;
    case 4: answer.andNot_BitmapBitmap(a_chunk, b_chunk); break;
    case 6: answer.not_Array(b_chunk); break;
    case 7: answer.not_Bitmap(b_chunk); break;
    }

    return answer;
}

void RoaringChunk::bitmap2Array()
{
    uint32_t size = chunk_[1];

    data_type old_chunk = chunk_;
    resetChunk(ARRAY, size);

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint64_t* old_data = reinterpret_cast<const uint64_t*>(&old_chunk[4]);

    for (uint32_t i = 0, pos = 0; i < BITMAP_SIZE && pos < size; ++i)
    {
        uint64_t block = old_data[i];
        while (block)
        {
            data[pos++] = uint16_t(i * 64 + __builtin_ctzll(block));
            block &= block - 1;
        }
    }
}

void RoaringChunk::not_Bitmap(const data_type& a)
{
    uint32_t size = MAX_CHUNK_CAPACITY - a[1];

    if (size < MAX_ARRAY_SIZE)
    {
        resetChunk(ARRAY, size);

        uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
        const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);

        for (uint32_t i = 0, pos = 0; i < BITMAP_SIZE && pos < size; ++i)
        {
            uint64_t block = ~a_data[i];
            while (block)
            {
                data[pos++] = uint16_t(i * 64 + __builtin_ctzll(block));
                block &= block - 1;
            }
        }
    }
    else
    {
        resetChunk(BITMAP, 0);

        uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
        const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);

        for (uint32_t i = 0; i < MAX_CHUNK_CAPACITY / 64; ++i)
        {
            data[i] = ~a_data[i];
        }
    }

    chunk_[1] = size;
}

void RoaringChunk::not_Array(const data_type& a)
{
    resetChunk(BITMAP, 0);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);

    memset(data, 0xff, MAX_CHUNK_CAPACITY / 8);

    uint32_t size = a[1];
    for (uint32_t i = 0; i < size; ++i)
    {
        data[a_data[i] / 64] ^= 1ULL << a_data[i];
    }

    chunk_[1] = MAX_CHUNK_CAPACITY - size;
}

void RoaringChunk::and_BitmapBitmap(const data_type& a, const data_type& b)
{
    resetChunk(BITMAP, 0);

    uint32_t size = 0;

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b[4]);

    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        data[i] = a_data[i] & b_data[i];
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size < MAX_ARRAY_SIZE) bitmap2Array();
}

void RoaringChunk::and_BitmapArray(const data_type& a, const data_type& b)
{
    uint32_t pos = 0, b_size = b[1];

    resetChunk(ARRAY, b_size);

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

    for (uint32_t i = 0; i < b_size; ++i)
    {
        uint16_t val = b_data[i];
        if (a_data[val / 64] & 1ULL << val)
            data[pos++] = val;
    }

    chunk_[1] = pos;
}

void RoaringChunk::and_ArrayArray(const data_type& a, const data_type& b)
{
    uint32_t a_size = a[1], b_size = b[1];

    resetChunk(ARRAY, std::min(a_size, b_size));

    uint32_t pos = 0, a_pos = 0, b_pos = 0;

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

    chunk_[1] = pos;
}

void RoaringChunk::or_BitmapBitmap(const data_type& a, const data_type& b)
{
    resetChunk(BITMAP, 0);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b[4]);

    uint32_t size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        data[i] = a_data[i] | b_data[i];
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size == MAX_CHUNK_CAPACITY) resetChunk(FULL, 0);
}

void RoaringChunk::or_BitmapArray(const data_type& a, const data_type& b)
{
    cloneChunk(a);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

    uint32_t size = b[1];
    for (uint32_t i = 0; i < size; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] |= 1ULL << val;
    }

    size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size == MAX_CHUNK_CAPACITY) resetChunk(FULL, 0);
}

void RoaringChunk::or_ArrayArray(const data_type& a, const data_type& b)
{
    uint32_t a_size = a[1], b_size = b[1];
    uint32_t size = a_size + b_size;

    if (size < MAX_ARRAY_SIZE)
    {
        resetChunk(ARRAY, size);

        uint32_t pos = 0, a_pos = 0, b_pos = 0;

        uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

        chunk_[1] = pos;
    }
    else
    {
        resetChunk(BITMAP, 0);

        uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

        size = 0;
        for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
        {
            size += _mm_popcnt_u64(data[i]);
        }

        chunk_[1] = size;
        if (size < MAX_ARRAY_SIZE) bitmap2Array();
    }
}

void RoaringChunk::xor_BitmapBitmap(const data_type& a, const data_type& b)
{
    resetChunk(BITMAP, 0);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b[4]);

    uint32_t size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        data[i] = a_data[i] & b_data[i];
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size == MAX_CHUNK_CAPACITY) resetChunk(FULL, 0);
    else if (size < MAX_ARRAY_SIZE) bitmap2Array();
}

void RoaringChunk::xor_BitmapArray(const data_type& a, const data_type& b)
{
    cloneChunk(a);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

    uint32_t size = b[1];
    for (uint32_t i = 0; i < size; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] ^= 1ULL << val;
    }

    size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size == MAX_CHUNK_CAPACITY) resetChunk(FULL, 0);
    else if (size < MAX_ARRAY_SIZE) bitmap2Array();
}

void RoaringChunk::xor_ArrayArray(const data_type& a, const data_type& b)
{
    uint32_t a_size = a[1], b_size = b[1];
    uint32_t size = a_size + b_size;

    if (size < MAX_ARRAY_SIZE)
    {
        resetChunk(ARRAY, size);

        uint32_t pos = 0, a_pos = 0, b_pos = 0;

        uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

        chunk_[1] = pos;
    }
    else
    {
        resetChunk(BITMAP, 0);

        uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
        const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
        const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

        size = 0;
        for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
        {
            size += _mm_popcnt_u64(data[i]);
        }

        chunk_[1] = size;
        if (size < MAX_ARRAY_SIZE) bitmap2Array();
    }
}

void RoaringChunk::andNot_BitmapBitmap(const data_type& a, const data_type& b)
{
    resetChunk(BITMAP, 0);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b[4]);

    uint32_t size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        data[i] = a_data[i] & ~b_data[i];
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size < MAX_ARRAY_SIZE) bitmap2Array();
}

void RoaringChunk::andNot_BitmapArray(const data_type& a, const data_type& b)
{
    cloneChunk(a);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

    uint32_t size = b[1];
    for (uint32_t i = 0; i < size; ++i)
    {
        uint16_t val = b_data[i];
        data[val / 64] &= ~(1ULL << val);
    }

    size = 0;
    for (uint32_t i = 0; i < BITMAP_SIZE; ++i)
    {
        size += _mm_popcnt_u64(data[i]);
    }

    chunk_[1] = size;
    if (size < MAX_ARRAY_SIZE) bitmap2Array();
}

void RoaringChunk::andNot_ArrayBitmap(const data_type& a, const data_type& b)
{
    uint32_t size = a[1];

    resetChunk(ARRAY, size);

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
    const uint64_t* b_data = reinterpret_cast<const uint64_t*>(&b[4]);

    uint32_t pos = 0;
    for (uint32_t i = 0; i < size; ++i)
    {
        uint16_t val = a_data[i];
        if ((b_data[val / 64] & 1ULL << val) == 0)
            data[pos++] = val;
    }

    chunk_[1] = pos;
}

void RoaringChunk::andNot_ArrayArray(const data_type& a, const data_type& b)
{
    uint32_t a_size = a[1], b_size = b[1];

    resetChunk(ARRAY, a_size);

    uint32_t pos = 0, a_pos = 0, b_pos = 0;

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

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

    chunk_[1] = pos;
}

NS_IZENELIB_AM_END
