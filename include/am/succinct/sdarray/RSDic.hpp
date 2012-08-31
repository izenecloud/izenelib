#ifndef SDARRAY_RSDIC_HPP__
#define SDARRAY_RSDIC_HPP__

#include "SDArray.hpp"


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace sdarray
{

class RSDic
{
    static const size_t BLOCKSIZE;

public:
    RSDic();
    ~RSDic();

    void add(size_t val);
    void build(const std::vector<size_t>& bv);
    void build(const std::vector<size_t>& bv, size_t size);

    size_t rank0(size_t pos) const;
    size_t rank1(size_t pos) const;

    size_t select0(size_t num) const;
    size_t select1(size_t num) const;

    size_t size() const;
    size_t allocSize() const;

    void save(std::ostream& os) const;
    void load(std::istream& is);

private:
    SDArray sda_;
};

}
}

NS_IZENELIB_AM_END

#endif // SDARRAY_RSDIC_HPP__
