#include <ir/Zambezi/buffer/AttrScoreBufferMaps.hpp>

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

void AttrScoreBufferMaps::save(std::ostream& ostr) const
{
    ostr.write((const char*)&capacity, sizeof(capacity));

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        if (!buffer[termNum])
        {
            uint32_t capacity = 0;
            ostr.write((const char*)&capacity, sizeof(uint32_t));
            break;
        }

        uint32_t capacity = buffer[termNum]->capacity();
        ostr.write((const char*)&capacity, sizeof(uint32_t));

        uint32_t size = buffer[termNum]->size();
        ostr.write((const char*)&size, sizeof(uint32_t));

        ostr.write((const char*)&(*buffer[termNum])[0], sizeof(ElemType) * size);
    }

    ostr.write((const char*)&tailPointer[0], sizeof(size_t) * termNum);
}

void AttrScoreBufferMaps::load(std::istream& istr)
{
    istr.read((char*)&capacity, sizeof(capacity));
    flags.reset(new boost::atomic_flag[capacity]);
    buffer.resize(capacity);

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        uint32_t capacity = 0;
        istr.read((char*)&capacity, sizeof(uint32_t));

        if (capacity == 0) break;

        resizePosting(termNum, capacity);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(uint32_t));
        buffer[termNum]->resize(size);

        istr.read((char*)&(*buffer[termNum])[0], sizeof(uint32_t) * size);
    }

    tailPointer.resize(capacity, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer[0], sizeof(size_t) * termNum);
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
    while (buffer[pos]->empty() || buffer[pos]->capacity() <= minLength);

    return pos;
}

boost::shared_ptr<AttrScoreBufferMaps::PostingType> AttrScoreBufferMaps::getPosting(uint32_t id) const
{
    boost::atomic_flag& flag = flags[id];
    while (flag.test_and_set());
    boost::shared_ptr<PostingType> res(buffer[id]);
    flag.clear();

    return res;
}

void AttrScoreBufferMaps::resizePosting(uint32_t id, size_t new_size)
{
    boost::shared_ptr<PostingType>& old_buffer = buffer[id];
    boost::shared_ptr<PostingType> new_buffer(new PostingType);
    new_buffer->reserve(new_size);
    new_buffer->assign(old_buffer->begin(), old_buffer->end());

    boost::atomic_flag& flag = flags[id];
    while (flag.test_and_set());
    old_buffer.swap(new_buffer);
    flag.clear();
}

}

NS_IZENELIB_IR_END
