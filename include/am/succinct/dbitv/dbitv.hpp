#ifndef _IZENELIB_AM_SUCCINCT_DBITV_DBITV_HPP
#define _IZENELIB_AM_SUCCINCT_DBITV_DBITV_HPP

#include <am/succinct/constants.hpp>

#include <boost/shared_array.hpp>
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

    bool access(size_t pos) const;
    bool access(size_t pos, size_t &r) const;

    size_t rank0(size_t pos) const;
    size_t rank1(size_t pos) const;
    size_t rank(size_t pos, bool bit) const;

    size_t select0(size_t ind) const;
    size_t select1(size_t ind) const;
    size_t select(size_t ind, bool bit) const;

    void save(std::ostream &os) const;
    void load(std::istream &is);

    inline size_t length() const
    {
        return len_;
    }

    inline size_t one_count() const
    {
        return one_count_;
    }

    inline size_t zero_count() const
    {
        return len_ - one_count_;
    }

    size_t allocSize() const;

private:
    struct SuperBlock
    {
        uint64_t rank_;
        uint64_t subrank_;
        uint64_t bits_[kBlockPerSuperBlock];

        SuperBlock() : rank_(), subrank_(), bits_() {}
    };

    void buildBlock_(uint64_t block, size_t bits, size_t sb_ind, size_t sb_off, size_t& rank_sb);

    bool support_select_;
    size_t len_;
    size_t one_count_;

    boost::shared_array<SuperBlock> super_blocks_;

    std::vector<size_t> select_one_inds_;
    std::vector<size_t> select_zero_inds_;
};

}
}

NS_IZENELIB_AM_END

#endif
