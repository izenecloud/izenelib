#include <ir/Zambezi/buffer/BufferMaps.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

BufferMaps::BufferMaps(uint32_t initialSize, IndexType type)
    : type_(type)
    , capacity_(initialSize)
    , docid_(initialSize)
    , tailPointer_(initialSize, UNDEFINED_POINTER)
{
    if (type != NON_POSITIONAL)
    {
        tf_.resize(initialSize);
    }

    if (type == POSITIONAL)
    {
        position_.resize(initialSize);
        posBlockHead_.resize(initialSize);
    }
}

BufferMaps::~BufferMaps()
{
}

void BufferMaps::save(std::ostream& ostr) const
{
}

void BufferMaps::load(std::istream& istr)
{
}

void BufferMaps::expand(uint32_t newSize)
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

bool BufferMaps::containsKey(uint32_t k) const
{
    return !docid_[k].empty();
}

std::vector<uint32_t>& BufferMaps::getDocidList(uint32_t k)
{
    expand(k + 1);
    return docid_[k];
}

const std::vector<uint32_t>& BufferMaps::getDocidList(uint32_t k) const
{
    return docid_[k];
}

std::vector<uint32_t>& BufferMaps::getTfList(uint32_t k)
{
    expand(k + 1);
    return tf_[k];
}

const std::vector<uint32_t>& BufferMaps::getTfList(uint32_t k) const
{
    return tf_[k];
}

std::vector<uint32_t>& BufferMaps::getPositionList(uint32_t k)
{
    expand(k + 1);
    return position_[k];
}

const std::vector<uint32_t>& BufferMaps::getPositionList(uint32_t k) const
{
    return position_[k];
}

uint32_t BufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
{
    do
    {
        if (++pos >= capacity_)
        {
            return -1;
        }
    }
    while (docid_[pos].capacity() < minLength);

    return pos;
}

}

NS_IZENELIB_IR_END
