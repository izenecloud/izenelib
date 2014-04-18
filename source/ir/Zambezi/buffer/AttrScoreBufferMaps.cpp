#include <ir/Zambezi/buffer/AttrScoreBufferMaps.hpp>
#include <ir/Zambezi/Utils.hpp>

#include <util/mem_utils.h>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

AttrScoreBufferMaps::AttrScoreBufferMaps(uint32_t initialSize)
    : capacity(initialSize)
    , flags(new boost::atomic_flag[initialSize])
    , buffer(initialSize)
    , tailPointer(initialSize, UNDEFINED_POINTER)
{
}

AttrScoreBufferMaps::~AttrScoreBufferMaps()
{
}

void AttrScoreBufferMaps::save(std::ostream& ostr, bool reverse) const
{
    ostr.write((const char*)&capacity, sizeof(capacity));

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        if (!buffer[termNum])
        {
            uint32_t buffer_cap = 0;
            ostr.write((const char*)&buffer_cap, sizeof(buffer_cap));
            break;
        }

        uint32_t buffer_cap = buffer[termNum][0];
        ostr.write((const char*)&buffer_cap, sizeof(buffer_cap));

        uint32_t size = buffer[termNum][1];
        ostr.write((const char*)&size, sizeof(size));

        if (reverse)
        {
            ostr.write((const char*)&buffer[termNum][4 + buffer_cap - size], sizeof(buffer[0][0]) * size);
            ostr.write((const char*)&buffer[termNum][4 + buffer_cap * 2 - size], sizeof(buffer[0][0]) * size);
        }
        else
        {
            ostr.write((const char*)&buffer[termNum][4], sizeof(buffer[0][0]) * size);
            ostr.write((const char*)&buffer[termNum][4 + buffer_cap], sizeof(buffer[0][0]) * size);
        }
    }

    ostr.write((const char*)&tailPointer[0], sizeof(tailPointer[0]) * termNum);
}

void AttrScoreBufferMaps::load(std::istream& istr, bool reverse)
{
    istr.read((char*)&capacity, sizeof(capacity));
    flags.reset(new boost::atomic_flag[capacity]);
    buffer.resize(capacity);

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        uint32_t buffer_cap = 0;
        istr.read((char*)&buffer_cap, sizeof(buffer_cap));

        if (buffer_cap == 0) break;

        resetBuffer(termNum, buffer_cap, reverse, false);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(size));

        if (reverse)
        {
            istr.read((char*)&buffer[termNum][4 + buffer_cap - size], sizeof(buffer[0][0]) * size);
            istr.read((char*)&buffer[termNum][4 + buffer_cap * 2 - size], sizeof(buffer[0][0]) * size);
        }
        else
        {
            istr.read((char*)&buffer[termNum][4], sizeof(buffer[0][0]) * size);
            istr.read((char*)&buffer[termNum][4 + buffer_cap], sizeof(buffer[0][0]) * size);
        }

        buffer[termNum][1] = size;
    }

    tailPointer.resize(capacity, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer[0], sizeof(tailPointer[0]) * termNum);
}

void AttrScoreBufferMaps::expand(uint32_t newSize)
{
    if (newSize <= capacity) return;

    if (capacity == 0)
    {
        capacity = newSize;
    }
    else
    {
        while (newSize > capacity)
        {
            capacity *= 2;
        }
    }

    buffer.resize(capacity);
    tailPointer.resize(capacity, UNDEFINED_POINTER);
}

uint32_t AttrScoreBufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
{
    do
    {
        if (++pos >= capacity)
        {
            return UNDEFINED_OFFSET;
        }
    }
    while (!buffer[pos] || buffer[pos][1] == 0 || buffer[pos][0] <= minLength);

    return pos;
}

boost::shared_array<uint32_t> AttrScoreBufferMaps::getBuffer(uint32_t id) const
{
    boost::atomic_flag& flag = flags[id];
    while (flag.test_and_set());
    boost::shared_array<uint32_t> res(buffer[id]);
    flag.clear();

    return res;
}

void AttrScoreBufferMaps::resetBuffer(uint32_t id, uint32_t new_cap, bool reverse, bool copy)
{
    boost::shared_array<uint32_t> new_buffer(cachealign_alloc<uint32_t>(new_cap * 2 + 4, 16), cachealign_deleter());
    memset(&new_buffer[1], 0, sizeof(new_buffer[0]) * (new_cap * 2 + 3));
    new_buffer[0] = new_cap;

    boost::shared_array<uint32_t>& old_buffer = buffer[id];
    if (copy && old_buffer && old_buffer[1] > 0)
    {
        uint32_t old_cap = old_buffer[0];
        uint32_t old_size = old_buffer[1];

        new_buffer[1] = old_size;

        if (reverse)
        {
            memcpy(&new_buffer[4 + new_cap - old_size], &old_buffer[4 + old_cap - old_size], sizeof(new_buffer[0]) * old_size);
            memcpy(&new_buffer[4 + new_cap * 2 - old_size], &old_buffer[4 + old_cap * 2 - old_size], sizeof(new_buffer[0]) * old_size);
        }
        else
        {
            memcpy(&new_buffer[4], &old_buffer[4], sizeof(new_buffer[0]) * old_size);
            memcpy(&new_buffer[4 + new_cap], &old_buffer[4 + old_cap], sizeof(new_buffer[0]) * old_size);
        }
    }

    boost::atomic_flag& flag = flags[id];
    while (flag.test_and_set());
    old_buffer.swap(new_buffer);
    flag.clear();
}

}

NS_IZENELIB_IR_END
