#ifndef _IZENELIB_AM_SUCCINCT_DBITV_DBITV_HPP
#define _IZENELIB_AM_SUCCINCT_DBITV_DBITV_HPP

#include <types.h>

#include <vector>
#include <iostream>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dense
{

class DBitV
{
public:
    DBitV(bool support_select);
    ~DBitV();

    void build(const std::vector<uint64_t> &bv, size_t len);
    void clear();

    bool lookup(size_t pos) const;
    bool lookup(size_t pos, size_t &r) const;

    size_t rank0(size_t pos) const;
    size_t rank1(size_t pos) const;
    size_t rank(size_t pos, bool bit) const;

    size_t select0(size_t ind) const;
    size_t select1(size_t ind) const;
    size_t select(size_t ind, bool bit) const;

    void save(std::ostream &os) const;
    void load(std::istream &is);

    inline size_t one_count() const
    {
        return one_count_;
    }

    inline size_t zero_count() const
    {
        return len_ - one_count_;
    }

    inline size_t length() const
    {
        return len_;
    }

    inline size_t bsize() const
    {
        return bits_.size();
    }

    size_t allocSize() const;

private:
    void buildBlock_(uint64_t block, size_t offset, size_t &rank_lb);

    template <class T>
    void save(std::ostream &os, const std::vector<T> &vs) const
    {
        size_t size = vs.size();
        os.write((const char *)&size, sizeof(size));
        os.write((const char *)&vs[0], sizeof(vs[0]) * size);
    }

    template <class T>
    void load(std::istream &is, std::vector<T> &vs)
    {
        size_t size = 0;
        is.read((char *)&size, sizeof(size));
        vs.resize(size);
        is.read((char *)&vs[0], sizeof(vs[0]) * size);
    }

    bool support_select_;
    size_t len_;
    size_t one_count_;

    std::vector<uint64_t> bits_;
    std::vector<size_t> rank_blocks_;
    std::vector<uint16_t> rank_small_blocks_;

    std::vector<size_t> select_one_inds_;
    std::vector<size_t> select_zero_inds_;
};

}
}

NS_IZENELIB_AM_END

#endif
