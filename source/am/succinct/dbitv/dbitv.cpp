#include <am/succinct/dbitv/dbitv.hpp>
#include <am/succinct/utils.hpp>

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
    super_blocks_.resize(len / kSuperBlockSize + 1);

    if (support_select_)
    {
        select_one_inds_.push_back(0);
        select_zero_inds_.push_back(0);
    }

    size_t index = len / kBlockSize;
    size_t offset = len % kBlockSize;
    size_t sb_index = 0;
    size_t sb_offset = 0;

    for (size_t i = 0; i < index; ++i)
    {
        super_blocks_[sb_index].bits_[sb_offset] = bv[i];
        buildBlock_(bv[i], kBlockSize);
        len_ += kBlockSize;

        if (++sb_offset == kBlockPerSuperBlock)
        {
            sb_offset = 0;
            super_blocks_[++sb_index].rank_ = one_count_;
        }
    }

    if (offset)
    {
        size_t block = bv[index] & ((1LLU << offset) - 1);
        super_blocks_[sb_index].bits_[sb_offset] = block;
        buildBlock_(block, offset);
        len_ += offset;
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

void DBitV::buildBlock_(uint64_t block, size_t offset)
{
    size_t rank_sb = SuccinctUtils::popcount(block);

    if (support_select_)
    {
        if (one_count_ % kSelectBlockSize + rank_sb >= kSelectBlockSize)
        {
            select_one_inds_.push_back(len_ / kSuperBlockSize + 1);
        }
        if ((len_ - one_count_) % kSelectBlockSize + offset - rank_sb >= kSelectBlockSize)
        {
            select_zero_inds_.push_back(len_ / kSuperBlockSize + 1);
        }
    }

    one_count_ += rank_sb;
}

void DBitV::clear()
{
    len_ = 0;
    one_count_ = 0;

    std::vector<SuperBlock>().swap(super_blocks_);

    if (support_select_)
    {
        std::vector<size_t>().swap(select_one_inds_);
        std::vector<size_t>().swap(select_zero_inds_);
    }
}

bool DBitV::lookup(size_t pos) const
{
    __assert(pos < len_);
    return super_blocks_[pos / kSuperBlockSize].bits_[pos % kSuperBlockSize / kBlockSize] & (uint64_t(1) << (pos % kBlockSize));
}

bool DBitV::lookup(size_t pos, size_t &r) const
{
    __assert(pos < len_);

    const SuperBlock &sb = super_blocks_[pos / kSuperBlockSize];
    size_t sb_offset = pos % kSuperBlockSize / kBlockSize;
    size_t b_offset = pos % kBlockSize;
    bool bit = sb.bits_[sb_offset] & (uint64_t(1) << b_offset);

    r = sb.rank_ + SuccinctUtils::popcount(sb.bits_[sb_offset] & ((uint64_t(1) << b_offset) - 1));
    for (size_t i = 0; i < sb_offset; ++i)
    {
        r += SuccinctUtils::popcount(sb.bits_[i]);
    }

    if (!bit)
    {
        r = pos - r;
    }

    return bit;
}

size_t DBitV::rank0(size_t pos) const
{
    return pos - rank1(pos);
}

size_t DBitV::rank1(size_t pos) const
{
    __assert(pos <= len_);

    const SuperBlock &sb = super_blocks_[pos / kSuperBlockSize];
    size_t sb_offset = pos % kSuperBlockSize / kBlockSize;

    size_t r = sb.rank_ + SuccinctUtils::popcount(sb.bits_[sb_offset] & ((uint64_t(1) << (pos % kBlockSize)) - 1));
    for (size_t i = 0; i < sb_offset; ++i)
    {
        r += SuccinctUtils::popcount(sb.bits_[i]);
    }

    return r;
}

size_t DBitV::rank(size_t pos, bool bit) const
{
    if (bit) return rank1(pos);
    else return pos - rank1(pos);
}

size_t DBitV::select0(size_t rank) const
{
    __assert(support_select_ && rank < len_ - one_count_);

    size_t select_ind = rank / kSelectBlockSize;
    size_t low = select_zero_inds_[select_ind];
    size_t high = select_zero_inds_[select_ind + 1];

    while (low < high)
    {
        size_t mid = (low + high) / 2;
        if (mid * kSuperBlockSize - super_blocks_[mid].rank_ <= rank)
        {
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }

    __assert(low > 0);
    const SuperBlock &sb = super_blocks_[--low];
    rank -= low * kSuperBlockSize - sb.rank_;

    size_t sb_offset = 0;
    size_t b_rank = SuccinctUtils::popcount(~sb.bits_[sb_offset]);
    while (rank >= b_rank)
    {
        rank -= b_rank;
        b_rank = SuccinctUtils::popcount(~sb.bits_[++sb_offset]);
    }

    return low * kSuperBlockSize + sb_offset * kBlockSize + SuccinctUtils::selectBlock(~sb.bits_[sb_offset], rank);
}

size_t DBitV::select1(size_t rank) const
{
    __assert(support_select_ && rank < one_count_);

    size_t select_ind = rank / kSelectBlockSize;
    size_t low = select_one_inds_[select_ind];
    size_t high = select_one_inds_[select_ind + 1];

    while (low < high)
    {
        size_t mid = (low + high) / 2;
        if (super_blocks_[mid].rank_ <= rank)
        {
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }

    __assert(low > 0);
    const SuperBlock &sb = super_blocks_[--low];
    rank -= sb.rank_;

    size_t sb_offset = 0;
    size_t b_rank = SuccinctUtils::popcount(sb.bits_[sb_offset]);
    while (rank >= b_rank)
    {
        rank -= b_rank;
        b_rank = SuccinctUtils::popcount(sb.bits_[++sb_offset]);
    }

    return low * kSuperBlockSize + sb_offset * kBlockSize + SuccinctUtils::selectBlock(sb.bits_[sb_offset], rank);
}

size_t DBitV::select(size_t rank, bool bit) const
{
    if (bit) return select1(rank);
    else return select0(rank);
}

void DBitV::save(std::ostream &os) const
{
    os.write((const char *)&support_select_, sizeof(support_select_));
    os.write((const char *)&len_, sizeof(len_));
    os.write((const char *)&one_count_, sizeof(one_count_));

    SuccinctUtils::saveVec(os, super_blocks_);

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

    SuccinctUtils::loadVec(is, super_blocks_);

    if (support_select_)
    {
        SuccinctUtils::loadVec(is, select_one_inds_);
        SuccinctUtils::loadVec(is, select_zero_inds_);
    }
}

size_t DBitV::allocSize() const
{
    return sizeof(DBitV)
        + sizeof(super_blocks_[0]) * super_blocks_.size()
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size();
}

}
}

NS_IZENELIB_AM_END
