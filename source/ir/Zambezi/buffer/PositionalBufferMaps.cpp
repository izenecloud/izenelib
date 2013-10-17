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
    , tailPointer_(initialSize, UNDEFINED_POINTER)
    , posBlockHead_(initialSize)
{
}

PositionalBufferMaps::~PositionalBufferMaps()
{
}

void PositionalBufferMaps::save(std::ostream& ostr) const
{
    ostr.write((const char*)&capacity_, sizeof(capacity_));

    for (size_t i = 0; i < capacity_; ++i)
    {
        uint32_t capacity = docid_[i].capacity();
        ostr.write((const char*)&capacity, sizeof(uint32_t));

        uint32_t size = docid_[i].size();
        ostr.write((const char*)&size, sizeof(uint32_t));

        ostr.write((const char*)&docid_[i][0], sizeof(uint32_t) * size);

        if (type_ != NON_POSITIONAL)
        {
            ostr.write((const char*)&tf_[i][0], sizeof(uint32_t) * size);
        }

        if (type_ == POSITIONAL)
        {
            capacity = position_[i].capacity();
            ostr.write((const char*)&capacity, sizeof(uint32_t));
            size = position_[i].size();
            ostr.write((const char*)&size, sizeof(uint32_t));
            ostr.write((const char*)&position_[i][0], sizeof(uint32_t) * size);
            ostr.write((const char*)&posBlockHead_[i], sizeof(uint32_t));
        }
    }

    ostr.write((const char*)&tailPointer_[0], sizeof(size_t) * capacity_);
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
        posBlockHead_.resize(capacity_);
    }

    for (size_t i = 0; i < capacity_; ++i)
    {
        uint32_t capacity = 0;
        istr.read((char*)&capacity, sizeof(uint32_t));
        docid_[i].reserve(capacity);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(uint32_t));
        docid_[i].resize(size);

        istr.read((char*)&docid_[i][0], sizeof(uint32_t) * size);

        if (type_ != NON_POSITIONAL)
        {
            tf_[i].reserve(capacity);
            tf_[i].resize(size);
            istr.read((char*)&tf_[i][0], sizeof(uint32_t) * size);
        }

        if (type_ == POSITIONAL)
        {
            capacity = 0;
            istr.read((char*)&capacity, sizeof(uint32_t));
            position_.reserve(capacity);

            size = 0;
            istr.read((char*)&size, sizeof(uint32_t));
            position_.resize(size);

            istr.read((char*)&position_[i][0], sizeof(uint32_t) * size);
            istr.read((char*)&posBlockHead_[i], sizeof(uint32_t));
        }
    }

    tailPointer_.resize(capacity_, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer_[0], sizeof(size_t) * capacity_);
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
        posBlockHead_.resize(capacity_);
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
