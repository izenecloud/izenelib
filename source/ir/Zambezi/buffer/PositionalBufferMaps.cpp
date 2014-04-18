#include <ir/Zambezi/buffer/PositionalBufferMaps.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

PositionalBufferMaps::PositionalBufferMaps(uint32_t initialSize, IndexType type)
    : type(type)
    , capacity(initialSize)
    , docid(initialSize)
    , tf(initialSize)
    , position(initialSize)
    , posBlockCount(initialSize)
    , tailPointer(initialSize, UNDEFINED_POINTER)
{
}

PositionalBufferMaps::~PositionalBufferMaps()
{
}

void PositionalBufferMaps::save(std::ostream& ostr) const
{
    ostr.write((const char*)&capacity, sizeof(capacity));

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        uint32_t buffer_cap = docid[termNum].capacity();
        ostr.write((const char*)&buffer_cap, sizeof(buffer_cap));

        if (buffer_cap == 0) break;

        uint32_t size = docid[termNum].size();
        ostr.write((const char*)&size, sizeof(size));

        ostr.write((const char*)&docid[termNum][0], sizeof(docid[0][0]) * size);

        if (type != NON_POSITIONAL)
        {
            ostr.write((const char*)&tf[termNum][0], sizeof(tf[0][0]) * size);
        }

        if (type == POSITIONAL)
        {
            ostr.write((const char*)&posBlockCount[termNum][0], sizeof(posBlockCount[0][0]) * size);

            buffer_cap = position[termNum].capacity();
            ostr.write((const char*)&buffer_cap, sizeof(buffer_cap));

            size = position[termNum].size();
            ostr.write((const char*)&size, sizeof(size));

            ostr.write((const char*)&position[termNum][0], sizeof(position[0][0]) * size);
        }
    }

    ostr.write((const char*)&tailPointer[0], sizeof(tailPointer[0]) * termNum);
}

void PositionalBufferMaps::load(std::istream& istr)
{
    istr.read((char*)&capacity, sizeof(capacity));

    docid.resize(capacity);
    if (type != NON_POSITIONAL)
    {
        tf.resize(capacity);
    }
    if (type == POSITIONAL)
    {
        position.resize(capacity);
        posBlockCount.resize(capacity);
    }

    size_t termNum;
    for (termNum = 0; termNum < capacity; ++termNum)
    {
        uint32_t buffer_cap = 0;
        istr.read((char*)&buffer_cap, sizeof(buffer_cap));

        if (buffer_cap == 0) break;

        docid[termNum].reserve(buffer_cap);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(size));
        docid[termNum].resize(size);

        istr.read((char*)&docid[termNum][0], sizeof(docid[0][0]) * size);

        if (type != NON_POSITIONAL)
        {
            tf[termNum].reserve(buffer_cap);
            tf[termNum].resize(size);
            istr.read((char*)&tf[termNum][0], sizeof(tf[0][0]) * size);
        }

        if (type == POSITIONAL)
        {
            posBlockCount[termNum].reserve(buffer_cap);
            posBlockCount[termNum].resize(size);
            istr.read((char*)&posBlockCount[termNum][0], sizeof(posBlockCount[0][0]) * size);

            buffer_cap = 0;
            istr.read((char*)&buffer_cap, sizeof(buffer_cap));
            position[termNum].reserve(buffer_cap);

            size = 0;
            istr.read((char*)&size, sizeof(size));
            position[termNum].resize(size);
            istr.read((char*)&position[termNum][0], sizeof(position[0][0]) * size);
        }
    }

    tailPointer.resize(capacity, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer[0], sizeof(tailPointer[0]) * termNum);
}

void PositionalBufferMaps::expand(uint32_t newSize)
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

    docid.resize(capacity);
    tailPointer.resize(capacity, UNDEFINED_POINTER);

    if (type != NON_POSITIONAL)
    {
        tf.resize(capacity);
    }

    if (type == POSITIONAL)
    {
        position.resize(capacity);
        posBlockCount.resize(capacity);
    }
}

bool PositionalBufferMaps::containsKey(uint32_t k) const
{
    return !docid[k].empty();
}

uint32_t PositionalBufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
{
    do
    {
        if (++pos >= capacity)
        {
            return -1;
        }
    }
    while (docid[pos].empty() || docid[pos].capacity() <= minLength);

    return pos;
}

}

NS_IZENELIB_IR_END
