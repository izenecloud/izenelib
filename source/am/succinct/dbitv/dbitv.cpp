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
    bits_.reserve(len / kSmallBlockSize + 1);
    rank_blocks_.reserve(len / kHugeBlockSize + 1);
    rank_small_blocks_.reserve(len / kSmallBlockSize + 1);

    rank_blocks_.push_back(0);

    if (support_select_)
    {
        select_one_inds_.push_back(0);
        select_zero_inds_.push_back(0);
    }

    size_t index = len / kSmallBlockSize;
    size_t offset = len % kSmallBlockSize;
    size_t rank_lb = 0;

    for (size_t i = 0; i < index; ++i)
    {
        buildBlock_(bv[i], kSmallBlockSize, rank_lb);

        if ((i + 1) % kSmallBlockPerHugeBlock == 0)
        {
            rank_blocks_.push_back(one_count_);
            rank_lb = 0;
        }
    }

    buildBlock_(offset ? bv[index] & ((1LLU << offset) - 1) : 0, offset, rank_lb);

    if (support_select_)
    {
        select_one_inds_.push_back(rank_small_blocks_.size());
        select_zero_inds_.push_back(rank_small_blocks_.size());

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

void DBitV::buildBlock_(uint64_t block, size_t offset, size_t &rank_lb)
{
    size_t rank_sb = SuccinctUtils::popcount64(block);

    bits_.push_back(block);
    rank_small_blocks_.push_back(rank_lb);

    if (support_select_)
    {
        if (rank_sb > 0 && one_count_ % kSelectBlockSize + rank_sb >= kSelectBlockSize)
        {
            size_t remain = SuccinctUtils::selectBlock1(block, kSelectBlockSize - one_count_ % kSelectBlockSize);
            select_one_inds_.push_back((len_ + remain) / kSmallBlockSize);
        }
        if (offset - rank_sb > 0 && (len_ - one_count_) % kSelectBlockSize + offset - rank_sb >= kSelectBlockSize)
        {
            size_t remain = SuccinctUtils::selectBlock0(block, kSelectBlockSize - (len_ - one_count_) % kSelectBlockSize);
            select_zero_inds_.push_back((len_ + remain) / kSmallBlockSize);
        }
    }

    len_ += offset;
    one_count_ += rank_sb;

    rank_lb += rank_sb;
}

size_t DBitV::rankOutOfSmallBlock_(size_t sblock) const
{
    return rank_blocks_[sblock / kSmallBlockPerHugeBlock] + rank_small_blocks_[sblock];
}

void DBitV::clear()
{
    len_ = 0;
    one_count_ = 0;

    std::vector<uint64_t>().swap(bits_);
    std::vector<size_t>().swap(rank_blocks_);
    std::vector<uint16_t>().swap(rank_small_blocks_);
}

bool DBitV::lookup(size_t pos) const
{
    __assert(pos < len_);
    return bits_[pos / kSmallBlockSize] & (uint64_t(1) << (pos % kSmallBlockSize));
}

bool DBitV::lookup(size_t pos, size_t &r) const
{
    bool bit = lookup(pos);
    r = rank(pos, bit);
    return bit;
}

size_t DBitV::rank0(size_t pos) const
{
    return pos - rank1(pos);
}

size_t DBitV::rank1(size_t pos) const
{
    __assert(pos <= len_);

    size_t sblock = pos / kSmallBlockSize;

    return rankOutOfSmallBlock_(sblock) + SuccinctUtils::popcount64(bits_[sblock] & ((uint64_t(1) << (pos % kSmallBlockSize)) - 1));
}

size_t DBitV::rank(size_t pos, bool bit) const
{
    if (bit) return rank1(pos);
    else return rank0(pos);
}

size_t DBitV::select0(size_t ind) const
{
    if (!support_select_ || ind >= len_ - one_count_) return -1;

    size_t select_ind = ind / kSelectBlockSize;
    size_t left = select_zero_inds_[select_ind];
    size_t right = select_zero_inds_[select_ind + 1];

    while (left < right)
    {
        size_t mid = (left + right) / 2;
        if (mid * kSmallBlockSize - rankOutOfSmallBlock_(mid) < ind)
        {
            left = mid + 1;
        }
        else
        {
            right = mid;
        }
    }

    if (left > 0) --left;

    return left * kSmallBlockSize + SuccinctUtils::selectBlock0(bits_[left], ind - left * kSmallBlockSize + rankOutOfSmallBlock_(left));
}

size_t DBitV::select1(size_t ind) const
{
    if (!support_select_ || ind >= one_count_) return -1;

    size_t select_ind = ind / kSelectBlockSize;
    size_t left = select_one_inds_[select_ind];
    size_t right = select_one_inds_[select_ind + 1];

    while (left < right)
    {
        size_t mid = (left + right) / 2;
        if (rankOutOfSmallBlock_(mid) < ind)
        {
            left = mid + 1;
        }
        else
        {
            right = mid;
        }
    }

    if (left > 0) --left;

    return left * kSmallBlockSize + SuccinctUtils::selectBlock1(bits_[left], ind - rankOutOfSmallBlock_(left));
}

size_t DBitV::select(size_t ind, bool bit) const
{
    if (bit) return select1(ind);
    else return select0(ind);
}

void DBitV::save(std::ostream &os) const
{
    os.write((const char *)&support_select_, sizeof(support_select_));
    os.write((const char *)&len_, sizeof(len_));
    os.write((const char *)&one_count_, sizeof(one_count_));

    save(os, bits_);
    save(os, rank_blocks_);
    save(os, rank_small_blocks_);

    if (support_select_)
    {
        save(os, select_one_inds_);
        save(os, select_zero_inds_);
    }
}

void DBitV::load(std::istream &is)
{
    is.read((char *)&support_select_, sizeof(support_select_));
    is.read((char *)&len_, sizeof(len_));
    is.read((char *)&one_count_, sizeof(one_count_));

    load(is, bits_);
    load(is, rank_blocks_);
    load(is, rank_small_blocks_);

    if (support_select_)
    {
        load(is, select_one_inds_);
        load(is, select_zero_inds_);
    }
}

size_t DBitV::allocSize() const
{
    return sizeof(DBitV)
        + sizeof(bits_[0]) * bits_.size()
        + sizeof(rank_blocks_[0]) * rank_blocks_.size()
        + sizeof(rank_small_blocks_[0]) * rank_small_blocks_.size()
        + sizeof(select_one_inds_[0]) * select_one_inds_.size()
        + sizeof(select_zero_inds_[0]) * select_zero_inds_.size();
}

}
}

NS_IZENELIB_AM_END
