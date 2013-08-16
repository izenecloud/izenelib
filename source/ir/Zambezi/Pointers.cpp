#include <ir/Zambezi/Pointers.hpp>
#include <ir/Zambezi/SegmentPool.hpp>
#include <ir/Zambezi/Consts.hpp>

#include <cmath>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

Pointers::Pointers(uint32_t size)
    : totalDocs_(0)
    , totalDocLen_(0)
    , df_(size, 0)
    , cf_(size, 0)
    , headPointers_(size, UNDEFINED_POINTER)
    , docLen_(size, 0)
    , maxTf_(size, 0)
    , maxTfDocLen_(size, 0)
{
    updateDefaultValues_();
}

Pointers::~Pointers()
{
}

void Pointers::save(std::ostream& ostr) const
{
    uint32_t size = headPointers_.size();
    ostr.write((const char*)&size, sizeof(uint32_t));
    uint32_t term = -1;
    while ((term = headPointers_.nextIndex(term)) != (uint32_t)-1)
    {
        ostr.write((const char*)&term, sizeof(uint32_t));
        ostr.write((const char*)&df_.get(term), sizeof(uint32_t));
        ostr.write((const char*)&cf_.get(term), sizeof(size_t));
        ostr.write((const char*)&headPointers_.get(term), sizeof(size_t));
        ostr.write((const char*)&maxTf_.get(term), sizeof(uint32_t));
        ostr.write((const char*)&maxTfDocLen_.get(term), sizeof(uint32_t));
    }

    size = docLen_.size();
    ostr.write((const char*)&size, sizeof(uint32_t));

    while ((term = docLen_.nextIndex(term)) != (uint32_t)-1)
    {
        ostr.write((const char*)&term, sizeof(uint32_t));
        ostr.write((const char*)&docLen_.get(term), sizeof(uint32_t));
    }

    ostr.write((const char*)&totalDocs_, sizeof(uint32_t));
    ostr.write((const char*)&totalDocLen_, sizeof(size_t));
}

void Pointers::load(std::istream& istr)
{
    uint32_t size = 0;
    istr.read((char*)&size, sizeof(uint32_t));
    uint32_t term, value;
    size_t pointer, cf;
    for (uint32_t i = 0; i < size; i++)
    {
        istr.read((char*)&term, sizeof(uint32_t));
        istr.read((char*)&value, sizeof(uint32_t));
        df_.set(term, value);
        istr.read((char*)&cf, sizeof(size_t));
        cf_.set(term, cf);
        istr.read((char*)&pointer, sizeof(size_t));
        headPointers_.set(term, pointer);
        istr.read((char*)&value, sizeof(uint32_t));
        maxTf_.set(term, value);
        istr.read((char*)&value, sizeof(uint32_t));
        maxTfDocLen_.set(term, value);
    }

    istr.read((char*)&size, sizeof(uint32_t));
    for (uint32_t i = 0; i < size; i++)
    {
        istr.read((char*)&term, sizeof(uint32_t));
        istr.read((char*)&value, sizeof(uint32_t));
        docLen_.set(term, value);
    }

    istr.read((char*)&totalDocs_, sizeof(uint32_t));
    istr.read((char*)&totalDocLen_, sizeof(size_t));

    updateDefaultValues_();
}

void Pointers::updateDefaultValues_()
{
    defaultDf_ = totalDocs_ / 100;
    defaultIdf_ = logf((totalDocs_ - defaultDf_ + 0.5f) / (defaultDf_ + 0.5f));
    defaultCf_ = (size_t) defaultDf_ * 2;
}

}

NS_IZENELIB_IR_END
