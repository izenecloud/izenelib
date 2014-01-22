#include <ir/Zambezi/buffer/PositionalBufferMaps.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

PositionalBufferMaps::PositionalBufferMaps(uint32_t initialSize, IndexType type)
    : type_(type)
    , capacity_(initialSize)
    , docid_(initialSize)
    , tf_(initialSize)
    , position_(initialSize)
    , posBlockCount_(initialSize)
    , tailPointer_(initialSize, UNDEFINED_POINTER)
{
}

PositionalBufferMaps::~PositionalBufferMaps()
{
}

void PositionalBufferMaps::save(std::ostream& ostr) const
{
    ostr.write((const char*)&capacity_, sizeof(capacity_));

    size_t termNum;
    for (termNum = 0; termNum < capacity_; ++termNum)
    {
        uint32_t capacity = docid_[termNum].capacity();
        ostr.write((const char*)&capacity, sizeof(capacity));

        if (capacity == 0) break;

        uint32_t size = docid_[termNum].size();
        ostr.write((const char*)&size, sizeof(size));

        ostr.write((const char*)&docid_[termNum][0], sizeof(docid_[0][0]) * size);

        if (type_ != NON_POSITIONAL)
        {
            ostr.write((const char*)&tf_[termNum][0], sizeof(tf_[0][0]) * size);
        }

        if (type_ == POSITIONAL)
        {
            ostr.write((const char*)&posBlockCount_[termNum][0], sizeof(posBlockCount_[0][0]) * size);

            capacity = position_[termNum].capacity();
            ostr.write((const char*)&capacity, sizeof(capacity));

            size = position_[termNum].size();
            ostr.write((const char*)&size, sizeof(size));

            ostr.write((const char*)&position_[termNum][0], sizeof(position_[0][0]) * size);
        }
    }

    ostr.write((const char*)&tailPointer_[0], sizeof(tailPointer_[0]) * termNum);
}

void PositionalBufferMaps::load(std::istream& istr)
{
    istr.read((char*)&capacity_, sizeof(capacity_));

    docid_.resize(capacity_);
    if (type_ != NON_POSITIONAL)
    {
        tf_.resize(capacity_);
    }
    if (type_ == POSITIONAL)
    {
        position_.resize(capacity_);
        posBlockCount_.resize(capacity_);
    }

    size_t termNum;
    for (termNum = 0; termNum < capacity_; ++termNum)
    {
        uint32_t capacity = 0;
        istr.read((char*)&capacity, sizeof(capacity));

        if (capacity == 0) break;

        docid_[termNum].reserve(capacity);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(size));
        docid_[termNum].resize(size);

        istr.read((char*)&docid_[termNum][0], sizeof(docid_[0][0]) * size);

        if (type_ != NON_POSITIONAL)
        {
            tf_[termNum].reserve(capacity);
            tf_[termNum].resize(size);
            istr.read((char*)&tf_[termNum][0], sizeof(tf_[0][0]) * size);
        }

        if (type_ == POSITIONAL)
        {
            posBlockCount_[termNum].reserve(capacity);
            posBlockCount_[termNum].resize(size);
            istr.read((char*)&posBlockCount_[termNum][0], sizeof(posBlockCount_[0][0]) * size);

            capacity = 0;
            istr.read((char*)&capacity, sizeof(capacity));
            position_[termNum].reserve(capacity);

            size = 0;
            istr.read((char*)&size, sizeof(size));
            position_[termNum].resize(size);
            istr.read((char*)&position_[termNum][0], sizeof(position_[0][0]) * size);
        }
    }

    tailPointer_.resize(capacity_, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer_[0], sizeof(tailPointer_[0]) * termNum);
}

void PositionalBufferMaps::expand(uint32_t newSize)
{
    if (newSize <= capacity_) return;

    if (capacity_ == 0)
    {
        capacity_ = newSize;
    }
    else
    {
        while (newSize > capacity_)
        {
            capacity_ *= 2;
        }
    }

    docid_.resize(capacity_);
    tailPointer_.resize(capacity_, UNDEFINED_POINTER);

    if (type_ != NON_POSITIONAL)
    {
        tf_.resize(capacity_);
    }

    if (type_ == POSITIONAL)
    {
        position_.resize(capacity_);
        posBlockCount_.resize(capacity_);
    }
}

bool PositionalBufferMaps::containsKey(uint32_t k) const
{
    return !docid_[k].empty();
}

uint32_t PositionalBufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
{
    do
    {
        if (++pos >= capacity_)
        {
            return -1;
        }
    }
    while (docid_[pos].capacity() <= minLength);

    return pos;
}

}

NS_IZENELIB_IR_END
