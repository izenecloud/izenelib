#include <am/succinct/dbitv/dbitv.hpp>
#include <am/succinct/utils.hpp>
#include <util/mem_utils.h>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dense
{

DBitV::DBitV(bool support_select)
    : support_select_(support_select)
    , len_(), one_count_()
{
}

DBitV::~DBitV()
{
}

void DBitV::build(const std::vector<uint64_t> &bv, size_t len)
{
    size_t size = len / kSuperBlockSize + 1;
    super_blocks_.reset(cachealign_alloc<SuperBlock>(size), cachealign_deleter());
    memset(super_blocks_.get(), 0, size * sizeof(super_blocks_[0]));

    if (support_select_)
    {
        select_one_inds_.push_back(0);
        select_zero_inds_.push_back(0);
    }

    size_t index = len / kBlockSize;
    size_t offset = len % kBlockSize;
    size_t sb_index = 0;
    size_t sb_offset = 0;
    size_t sb_rank = 0;

    for (size_t i = 0; i < index; ++i)
    {
        buildBlock_(bv[i], kBlockSize, sb_index, sb_offset, sb_rank);

        if (++sb_offset == kBlockPerSuperBlock)
        {
            sb_offset = 0;
            sb_rank = 0;
            super_blocks_[++sb_index].rank_ = one_count_;
        }
    }

    if (offset)
    {
        buildBlock_(bv[index] & ((1ULL << offset) - 1), offset, sb_index, sb_offset, sb_rank);
    }
    else
    {
        super_blocks_[sb_index].subrank_ |= sb_rank << 9 * (sb_offset + 1);
    }

    if (support_select_)
    {
        select_one_inds_.push_back(len_ / kSuperBlockSize + 1);
        select_zero_inds_.push_back(len_ / kSuperBlockSize + 1);

        if (select_one_inds_.capacity() > select_one_inds_.size())
        {
            std::vector<size_t>(select_one_inds_).swap(select_one_inds_);
        }
        if (select_zero_inds_.capacity() > select_zero_inds_.size())
        {
            std::vector<size_t>(select_zero_inds_).swap(select_zero_inds_);
        }
    }
}

void DBitV::buildBlock_(uint64_t block, size_t bits, size_t sb_ind, size_t sb_off, size_t& rank_sb)
{
    size_t rank_b = SuccinctUtils::popcount(block);

    if (support_select_)
    {
        if (one_count_ % kSelectBlockSize + rank_b >= kSelectBlockSize)
        {
            select_one_inds_.push_back(len_ / kSuperBlockSize + 1);
        }
        if ((len_ - one_count_) % kSelectBlockSize + bits - rank_b >= kSelectBlockSize)
        {
            select_zero_inds_.push_back(len_ / kSuperBlockSize + 1);
        }
    }

    len_ += bits;
    one_count_ += rank_b;
    rank_sb += rank_b;

    super_blocks_[sb_ind].bits_[sb_off] = block;
    super_blocks_[sb_ind].subrank_ |= rank_sb << 9 * (sb_off + 1);
}

void DBitV::clear()
{
    len_ = 0;
    one_count_ = 0;

    super_blocks_.reset();

    if (support_select_)
    {
        std::vector<size_t>().swap(select_one_inds_);
        std::vector<size_t>().swap(select_zero_inds_);
    }
}

bool DBitV::access(size_t pos) const
{
    __assert(pos < len_);

    size_t index = pos / kSuperBlockSize;
    size_t offset = pos % kSuperBlockSize;

    return super_blocks_[index].bits_[offset / kBlockSize] & 1ULL << offset;
}

bool DBitV::access(size_t pos, size_t &r) const
{
    __assert(pos < len_);

    size_t index = pos / kSuperBlockSize;
    size_t offset = pos % kSuperBlockSize;
    size_t sb_index = offset / kBlockSize;
    size_t sb_mask = 1ULL << offset;

    const SuperBlock &sb = super_blocks_[index];

    if (sb.bits_[sb_index] & sb_mask)
    {
        r = sb.rank_ + ((sb.subrank_ >> 9 * sb_index) & 511) + SuccinctUtils::popcount(sb.bits_[sb_index] & (sb_mask - 1));
        return true;
    }
    else
    {
        r = pos - sb.rank_ - ((sb.subrank_ >> 9 * sb_index) & 511) - SuccinctUtils::popcount(sb.bits_[sb_index] & (sb_mask - 1));
        return false;
    }
}

size_t DBitV::rank0(size_t pos) const
{
    return pos - rank1(pos);
}

size_t DBitV::rank1(size_t pos) const
{
    __assert(pos <= len_);

    size_t index = pos / kSuperBlockSize;
    size_t offset = pos % kSuperBlockSize;
    size_t sb_index = offset / kBlockSize;

    const SuperBlock &sb = super_blocks_[index];

    return sb.rank_ + ((sb.subrank_ >> 9 * sb_index) & 511)
        + SuccinctUtils::popcount(sb.bits_[sb_index] & ((1ULL << pos) - 1));
}

size_t DBitV::rank(size_t pos, bool bit) const
{
    return bit ? rank1(pos) : rank0(pos);
}

size_t DBitV::select0(size_t ind) const
{
    __assert(support_select_ && ind < len_ - one_count_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t low = select_zero_inds_[select_ind];
    size_t high = select_zero_inds_[select_ind + 1];

    while (low + kSelectLinearFallback < high)
    {
        size_t mid = (low + high) / 2;
        asm("cmpq %3, %2\n\tcmovge %4, %0\n\tcmovl %5, %1"
                : "+r"(low), "+r"(high)
                : "r"(ind), "g"(mid * kSuperBlockSize - super_blocks_[mid].rank_), "g"(mid + 1), "g"(mid));
    }

    for (; low < high && low * kSuperBlockSize - super_blocks_[low].rank_ <= ind; ++low);

    __assert(low > 0);
    const SuperBlock &sb = super_blocks_[--low];
    ind -= low * kSuperBlockSize - sb.rank_;

    size_t sb_index = 1;
    uint64_t subrank = sb.subrank_;

    while (ind >= 64 * sb_index - ((subrank >> 9 * sb_index) & 511))
    {
        ++sb_index;
    }
    --sb_index;

    return low * kSuperBlockSize + sb_index * kBlockSize + SuccinctUtils::selectBlock(~sb.bits_[sb_index], ind + ((subrank >> 9 * sb_index) & 511) - 64 * sb_index);
}

size_t DBitV::select1(size_t ind) const
{
    __assert(support_select_ && ind < one_count_);

    size_t select_ind = ind / kSelectBlockSize;
    size_t low = select_one_inds_[select_ind];
    size_t high = select_one_inds_[select_ind + 1];

    while (low + kSelectLinearFallback < high)
    {
        size_t mid = (low + high) / 2;
        asm("cmpq %3, %2\n\tcmovge %4, %0\n\tcmovl %5, %1"
                : "+r"(low), "+r"(high)
                : "r"(ind), "g"(super_blocks_[mid].rank_), "g"(mid + 1), "g"(mid));
    }

    for (; low < high && super_blocks_[low].rank_ <= ind; ++low);

    __assert(low > 0);
    const SuperBlock &sb = super_blocks_[--low];
    ind -= sb.rank_;

    size_t sb_index = 1;
    uint64_t subrank = sb.subrank_;

    while (ind >= ((subrank >> 9 * sb_index) & 511))
    {
        ++sb_index;
    }
    --sb_index;

    return low * kSuperBlockSize + sb_index * kBlockSize + SuccinctUtils::selectBlock(sb.bits_[sb_index], ind - ((subrank >> 9 * sb_index) & 511));
}

size_t DBitV::select(size_t ind, bool bit) const
{
    return bit ? select1(ind) : select0(ind);
}

void DBitV::save(std::ostream &os) const
{
    os.write((const char *)&support_select_, sizeof(support_select_));
    os.write((const char *)&len_, sizeof(len_));
    os.write((const char *)&one_count_, sizeof(one_count_));

    size_t size = len_ / kSuperBlockSize + 1;
    os.write((const char *)&super_blocks_[0], size * sizeof(super_blocks_[0]));

    if (support_select_)
    {
        SuccinctUtils::saveVec(os, select_one_inds_);
        SuccinctUtils::saveVec(os, select_zero_inds_);
    }
}

void DBitV::load(std::istream &is)
{
    is.read((char *)&support_select_, sizeof(support_select_));
    is.read((char *)&len_, sizeof(len_));
    is.read((char *)&one_count_, sizeof(one_count_));

    size_t size = len_ / kSuperBlockSize + 1;
    super_blocks_.reset(cachealign_alloc<SuperBlock>(size), cachealign_deleter());
    memset(super_blocks_.get(), 0, size * sizeof(super_blocks_[0]));
    is.read((char *)&super_blocks_[0], size * sizeof(super_blocks_[0]));

    if (support_select_)
    {
        SuccinctUtils::loadVec(is, select_one_inds_);
        SuccinctUtils::loadVec(is, select_zero_inds_);
    }
}

size_t DBitV::allocSize() const
{
    return sizeof(DBitV)
        + sizeof(super_blocks_[0]) * (len_ / kSuperBlockSize + 1)
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size();
}

}
}

NS_IZENELIB_AM_END
