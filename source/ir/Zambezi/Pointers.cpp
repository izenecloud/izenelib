#include <ir/Zambezi/Pointers.hpp>
#include <ir/Zambezi/SegmentPool.hpp>
#include <ir/Zambezi/Consts.hpp>

#include <cmath>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

Pointers::Pointers(uint32_t termNum, uint32_t docNum)
    : totalDocs_(0)
    , totalDocLen_(0)
    , df_(termNum, 0)
    , cf_(termNum, 0)
    , headPointers_(termNum, UNDEFINED_POINTER)
    , maxTf_(termNum, 0)
    , maxTfDocLen_(termNum, 0)
    , docLen_(docNum, 0)
{
    updateDefaultValues_();
}

Pointers::~Pointers()
{
}

void Pointers::save(std::ostream& ostr) const
{
    uint32_t size = df_.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&df_.get(0), sizeof(df_.get(0)) * size);
    ostr.write((const char*)&cf_.get(0), sizeof(cf_.get(0)) * size);
    ostr.write((const char*)&headPointers_.get(0), sizeof(headPointers_.get(0)) * size);

    size = maxTf_.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&maxTf_.get(0), sizeof(maxTf_.get(0)) * size);
    ostr.write((const char*)&maxTfDocLen_.get(0), sizeof(maxTfDocLen_.get(0)) * size);

    size = docLen_.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&docLen_.get(0), sizeof(docLen_.get(0)) * size);

    ostr.write((const char*)&totalDocs_, sizeof(totalDocs_));
    ostr.write((const char*)&totalDocLen_, sizeof(totalDocLen_));
}

void Pointers::load(std::istream& istr)
{
    uint32_t size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= df_.getCounter().size());

    istr.read((char*)&df_.get(0), sizeof(df_.get(0)) * size);
    istr.read((char*)&cf_.get(0), sizeof(cf_.get(0)) * size);
    istr.read((char*)&headPointers_.get(0), sizeof(headPointers_.get(0)) * size);

    size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= maxTf_.getCounter().size());

    istr.read((char*)&maxTf_.get(0), sizeof(maxTf_.get(0)) * size);
    istr.read((char*)&maxTfDocLen_.get(0), sizeof(maxTfDocLen_.get(0)) * size);

    size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= docLen_.getCounter().size());

    istr.read((char*)&docLen_.get(0), sizeof(docLen_.get(0)) * size);

    istr.read((char*)&totalDocs_, sizeof(totalDocs_));
    istr.read((char*)&totalDocLen_, sizeof(totalDocLen_));

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
