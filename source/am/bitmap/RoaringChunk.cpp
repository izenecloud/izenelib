#include <am/bitmap/RoaringChunk.h>

#include <immintrin.h>

NS_IZENELIB_AM_BEGIN

RoaringChunk::RoaringChunk(uint32_t key)
    : key_(key)
    , updatable_(false)
    , full_(false)
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
    , full_(b.full_)
    , chunk_(b.chunk_)
{
}

RoaringChunk& RoaringChunk::operator=(const self_type& b)
{
    key_ = b.key_;
    updatable_ = b.updatable_;
    full_ = b.full_;
    chunk_ = b.chunk_;

    return *this;
}

bool RoaringChunk::operator==(const self_type& b) const
{
    return key_ == b.key_
        && updatable_ == b.updatable_
        && full_ == b.full_
        && chunk_ == b.chunk_;
}

void RoaringChunk::resetChunk(Type type, uint32_t capacity)
{
    switch (type)
    {
    case ARRAY:
        capacity = (capacity + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;
        chunk_.reset(cachealign_alloc<uint32_t>(capacity / 2 + 4, 16), cachealign_deleter());
        chunk_[0] = ARRAY;
        chunk_[1] = 0;
        chunk_[2] = capacity;
        chunk_[3] = capacity / 2 + 4;
        break;

    case BITMAP:
        chunk_.reset(cachealign_alloc<uint32_t>(MAX_CHUNK_CAPACITY / 32 + 4, 16), cachealign_deleter());
        chunk_[0] = BITMAP;
        chunk_[1] = 0;
        chunk_[2] = MAX_CHUNK_CAPACITY;
        chunk_[3] = MAX_CHUNK_CAPACITY / 32 + 4;
        memset(&chunk_[4], capacity, MAX_CHUNK_CAPACITY / 8);
        break;

    case FULL:
        full_ = true;
        chunk_.reset();
        break;

    default:
        full_ = false;
        chunk_.reset();
        break;
    }
}

void RoaringChunk::init(uint32_t key, uint32_t value)
{
    key_ = key;
    updatable_ = true;

    chunk_.reset(cachealign_alloc<uint32_t>(INIT_ARRAY_SIZE / 2 + 4, 16), cachealign_deleter());
    chunk_[0] = ARRAY;
    chunk_[1] = 1;
    chunk_[2] = INIT_ARRAY_SIZE;
    chunk_[3] = INIT_ARRAY_SIZE / 2 + 4;
    chunk_[4] = value;
}

void RoaringChunk::trim()
{
    if (!chunk_) return;

    if (!chunk_[1])
    {
        while (flag_.test_and_set());
        chunk_.reset();
        flag_.clear();
    }
    else if (chunk_[0] == ARRAY)
    {
        if (chunk_[1] <= chunk_[2] - INIT_ARRAY_SIZE)
        {
            uint32_t capacity = (chunk_[1] + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;

            data_type new_chunk(cachealign_alloc<uint32_t>(capacity / 2 + 4, 16), cachealign_deleter());
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
    else
    {
        if (chunk_[1] < MAX_ARRAY_SIZE)
        {
            uint32_t size = chunk_[1];
            uint32_t capacity = (size + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;

            data_type new_chunk(cachealign_alloc<uint32_t>(capacity / 2 + 4, 16), cachealign_deleter());
            new_chunk[0] = ARRAY;
            new_chunk[1] = size;
            new_chunk[2] = capacity;
            new_chunk[3] = capacity / 2 + 4;

            uint16_t* data = reinterpret_cast<uint16_t*>(&new_chunk[4]);
            const uint64_t* old_data = reinterpret_cast<const uint64_t*>(&chunk_[4]);

            for (uint32_t i = 0, pos = 0; i < BITMAP_SIZE && pos < size; ++i)
            {
                uint64_t block = old_data[i];
                while (block)
                {
                    data[pos++] = uint16_t(i * 64 + __builtin_ctzll(block));
                    block &= block - 1;
                }
            }

            while (flag_.test_and_set());
            chunk_.swap(new_chunk);
            flag_.clear();
        }
        else if (chunk_[1] == MAX_CHUNK_CAPACITY)
        {
            full_ = true;

            while (flag_.test_and_set());
            chunk_.reset();
            flag_.clear();
        }
    }

    updatable_ = false;
}

void RoaringChunk::add(uint32_t x)
{
    if (!chunk_)
    {
        updatable_ = true;

        data_type new_chunk(cachealign_alloc<uint32_t>(INIT_ARRAY_SIZE / 2 + 4, 16), cachealign_deleter());
        new_chunk[0] = ARRAY;
        new_chunk[1] = 1;
        new_chunk[2] = INIT_ARRAY_SIZE;
        new_chunk[3] = INIT_ARRAY_SIZE / 2 + 4;
        new_chunk[4] = x;

        while (flag_.test_and_set());
        chunk_.swap(new_chunk);
        flag_.clear();
    }
    else if (chunk_[0] == ARRAY)
    {
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
                data_type new_chunk(cachealign_alloc<uint32_t>(capacity / 2 + 4, 16), cachealign_deleter());
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
                data_type new_chunk(cachealign_alloc<uint32_t>(MAX_CHUNK_CAPACITY / 32 + 4, 16), cachealign_deleter());
                new_chunk[0] = BITMAP;
                new_chunk[1] = chunk_[1] + 1;
                new_chunk[2] = MAX_CHUNK_CAPACITY;
                new_chunk[3] = MAX_CHUNK_CAPACITY / 32 + 4;
                memset(&new_chunk[4], 0, MAX_CHUNK_CAPACITY / 8);

                uint64_t* data = reinterpret_cast<uint64_t*>(&new_chunk[4]);
                const uint16_t* old_data = reinterpret_cast<const uint16_t*>(&chunk_[4]);

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
    }
    else
    {
        if (chunk_[1] < MAX_CHUNK_CAPACITY - 1)
        {
            uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
            data[x / 64] |= 1ULL << x;
            ++chunk_[1];
        }
        else
        {
            full_ = true;

            while (flag_.test_and_set());
            chunk_.reset();
            flag_.clear();
        }
    }
}

bool RoaringChunk::contains(uint32_t x) const
{
    data_type tmp_chunk = getChunk();

    if (!tmp_chunk) return full_;

    if (tmp_chunk[0] == ARRAY)
    {
        const uint16_t* data = reinterpret_cast<const uint16_t*>(&tmp_chunk[4]);

        if (data[tmp_chunk[1]] < x) return false;

        __m128i pivot = _mm_set1_epi16((uint16_t)x);
        const uint16_t* tmp = data;

        for (;; tmp += INIT_ARRAY_SIZE)
        {
            int res = _mm_movemask_epi8(_mm_packs_epi16(
                        _mm_cmplt_epi16(_mm_load_si128(
                                reinterpret_cast<const __m128i*>(tmp)), pivot),
                        _mm_cmplt_epi16(_mm_load_si128(
                                reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

            if (res != 0xffff)
            {
                return tmp[__builtin_ctz(~res)] == x;
            }
        }

        return false;
    }
    else
    {
        const uint64_t* data = reinterpret_cast<const uint64_t*>(&tmp_chunk[4]);

        return data[x / 64] & 1ULL << x;
    }
}

void RoaringChunk::cloneChunk(const data_type& chunk)
{
    if (!chunk) return;

    if (chunk[0] == ARRAY)
    {
        uint32_t size = chunk[1];
        uint32_t capacity = (size + INIT_ARRAY_SIZE - 1) & -INIT_ARRAY_SIZE;
        chunk_.reset(cachealign_alloc<uint32_t>(capacity / 2 + 4, 16), cachealign_deleter());
        chunk_[0] = ARRAY;
        chunk_[1] = size;
        chunk_[2] = capacity;
        chunk_[3] = capacity / 2 + 4;
        memcpy(&chunk_[4], &chunk[4], size * 2);
    }
    else
    {
        chunk_.reset(cachealign_alloc<uint32_t>(chunk[3], 16), cachealign_deleter());
        memcpy(&chunk_[0], &chunk[0], chunk[3] * 4);
    }
}

void RoaringChunk::atomicCopy(const self_type& b)
{
    key_ = b.key_;
    chunk_ = b.getChunk();
    full_ = b.full_;
    if (full_) chunk_.reset();
}

RoaringChunk RoaringChunk::operator~() const
{
    self_type answer(key_);

    data_type chunk = getChunk();

    if (!chunk) answer.full_ = !full_;
    else if (chunk[0] == ARRAY) answer.not_Array(chunk);
    else answer.not_Bitmap(chunk);

    return answer;
}

RoaringChunk RoaringChunk::operator&(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = (!a_chunk ? 2 + full_ : a_chunk[0]) * 4
        + (!b_chunk ? 2 + b.full_ : b_chunk[0]);

    switch (type)
    {
    case 0: answer.and_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.and_BitmapArray(b_chunk, a_chunk); break;
    case 4: answer.and_BitmapArray(a_chunk, b_chunk); break;
    case 5: answer.and_BitmapBitmap(a_chunk, b_chunk); break;
    case 3:
    case 7: answer.chunk_ = a_chunk; break;
    case 12:
    case 13: answer.chunk_ = b_chunk; break;
    case 15: answer.full_ = true; break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator|(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = (!a_chunk ? 2 + full_ : a_chunk[0]) * 4
        + (!b_chunk ? 2 + b.full_ : b_chunk[0]);

    switch (type)
    {
    case 0: answer.or_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.or_BitmapArray(b_chunk, a_chunk); break;
    case 4: answer.or_BitmapArray(a_chunk, b_chunk); break;
    case 5: answer.or_BitmapBitmap(a_chunk, b_chunk); break;
    case 2:
    case 6: answer.chunk_ = a_chunk; break;
    case 8:
    case 9: answer.chunk_ = b_chunk; break;
    case 10: break;
    default: answer.full_ = true; break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator^(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = (!a_chunk ? 2 + full_ : a_chunk[0]) * 4
        + (!b_chunk ? 2 + b.full_ : b_chunk[0]);

    switch (type)
    {
    case 0: answer.xor_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.xor_BitmapArray(b_chunk, a_chunk); break;
    case 3: answer.not_Array(a_chunk); break;
    case 4: answer.xor_BitmapArray(a_chunk, b_chunk); break;
    case 5: answer.xor_BitmapBitmap(a_chunk, b_chunk); break;
    case 2:
    case 6: answer.chunk_ = a_chunk; break;
    case 7: answer.not_Bitmap(a_chunk); break;
    case 8:
    case 9: answer.chunk_ = b_chunk; break;
    case 12: answer.not_Array(b_chunk); break;
    case 13: answer.not_Bitmap(b_chunk); break;
    case 11:
    case 14: answer.full_ = true; break;
    }

    return answer;
}

RoaringChunk RoaringChunk::operator-(const self_type& b) const
{
    self_type answer(key_);

    data_type a_chunk = getChunk();
    data_type b_chunk = b.getChunk();

    uint32_t type = (!a_chunk ? 2 + full_ : a_chunk[0]) * 4
        + (!b_chunk ? 2 + b.full_ : b_chunk[0]);

    switch (type)
    {
    case 0: answer.andNot_ArrayArray(a_chunk, b_chunk); break;
    case 1: answer.andNot_ArrayBitmap(a_chunk, b_chunk); break;
    case 4: answer.andNot_BitmapArray(a_chunk, b_chunk); break;
    case 5: answer.andNot_BitmapBitmap(a_chunk, b_chunk); break;
    case 2:
    case 6: answer.chunk_ = a_chunk; break;
    case 12: answer.full_ = true; break;
    case 13: answer.not_Array(b_chunk); break;
    case 14: answer.not_Bitmap(b_chunk); break;
    }

    return answer;
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
    resetChunk(BITMAP, 0xff);

    uint64_t* data = reinterpret_cast<uint64_t*>(&chunk_[4]);
    const uint16_t* a_data = reinterpret_cast<const uint16_t*>(&a[4]);

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
    trim();
}

void RoaringChunk::and_BitmapArray(const data_type& a, const data_type& b)
{
    uint32_t b_size = b[1];

    resetChunk(ARRAY, b_size);

    uint16_t* data = reinterpret_cast<uint16_t*>(&chunk_[4]);
    const uint64_t* a_data = reinterpret_cast<const uint64_t*>(&a[4]);
    const uint16_t* b_data = reinterpret_cast<const uint16_t*>(&b[4]);

    uint32_t size = 0;
    for (uint32_t i = 0; i < b_size; ++i)
    {
        uint16_t val = b_data[i];
        if (a_data[val / 64] & 1ULL << val)
            data[size++] = val;
    }    chunk_[1] = size;

    chunk_[1] = size;
    trim();
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
        uint16_t a_tail = a_data[a_size - 1], b_tail = b_data[b_size - 1];
        uint16_t a_current = a_data[0], b_current = b_data[0];

        while (true)
        {
            if (a_current < b_current)
            {
                if (a_tail < b_current) break;

                const uint16_t* tmp = a_data + (a_pos & -INIT_ARRAY_SIZE);
                __m128i pivot = _mm_set1_epi16(b_current);

                for (;; tmp += INIT_ARRAY_SIZE)
                {
                    int res = _mm_movemask_epi8(_mm_packs_epi16(
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp)), pivot),
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                    if (res != 0xffff)
                    {
                        a_pos = tmp - a_data + __builtin_ctz(~res);
                        a_current = a_data[a_pos];
                        break;
                    }
                }
            }

            if (a_current > b_current)
            {
                if (a_current > b_tail) break;

                const uint16_t* tmp = b_data + (b_pos & -INIT_ARRAY_SIZE);
                __m128i pivot = _mm_set1_epi16(a_current);

                for (;; tmp += INIT_ARRAY_SIZE)
                {
                    int res = _mm_movemask_epi8(_mm_packs_epi16(
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp)), pivot),
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                    if (res != 0xffff)
                    {
                        b_pos = tmp - b_data + __builtin_ctz(~res);
                        b_current = b_data[b_pos];
                        break;
                    }
                }
            }

            if (a_current == b_current)
            {
                data[pos++] = a_current;

                if (++a_pos == a_size || ++b_pos == b_size) break;

                a_current = a_data[a_pos];
                b_current = b_data[b_pos];
            }
        }
    }

    chunk_[1] = pos;
    trim();
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
    trim();
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
    trim();
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
            uint16_t a_tail = a_data[a_size - 1], b_tail = b_data[b_size - 1];
            uint16_t a_current = a_data[0], b_current = b_data[0];

            while (true)
            {
                if (a_current < b_current)
                {
                    if (a_tail < b_current)
                    {
                        memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
                        pos += a_size - a_pos;
                        break;
                    }

                    const uint16_t* tmp = a_data + (a_pos & -INIT_ARRAY_SIZE);
                    __m128i pivot = _mm_set1_epi16(b_current);

                    for (;; tmp += INIT_ARRAY_SIZE)
                    {
                        int res = _mm_movemask_epi8(_mm_packs_epi16(
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp)), pivot),
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                        if (res != 0xffff)
                        {
                            uint32_t new_pos = tmp - a_data + __builtin_ctz(~res);
                            memcpy(&data[pos], &a_data[a_pos], (new_pos - a_pos) * 2);
                            pos += new_pos - a_pos;
                            a_pos = new_pos;
                            a_current = a_data[a_pos];
                            break;
                        }
                    }
                }

                if (a_current > b_current)
                {
                    if (a_current > b_tail)
                    {
                        memcpy(&data[pos], &b_data[b_pos], (b_size - b_pos) * 2);
                        pos += b_size - b_pos;
                        break;
                    }

                    const uint16_t* tmp = b_data + (b_pos & -INIT_ARRAY_SIZE);
                    __m128i pivot = _mm_set1_epi16(a_current);

                    for (;; tmp += INIT_ARRAY_SIZE)
                    {
                        int res = _mm_movemask_epi8(_mm_packs_epi16(
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp)), pivot),
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                        if (res != 0xffff)
                        {
                            uint32_t new_pos = tmp - b_data + __builtin_ctz(~res);
                            memcpy(&data[pos], &b_data[b_pos], (new_pos - b_pos) * 2);
                            pos += new_pos - b_pos;
                            b_pos = new_pos;
                            b_current = b_data[b_pos];
                            break;
                        }
                    }
                }

                if (a_current == b_current)
                {
                    data[pos++] = a_current;

                    if (++a_pos == a_size || ++b_pos == b_size) break;

                    a_current = a_data[a_pos];
                    b_current = b_data[b_pos];
                }
            }
        }

        if (a_pos == a_size)
        {
            memcpy(&data[pos], &b_data[b_pos], (b_size - b_pos) * 2);
            pos += b_size - b_pos;
        }
        else if (b_pos == b_size)
        {
            memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
            pos += a_size - a_pos;
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
    }

    trim();
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
    trim();
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
    trim();
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
            uint16_t a_tail = a_data[a_size - 1], b_tail = b_data[b_size - 1];
            uint16_t a_current = a_data[0], b_current = b_data[0];

            while (true)
            {
                if (a_current < b_current)
                {
                    if (a_tail < b_current)
                    {
                        memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
                        pos += a_size - a_pos;
                        break;
                    }

                    const uint16_t* tmp = a_data + (a_pos & -INIT_ARRAY_SIZE);
                    __m128i pivot = _mm_set1_epi16(b_current);

                    for (;; tmp += INIT_ARRAY_SIZE)
                    {
                        int res = _mm_movemask_epi8(_mm_packs_epi16(
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp)), pivot),
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                        if (res != 0xffff)
                        {
                            uint32_t new_pos = tmp - a_data + __builtin_ctz(~res);
                            memcpy(&data[pos], &a_data[a_pos], (new_pos - a_pos) * 2);
                            pos += new_pos - a_pos;
                            a_pos = new_pos;
                            a_current = a_data[a_pos];
                            break;
                        }
                    }
                }

                if (a_current > b_current)
                {
                    if (a_current > b_tail)
                    {
                        memcpy(&data[pos], &b_data[b_pos], (b_size - b_pos) * 2);
                        pos += b_size - b_pos;
                        break;
                    }

                    const uint16_t* tmp = b_data + (b_pos & -INIT_ARRAY_SIZE);
                    __m128i pivot = _mm_set1_epi16(a_current);

                    for (;; tmp += INIT_ARRAY_SIZE)
                    {
                        int res = _mm_movemask_epi8(_mm_packs_epi16(
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp)), pivot),
                                    _mm_cmplt_epi16(_mm_load_si128(
                                            reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                        if (res != 0xffff)
                        {
                            uint32_t new_pos = tmp - b_data + __builtin_ctz(~res);
                            memcpy(&data[pos], &b_data[b_pos], (new_pos - b_pos) * 2);
                            pos += new_pos - b_pos;
                            b_pos = new_pos;
                            b_current = b_data[b_pos];
                            break;
                        }
                    }
                }

                if (a_current == b_current)
                {
                    if (++a_pos == a_size || ++b_pos == b_size) break;

                    a_current = a_data[a_pos];
                    b_current = b_data[b_pos];
                }
            }
        }

        if (a_pos == a_size)
        {
            memcpy(&data[pos], &b_data[b_pos], (b_size - b_pos) * 2);
            pos += b_size - b_pos;
        }
        else if (b_pos == b_size)
        {
            memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
            pos += a_size - a_pos;
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
    }

    trim();
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
    trim();
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
    trim();
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
    trim();
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
        uint16_t a_tail = a_data[a_size - 1], b_tail = b_data[b_size - 1];
        uint16_t a_current = a_data[0], b_current = b_data[0];

        while (true)
        {
            if (a_current < b_current)
            {
                if (a_tail < b_current)
                {
                    memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
                    pos += a_size - a_pos;
                    break;
                }

                const uint16_t* tmp = a_data + (a_pos & -INIT_ARRAY_SIZE);
                __m128i pivot = _mm_set1_epi16(b_current);

                for (;; tmp += INIT_ARRAY_SIZE)
                {
                    int res = _mm_movemask_epi8(_mm_packs_epi16(
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp)), pivot),
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                    if (res != 0xffff)
                    {
                        uint32_t new_pos = tmp - a_data + __builtin_ctz(~res);
                        memcpy(&data[pos], &a_data[a_pos], (new_pos - a_pos) * 2);
                        pos += new_pos - a_pos;
                        a_pos = new_pos;
                        a_current = a_data[a_pos];
                        break;
                    }
                }
            }

            if (a_current > b_current)
            {
                if (a_current > b_tail) break;

                const uint16_t* tmp = b_data + (b_pos & -INIT_ARRAY_SIZE);
                __m128i pivot = _mm_set1_epi16(a_current);

                for (;; tmp += INIT_ARRAY_SIZE)
                {
                    int res = _mm_movemask_epi8(_mm_packs_epi16(
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp)), pivot),
                                _mm_cmplt_epi16(_mm_load_si128(
                                        reinterpret_cast<const __m128i*>(tmp) + 1), pivot)));

                    if (res != 0xffff)
                    {
                        b_pos = tmp - b_data + __builtin_ctz(~res);
                        b_current = b_data[b_pos];
                        break;
                    }
                }
            }

            if (a_current == b_current)
            {
                if (++a_pos == a_size || ++b_pos == b_size) break;

                a_current = a_data[a_pos];
                b_current = b_data[b_pos];
            }
        }
    }

    if (b_pos == b_size)
    {
        memcpy(&data[pos], &a_data[a_pos], (a_size - a_pos) * 2);
        pos += a_size - a_pos;
    }

    chunk_[1] = pos;
    trim();
}

NS_IZENELIB_AM_END
