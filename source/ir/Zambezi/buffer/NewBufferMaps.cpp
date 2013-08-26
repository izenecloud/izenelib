#include <ir/Zambezi/buffer/NewBufferMaps.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

NewBufferMaps::NewBufferMaps(uint32_t initialSize)
    : capacity_(initialSize)
    , docid_(initialSize)
    , tailPointer_(initialSize, UNDEFINED_POINTER)
{
}

NewBufferMaps::~NewBufferMaps()
{
}

void NewBufferMaps::save(std::ostream& ostr) const
{
}

void NewBufferMaps::load(std::istream& istr)
{
}

void NewBufferMaps::expand(uint32_t newSize)
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
    score_.resize(capacity_);
    tailPointer_.resize(capacity_, UNDEFINED_POINTER);
}

bool NewBufferMaps::containsKey(uint32_t k) const
{
    return !docid_[k].empty();
}

uint32_t NewBufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
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
