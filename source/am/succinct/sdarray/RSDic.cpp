#include <am/succinct/sdarray/RSDic.hpp>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace sdarray
{

const size_t RSDic::BLOCKSIZE = 64;

RSDic::RSDic()
{
}

RSDic::~RSDic()
{
}

void RSDic::add(size_t val)
{
    assert(val > 0);
    sda_.add(val);
}

void RSDic::build(const std::vector<size_t>& bv)
{
    build(bv, bv.size() * BLOCKSIZE);
}

void RSDic::build(const std::vector<size_t>& bv, size_t size)
{
    sda_.clear();

    size_t prev = SDArray::NOTFOUND;
    for (size_t i = 0; i < size; ++i)
    {
        if ((bv[i / BLOCKSIZE] >> (i % BLOCKSIZE)) & 1LLU)
        {
            if (prev == SDArray::NOTFOUND)
                sda_.add(i + 1);
            else
                sda_.add(i - prev);
            prev = i;
        }
    }

    sda_.build();
}

size_t RSDic::rank0(size_t pos) const
{
    return pos + 1 - rank1(pos);
}

size_t RSDic::rank1(size_t pos) const
{
    size_t val = sda_.find(pos + 1);
    if (val == SDArray::NOTFOUND)
        val = 0;

    return val;
}

size_t RSDic::select0(size_t num) const
{
    // FIXME
    size_t val = sda_.find(num);
    if (val == SDArray::NOTFOUND)
        return SDArray::NOTFOUND;

    size_t pos = num, pos1 = num + val;
    while (pos < pos1)
    {
        pos = pos1;
        if ((val = sda_.find(pos)) == SDArray::NOTFOUND)
            return SDArray::NOTFOUND;

        pos1 = num + val;
    }

    return pos - 1;
}

size_t RSDic::select1(size_t num) const
{
    return sda_.prefixSum(num) - 1;
}

size_t RSDic::size() const
{
    return sda_.size();
}

size_t RSDic::allocSize() const
{
    return sda_.allocSize();
}

void RSDic::save(std::ostream& os) const
{
    sda_.save(os);
}

void RSDic::load(std::istream& is)
{
    sda_.load(is);
}

}
}

NS_IZENELIB_AM_END
