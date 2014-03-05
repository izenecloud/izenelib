#include <ir/Zambezi/Pointers.hpp>
#include <ir/Zambezi/SegmentPool.hpp>
#include <ir/Zambezi/Consts.hpp>

#include <cmath>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

Pointers::Pointers(uint32_t termNum, uint32_t docNum)
    : totalDocs(0)
    , totalDocLen(0)
    , df(termNum, 0)
    , cf(termNum, 0)
    , headPointers(termNum, UNDEFINED_POINTER)
    , maxTf(termNum, 0)
    , maxTfDocLen(termNum, 0)
    , docLen(docNum, 0)
{
    updateDefaultValues_();
}

Pointers::~Pointers()
{
}

void Pointers::save(std::ostream& ostr) const
{
    uint32_t size = df.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&df.get(0), sizeof(df.get(0)) * size);
    ostr.write((const char*)&cf.get(0), sizeof(cf.get(0)) * size);
    ostr.write((const char*)&headPointers.get(0), sizeof(headPointers.get(0)) * size);

    size = maxTf.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&maxTf.get(0), sizeof(maxTf.get(0)) * size);
    ostr.write((const char*)&maxTfDocLen.get(0), sizeof(maxTfDocLen.get(0)) * size);

    size = docLen.size();
    ostr.write((const char*)&size, sizeof(size));

    ostr.write((const char*)&docLen.get(0), sizeof(docLen.get(0)) * size);

    ostr.write((const char*)&totalDocs, sizeof(totalDocs));
    ostr.write((const char*)&totalDocLen, sizeof(totalDocLen));
}

void Pointers::load(std::istream& istr)
{
    uint32_t size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= df.getCounter().size());

    istr.read((char*)&df.get(0), sizeof(df.get(0)) * size);
    istr.read((char*)&cf.get(0), sizeof(cf.get(0)) * size);
    istr.read((char*)&headPointers.get(0), sizeof(headPointers.get(0)) * size);

    size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= maxTf.getCounter().size());

    istr.read((char*)&maxTf.get(0), sizeof(maxTf.get(0)) * size);
    istr.read((char*)&maxTfDocLen.get(0), sizeof(maxTfDocLen.get(0)) * size);

    size = 0;
    istr.read((char*)&size, sizeof(size));
    assert(size <= docLen.getCounter().size());

    istr.read((char*)&docLen.get(0), sizeof(docLen.get(0)) * size);

    istr.read((char*)&totalDocs, sizeof(totalDocs));
    istr.read((char*)&totalDocLen, sizeof(totalDocLen));

    updateDefaultValues_();
}

void Pointers::updateDefaultValues_()
{
    defaultDf = totalDocs / 100;
    defaultIdf = logf((totalDocs - defaultDf + 0.5f) / (defaultDf + 0.5f));
    defaultCf = (size_t) defaultDf * 2;
}

}

NS_IZENELIB_IR_END
