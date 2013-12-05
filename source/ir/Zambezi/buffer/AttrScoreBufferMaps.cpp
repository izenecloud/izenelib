#include <ir/Zambezi/buffer/AttrScoreBufferMaps.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

AttrScoreBufferMaps::AttrScoreBufferMaps(uint32_t initialSize)
    : capacity_(initialSize)
    , docid_(initialSize)
    , score_(initialSize)
    , tailPointer_(initialSize, UNDEFINED_POINTER)
{
}

AttrScoreBufferMaps::~AttrScoreBufferMaps()
{
}

void AttrScoreBufferMaps::save(std::ostream& ostr) const
{
    ostr.write((const char*)&capacity_, sizeof(capacity_));

    size_t termNum;
    for (termNum = 0; termNum < capacity_; ++termNum)
    {
        uint32_t capacity = docid_[termNum].capacity();
        ostr.write((const char*)&capacity, sizeof(uint32_t));

        if (capacity == 0) break;

        uint32_t size = docid_[termNum].size();
        ostr.write((const char*)&size, sizeof(uint32_t));

        ostr.write((const char*)&docid_[termNum][0], sizeof(uint32_t) * size);
        ostr.write((const char*)&score_[termNum][0], sizeof(uint32_t) * size);
    }

    ostr.write((const char*)&tailPointer_[0], sizeof(size_t) * termNum);
}

void AttrScoreBufferMaps::load(std::istream& istr)
{
    istr.read((char*)&capacity_, sizeof(capacity_));
    docid_.resize(capacity_);
    score_.resize(capacity_);

    size_t termNum;
    for (termNum = 0; termNum < capacity_; ++termNum)
    {
        uint32_t capacity = 0;
        istr.read((char*)&capacity, sizeof(uint32_t));

        if (capacity == 0) break;

        docid_[termNum].reserve(capacity);
        score_[termNum].reserve(capacity);

        uint32_t size = 0;
        istr.read((char*)&size, sizeof(uint32_t));
        docid_[termNum].resize(size);
        score_[termNum].resize(size);

        istr.read((char*)&docid_[termNum][0], sizeof(uint32_t) * size);
        istr.read((char*)&score_[termNum][0], sizeof(uint32_t) * size);
    }

    tailPointer_.resize(capacity_, UNDEFINED_POINTER);
    istr.read((char*)&tailPointer_[0], sizeof(size_t) * termNum);
}

void AttrScoreBufferMaps::expand(uint32_t newSize)
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

uint32_t AttrScoreBufferMaps::nextIndex(uint32_t pos, uint32_t minLength) const
{
    do
    {
        if (++pos >= capacity_)
        {
            return UNDEFINED_OFFSET;
        }
    }
    while (docid_[pos].empty() || docid_[pos].capacity() <= minLength);

    return pos;
}

}

NS_IZENELIB_IR_END
