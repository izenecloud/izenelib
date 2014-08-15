#include <am/bitmap/RoaringBitmap.h>
#include <am/bitmap/RoaringChunk.h>

NS_IZENELIB_AM_BEGIN

RoaringBitmap::RoaringBitmap(bool updatable)
    : capacity_(0)
    , size_(0)
    , cardinality_(0)
    , updatable_(updatable)
{
}

RoaringBitmap::~RoaringBitmap()
{
}

boost::shared_array<RoaringChunk> RoaringBitmap::getArray() const
{
    if (!updatable_) return array_;

    while (flag_.test_and_set());
    array_type tmp_array(array_);
    flag_.clear();

    return tmp_array;
}

RoaringBitmap::RoaringBitmap(const RoaringBitmap& b)
    : capacity_(b.capacity_)
    , size_(b.size_)
    , cardinality_(b.cardinality_)
    , updatable_(false)
    , array_(b.getArray())
{
}

RoaringBitmap& RoaringBitmap::operator=(const RoaringBitmap& b)
{
    capacity_ = b.capacity_;
    size_ = b.size_;
    cardinality_ = b.cardinality_;
    updatable_ = false;
    array_ = b.getArray();

    return *this;
}

bool RoaringBitmap::operator==(const RoaringBitmap& b) const
{
    return size_ == b.size_
        && cardinality_ == b.cardinality_
        && std::equal(&array_[0], &array_[size_], &b.array_[0]);
}

void RoaringBitmap::extendArray(uint32_t k)
{
    if (size_ + k >= capacity_)
    {
        uint32_t newCapacity = capacity_ < 1024 ? 2 * (size_ + k) : 5 * (size_ + k) / 4;
        newCapacity = std::min(newCapacity, 65536U);

        array_type newContainer(new chunk_type[newCapacity]);
        std::copy(&array_[0], &array_[capacity_], &newContainer[0]);

        while (flag_.test_and_set());
        array_.swap(newContainer);
        flag_.clear();
        capacity_ = newCapacity;
    }
}

void RoaringBitmap::append(const RoaringChunk& value)
{
    uint32_t inc = value.getCardinality();
    if (inc == 0) return;
    extendArray(1);
    array_[size_++] = value;
    cardinality_ += inc;
}

void RoaringBitmap::appendCopy(const RoaringBitmap::array_type& sa, uint32_t index)
{
    extendArray(1);
    array_[size_].atomicCopy(sa[index]);
    cardinality_ += array_[size_++].getCardinality();
}

void RoaringBitmap::appendCopy(const RoaringBitmap::array_type& sa, uint32_t begin, uint32_t end)
{
    extendArray(end - begin);
    for (uint32_t i = begin; i < end; ++i)
    {
        array_[size_].atomicCopy(sa[i]);
        cardinality_ += array_[size_++].getCardinality();
    }
}

void RoaringBitmap::add(uint32_t x)
{
    uint32_t hb = x >> 16;
    uint32_t lb = x & 0xffff;

    if (array_[size_].getKey() == hb)
    {
        array_[size_].add(lb);
    }
    else
    {
        if (size_) array_[size_].trim();

        extendArray(1);
        array_[size_++].init(hb, lb);
        ++cardinality_;
    }
}

bool RoaringBitmap::contains(uint32_t x) const
{
    uint32_t hb = x >> 16;
    uint32_t lb = x & 0xffff;

    uint32_t tmp_size = size_;
    if (tmp_size == 0) return false;

    array_type tmp_array = getArray();

    if (tmp_array[tmp_size - 1].getKey() == hb)
        return tmp_array[tmp_size - 1].contains(lb);

    uint32_t begin = 0;
    uint32_t end = tmp_size - 1;
    uint32_t key = -1;

    while (begin < end && key != x)
    {
        uint32_t middle = (begin + end) / 2;
        key = tmp_array[middle].getKey();

        asm("cmpl %3, %2\n\tcmovg %4, %0\n\tcmovle %5, %1"
                : "+r" (begin), "+r" (end)
                : "r" (x), "g" (key), "g" (middle + 1), "g" (middle));
    }

    return key == x && tmp_array[end].contains(x & 0xffff);
}

RoaringBitmap RoaringBitmap::operator&(const RoaringBitmap& b) const
{
    RoaringBitmap answer(false);

    uint32_t pos1 = 0, pos2 = 0;
    uint32_t length1 = size_, length2 = b.size_;

    array_type array1 = getArray(), array2 = b.getArray();

    if (pos1 < length1 && pos2 < length2)
    {
        uint32_t s1 = array1[pos1].getKey();
        uint32_t s2 = array2[pos2].getKey();

        while (true)
        {
            if (s1 < s2)
            {
                if (++pos1 == length1) break;
                s1 = array1[pos1].getKey();
            }
            else if (s1 > s2)
            {
                if (++pos2 == length2) break;
                s2 = array2[pos2].getKey();
            }
            else
            {
                answer.append(array1[pos1] & array2[pos2]);

                if (++pos1 == length1 || ++pos2 == length2) break;

                s1 = array1[pos1].getKey();
                s2 = array2[pos2].getKey();
            }
        }
    }

    return answer;
}

RoaringBitmap RoaringBitmap::operator|(const RoaringBitmap& b) const
{
    RoaringBitmap answer(false);

    uint32_t pos1 = 0, pos2 = 0;
    uint32_t length1 = size_, length2 = b.size_;

    array_type array1 = getArray(), array2 = b.getArray();

    if (pos1 < length1 && pos2 < length2)
    {
        uint32_t s1 = array1[pos1].getKey();
        uint32_t s2 = array2[pos2].getKey();

        while (true)
        {
            if (s1 < s2)
            {
                uint32_t old_pos = pos1;
                do
                {
                    if (++pos1 == length1) break;
                    s1 = array1[pos1].getKey();
                } while (s1 < s2);
                answer.appendCopy(array1, old_pos, pos1);

                if (pos1 == length1) break;
            }
            else if (s1 > s2)
            {
                uint32_t old_pos = pos2;
                do
                {
                    if (++pos2 == length2) break;
                    s2 = array2[pos2].getKey();
                } while (s1 > s2);
                answer.appendCopy(array2, old_pos, pos2);

                if (pos2 == length2) break;
            }
            else
            {
                answer.append(array1[pos1] | array2[pos2]);

                ++pos1;
                ++pos2;

                if (pos1 == length1 || pos2 == length2) break;

                s1 = array1[pos1].getKey();
                s2 = array2[pos2].getKey();
            }
        }
    }

    if (pos1 == length1)
    {
        answer.appendCopy(array2, pos2, length2);
    }
    else if (pos2 == length2)
    {
        answer.appendCopy(array1, pos1, length1);
    }

    return answer;
}

RoaringBitmap RoaringBitmap::operator^(const RoaringBitmap& b) const
{
    RoaringBitmap answer(false);

    uint32_t pos1 = 0, pos2 = 0;
    uint32_t length1 = size_, length2 = b.size_;

    array_type array1 = getArray(), array2 = b.getArray();

    if (pos1 < length1 && pos2 < length2)
    {
        uint32_t s1 = array1[pos1].getKey();
        uint32_t s2 = array2[pos2].getKey();

        while (true)
        {
            if (s1 < s2)
            {
                uint32_t old_pos = pos1;
                do
                {
                    if (++pos1 == length1) break;
                    s1 = array1[pos1].getKey();
                } while (s1 < s2);
                answer.appendCopy(array1, old_pos, pos1);

                if (pos1 == length1) break;
            }
            else if (s1 > s2)
            {
                uint32_t old_pos = pos2;
                do
                {
                    if (++pos2 == length2) break;
                    s2 = array2[pos2].getKey();
                } while (s1 > s2);
                answer.appendCopy(array2, old_pos, pos2);

                if (pos2 == length2) break;
            }
            else
            {
                answer.append(array1[pos1] ^ array2[pos2]);

                ++pos1;
                ++pos2;

                if (pos1 == length1 || pos2 == length2) break;

                s1 = array1[pos1].getKey();
                s2 = array2[pos2].getKey();
            }
        }
    }

    if (pos1 == length1)
    {
        answer.appendCopy(array2, pos2, length2);
    }
    else if (pos2 == length2)
    {
        answer.appendCopy(array1, pos1, length1);
    }

    return answer;
}

RoaringBitmap RoaringBitmap::operator-(const RoaringBitmap& b) const
{
    RoaringBitmap answer(false);

    uint32_t pos1 = 0, pos2 = 0;
    uint32_t length1 = size_, length2 = b.size_;

    array_type array1 = getArray(), array2 = b.getArray();

    if (pos1 < length1 && pos2 < length2)
    {
        uint32_t s1 = array1[pos1].getKey();
        uint32_t s2 = array2[pos2].getKey();

        while (true)
        {
            if (s1 < s2)
            {
                uint32_t old_pos = pos1;
                do
                {
                    if (++pos1 == length1) break;
                    s1 = array1[pos1].getKey();
                } while (s1 < s2);
                answer.appendCopy(array1, old_pos, pos1);

                if (pos1 == length1) break;
            }
            else if (s1 > s2)
            {
                if (++pos2 == length2) break;
                s2 = array2[pos2].getKey();
            }
            else
            {
                answer.append(array1[pos1] - array2[pos2]);

                if (++pos1 == length1 || ++pos2 == length2) break;

                s1 = array1[pos1].getKey();
                s2 = array2[pos2].getKey();
            }
        }
    }

    if (pos2 == length2)
    {
        answer.appendCopy(array1, pos1, length1);
    }

    return answer;
}

void RoaringBitmap::swap(RoaringBitmap& b)
{
    std::swap(capacity_, b.capacity_);
    std::swap(size_, b.size_);
    std::swap(cardinality_, b.cardinality_);
    array_.swap(b.array_);
}

NS_IZENELIB_IR_END
