/*
 *  Copyright (c) 2010 Daisuke Okanohara
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <am/succinct/wat_array/wat_array.hpp>

#include <queue>
#include <algorithm>
#include <stack>

using namespace std;

namespace wat_array
{

WatArray::WatArray() : alphabet_num_(0), alphabet_bit_num_(0), length_(0)
{
}

WatArray::~WatArray()
{
}

void WatArray::Clear()
{
    vector<BitArray>().swap(bit_arrays_);
    occs_.Clear();
    alphabet_num_ = 0;
    alphabet_bit_num_ = 0;
    length_ = 0;
}

void WatArray::Init(const vector<uint64_t>& array)
{
    Clear();
    alphabet_num_     = GetAlphabetNum(array);
    alphabet_bit_num_ = Log2(alphabet_num_);

    length_           = static_cast<uint64_t>(array.size());
    SetArray(array);
    SetOccs(array);
}

uint64_t WatArray::Lookup(uint64_t pos) const
{
    if (pos >= length_) return NOTFOUND;
    uint64_t st = 0;
    uint64_t en = length_;
    uint64_t c = 0;
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        const BitArray& ba = bit_arrays_[i];
        uint64_t boundary  = st + ba.Rank(0, en) - ba.Rank(0, st);
        uint64_t bit       = ba.Lookup(st + pos);
        c <<= 1;
        if (bit)
        {
            pos = ba.Rank(1, st + pos) - ba.Rank(1, st);
            st = boundary;
            c |= 1LLU;
        }
        else
        {
            pos = ba.Rank(0, st + pos) - ba.Rank(0, st);
            en = boundary;
        }

    }
    return c;
}

uint64_t WatArray::Rank(uint64_t c, uint64_t pos) const
{
    uint64_t rank_less_than = 0;
    uint64_t rank_more_than = 0;
    uint64_t rank           = 0;
    RankAll(c, pos, rank, rank_less_than, rank_more_than);
    return rank;
}

uint64_t WatArray::RankLessThan(uint64_t c, uint64_t pos) const
{
    uint64_t rank_less_than = 0;
    uint64_t rank_more_than = 0;
    uint64_t rank           = 0;
    RankAll(c, pos, rank, rank_less_than, rank_more_than);
    return rank_less_than;
}

uint64_t WatArray::RankMoreThan(uint64_t c, uint64_t pos) const
{
    uint64_t rank_less_than = 0;
    uint64_t rank_more_than = 0;
    uint64_t rank           = 0;
    RankAll(c, pos, rank, rank_less_than, rank_more_than);
    return rank_more_than;
}

void WatArray::RankAll(uint64_t c, uint64_t pos,
                       uint64_t& rank,  uint64_t& rank_less_than, uint64_t& rank_more_than) const
{
    if (c >= alphabet_num_)
    {
        rank_less_than = NOTFOUND;
        rank_more_than = NOTFOUND;
        rank           = NOTFOUND;
        return;
    }
    if (pos > length_)
    {
        pos = length_;
    }
    uint64_t beg_node = 0;
    uint64_t end_node = length_;
    rank_less_than = 0;
    rank_more_than = 0;

    for (size_t i = 0; i < bit_arrays_.size() && beg_node < end_node; ++i)
    {
        const BitArray& ba = bit_arrays_[i];
        uint64_t beg_node_zero = ba.Rank(0, beg_node);
        uint64_t beg_node_one  = beg_node - beg_node_zero;
        uint64_t end_node_zero = ba.Rank(0, end_node);
        uint64_t boundary      = beg_node + end_node_zero - beg_node_zero;
        uint64_t bit           = GetMSB(c, i, bit_arrays_.size());
        if (!bit)
        {
            rank_more_than += ba.Rank(1, pos) - beg_node_one;
            pos      = beg_node + ba.Rank(0, pos) - beg_node_zero;
            end_node = boundary;
        }
        else
        {
            rank_less_than += ba.Rank(0, pos) - beg_node_zero;
            pos      = boundary + ba.Rank(1, pos) - (beg_node - beg_node_zero);
            beg_node = boundary;
        }
    }
    rank = pos - beg_node;
}

uint64_t WatArray::Select(uint64_t c, uint64_t rank) const
{
    if (c >= alphabet_num_)
    {
        return NOTFOUND;
    }
    if (rank >= Freq(c))
    {
        return NOTFOUND;
    }

    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        uint64_t lower_c = c & ~((1LLU << (i + 1)) - 1);
        uint64_t beg_node = occs_.Select(1, lower_c  + 1) - lower_c;
        const BitArray& ba = bit_arrays_[alphabet_bit_num_ - i - 1];
        uint64_t bit = GetLSB(c, i);
        uint64_t before_rank = ba.Rank(bit, beg_node);
        rank = ba.Select(bit, before_rank + rank) - beg_node + 1;
    }
    return rank - 1;
}

uint64_t WatArray::FreqRange(uint64_t min_c, uint64_t max_c, uint64_t begin_pos, uint64_t end_pos) const
{
    if (min_c >= alphabet_num_) return 0;
    if (max_c <= min_c) return 0;
    if (end_pos > length_ || begin_pos > end_pos) return 0;
    return
        + RankLessThan(max_c, end_pos)
        - RankLessThan(min_c, end_pos)
        - RankLessThan(max_c, begin_pos)
        + RankLessThan(min_c, begin_pos);
}

void WatArray::MaxRange(uint64_t begin_pos, uint64_t end_pos, uint64_t& pos, uint64_t& val) const
{
    QuantileRange(begin_pos, end_pos, end_pos - begin_pos - 1, pos, val);
}

void WatArray::MinRange(uint64_t begin_pos, uint64_t end_pos, uint64_t& pos, uint64_t& val) const
{
    QuantileRange(begin_pos, end_pos, 0,  pos, val);
}

void WatArray::QuantileRange(uint64_t begin_pos, uint64_t end_pos, uint64_t k, uint64_t& pos, uint64_t& val) const
{
    if (end_pos > length_ || begin_pos >= end_pos)
    {
        pos = NOTFOUND;
        val = NOTFOUND;
        return;
    }

    val = 0;
    uint64_t beg_node = 0;
    uint64_t end_node = length_;
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        const BitArray& ba = bit_arrays_[i];
        uint64_t beg_node_zero = ba.Rank(0, beg_node);
        uint64_t end_node_zero = ba.Rank(0, end_node);
        uint64_t beg_node_one  = beg_node - beg_node_zero;
        uint64_t beg_zero  = ba.Rank(0, begin_pos);
        uint64_t end_zero  = ba.Rank(0, end_pos);
        uint64_t beg_one   = begin_pos - beg_zero;
        uint64_t end_one   = end_pos - end_zero;
        uint64_t boundary  = beg_node + end_node_zero - beg_node_zero;

        if (end_zero - beg_zero > k)
        {
            end_node = boundary;
            begin_pos = beg_node + beg_zero - beg_node_zero;
            end_pos   = beg_node + end_zero - beg_node_zero;
            val       = val << 1;
        }
        else
        {
            beg_node  = boundary;
            begin_pos = boundary + beg_one - beg_node_one;
            end_pos   = boundary + end_one - beg_node_one;
            val       = (val << 1) + 1;
            k -= end_zero - beg_zero;
        }
    }

    uint64_t rank = begin_pos - beg_node;
    pos = Select(val, rank + 1);
}
void WatArray::QuantileRangeAll(uint64_t begin_pos, uint64_t end_pos, vector<uint64_t>& ret) const
{
    uint64_t val;
    if (end_pos > length_ || begin_pos >= end_pos)
    {
        //pos = NOTFOUND;
        //val = NOTFOUND;
        return;
    }

    val = 0;
    uint64_t beg_node = 0;
    uint64_t end_node = length_;
    size_t i = 0;
    //uint64_t k=0;
    QuantileRangeEach(begin_pos, end_pos, i,beg_node ,end_node, val,ret);
    //QuantileRangeEach_NonRecursive(begin_pos, end_pos, i,beg_node, end_node, val, ret);


}
void WatArray::QuantileRangeEach(uint64_t begin_pos, uint64_t end_pos, size_t i,uint64_t beg_node ,uint64_t end_node, uint64_t val,vector<uint64_t>& ret) const
{
    //cout<<"QuantileRangeEach "<<" begin_pos: "<<begin_pos<<" end_pos: "<<end_pos<<" i: "<<i<<" beg_node: "<<beg_node<<" end_node: "<<end_node<<" val: "<<val<<" ret: "<<ret.size()<<endl;
    if(i==bit_arrays_.size())
    {
        ret.push_back(val);
    }
    else
    {
        const BitArray& ba = bit_arrays_[i];
        uint64_t beg_node_zero = ba.Rank(0, beg_node);
        uint64_t end_node_zero = ba.Rank(0, end_node);
        uint64_t beg_node_one  = beg_node - beg_node_zero;
        uint64_t beg_zero  = ba.Rank(0, begin_pos);
        uint64_t end_zero  = ba.Rank(0, end_pos);
        uint64_t beg_one   = begin_pos - beg_zero;
        uint64_t end_one   = end_pos - end_zero;
        uint64_t boundary  = beg_node + end_node_zero - beg_node_zero;


           // end_node = boundary;

           // val       = val << 1;

        if (end_zero - beg_zero >0)
        {
            begin_pos = beg_node + beg_zero - beg_node_zero;
            end_pos   = beg_node + end_zero - beg_node_zero;
            QuantileRangeEach(begin_pos, end_pos, i+1,beg_node ,boundary, (val<<1),ret);
        }
        else
        {
             if (end_zero - beg_zero ==0)
             {
                ret.push_back(val);
             }
        }
        if (end_one - beg_one >0)
        {
            begin_pos = boundary + beg_one - beg_node_one;
            end_pos   = boundary + end_one - beg_node_one;
            QuantileRangeEach(begin_pos, end_pos, i+1,boundary ,end_node, (val << 1) + 1,ret);
        }
        else
        {
             if (end_one - beg_one ==0)
             {
                ret.push_back(val);
             }
        }

    }


}

void WatArray::QuantileRangeEach_NonRecursive(uint64_t begin_pos, uint64_t end_pos, size_t i,uint64_t beg_node ,uint64_t end_node, uint64_t val,vector<uint64_t>& ret) const
{
    //cout<<"QuantileRangeEach"<<"begin_pos"<<begin_pos<<"end_pos"<<end_pos<<"i"<<i<<"beg_node"<<beg_node<<"end_node"<<end_node<<"val"<<val<<"ret"<<ret.size()<<endl;
    stack<Parameter> Params;
    Params.push(Parameter(begin_pos, end_pos, i, beg_node, end_node, val));

    while(!Params.empty())
    {
        Parameter &MyParam = Params.top();
        //cout<<"QuantileRangeEach "<<" begin_pos: "<<MyParam.begin_pos<<" end_pos: "<<MyParam.end_pos<<" i: "<<MyParam.i<<" beg_node: "<<MyParam.beg_node<<" end_node: "<<MyParam.end_node<<" val: "<<MyParam.val<<" ret: "<<ret.size()<<endl;

        uint64_t param_begin_pos = MyParam.begin_pos;
        uint64_t param_end_pos = MyParam.end_pos;
        uint64_t param_val = MyParam.val;
        size_t param_i = MyParam.i;
        uint64_t param_beg_node = MyParam.beg_node;
        uint64_t param_end_node = MyParam.end_node;
        Params.pop();

        if(param_i==bit_arrays_.size())
        {
            ret.push_back(param_val);
        }
        else
        {
            const BitArray& ba = bit_arrays_[param_i];
            uint64_t beg_node_zero = ba.Rank(0, param_beg_node);
            uint64_t end_node_zero = ba.Rank(0, param_end_node);
            uint64_t beg_node_one  = param_beg_node - beg_node_zero;
            uint64_t beg_zero  = ba.Rank(0, param_begin_pos);
            uint64_t end_zero  = ba.Rank(0, param_end_pos);
            uint64_t beg_one   = param_begin_pos - beg_zero;
            uint64_t end_one   = param_end_pos - end_zero;
            uint64_t boundary  = param_beg_node + end_node_zero - beg_node_zero;

            if (end_zero - beg_zero >0)
            {
                Params.push(Parameter(param_beg_node + beg_zero - beg_node_zero,
                            param_beg_node + end_zero - beg_node_zero,
                            param_i+1,
                            param_beg_node,
                            boundary,
                            (param_val<<1)
                            ));
            }
            else if (end_zero - beg_zero ==0)
            {
                ret.push_back(param_val);
            }

            if (end_one - beg_one >0)
            {
                Params.push(Parameter(boundary + beg_one - beg_node_one,
                            boundary + end_one - beg_node_one,
                            param_i+1,
                            boundary,
                            param_end_node,
                            (param_val << 1) + 1
                            ));
            }
            else if (end_one - beg_one ==0)
            {
                ret.push_back(param_val);
            }

        }

    }

}



class WatArray::ListModeComparator
{
public:
    ListModeComparator()
    {
    }

    bool operator() (const QueryOnNode& lhs,
                     const QueryOnNode& rhs) const
    {
        if (lhs.end_pos - lhs.beg_pos != rhs.end_pos - rhs.beg_pos)
        {
            return lhs.end_pos - lhs.beg_pos < rhs.end_pos - rhs.beg_pos;
        }
        else if (lhs.depth != rhs.depth)
        {
            return lhs.depth < rhs.depth;
        }
        else
        {
            return lhs.beg_pos > rhs.beg_pos;
        }
    }
};

class WatArray::ListMinComparator
{
public:
    ListMinComparator()
    {
    }

    bool operator() (const QueryOnNode& lhs,
                     const QueryOnNode& rhs) const
    {
        if (lhs.depth != rhs.depth)
            return lhs.depth < rhs.depth;
        else return lhs.beg_node > rhs.beg_node;
    }
};

class WatArray::ListMaxComparator
{
public:
    ListMaxComparator()
    {
    }

    bool operator() (const QueryOnNode& lhs,
                     const QueryOnNode& rhs) const
    {
        if (lhs.depth != rhs.depth)
            return lhs.depth < rhs.depth;
        else return lhs.beg_node < rhs.beg_node;
    }
};

void WatArray::ListModeRange(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos,
                             uint64_t num, vector<ListResult>& res) const
{
    ListRange<ListModeComparator>(min_c, max_c, beg_pos, end_pos, num, res);
}

void WatArray::ListMinRange(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos,
                            uint64_t num, vector<ListResult>& res) const
{
    ListRange<ListMinComparator>(min_c, max_c, beg_pos, end_pos, num, res);
}

void WatArray::ListMaxRange(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos,
                            uint64_t num, vector<ListResult>& res) const
{
    ListRange<ListMaxComparator>(min_c, max_c, beg_pos, end_pos, num, res);
}

bool WatArray::CheckPrefix(uint64_t prefix, uint64_t depth, uint64_t min_c, uint64_t max_c) const
{
    if (PrefixCode(min_c,   depth, alphabet_bit_num_) <= prefix &&
            PrefixCode(max_c - 1, depth, alphabet_bit_num_) >= prefix) return true;
    else return false;
}

void WatArray::ExpandNode(uint64_t min_c, uint64_t max_c,
                          const QueryOnNode& qon, vector<QueryOnNode>& next) const
{
    const BitArray& ba = bit_arrays_[qon.depth];

    uint64_t beg_node_zero = ba.Rank(0, qon.beg_node);
    uint64_t end_node_zero = ba.Rank(0, qon.end_node);
    uint64_t beg_node_one  = qon.beg_node - beg_node_zero;
    uint64_t beg_zero  = ba.Rank(0, qon.beg_pos);
    uint64_t end_zero  = ba.Rank(0, qon.end_pos);
    uint64_t beg_one   = qon.beg_pos - beg_zero;
    uint64_t end_one   = qon.end_pos - end_zero;
    uint64_t boundary  = qon.beg_node + end_node_zero - beg_node_zero;
    if (end_zero - beg_zero > 0)  // child for zero
    {
        uint64_t next_prefix = qon.prefix_char << 1;
        if (CheckPrefix(next_prefix, qon.depth + 1, min_c, max_c))
        {
            next.push_back(QueryOnNode(qon.beg_node,
                                       boundary,
                                       qon.beg_node + beg_zero - beg_node_zero,
                                       qon.beg_node + end_zero - beg_node_zero,
                                       qon.depth + 1,
                                       next_prefix));
        }
    }
    if (end_one - beg_one > 0)  // child for one
    {
        uint64_t next_prefix = (qon.prefix_char << 1) + 1;
        if (CheckPrefix(next_prefix, qon.depth + 1, min_c, max_c))
        {
            next.push_back(QueryOnNode(boundary,
                                       qon.end_node,
                                       boundary + beg_one - beg_node_one,
                                       boundary + end_one - beg_node_one,
                                       qon.depth + 1,
                                       next_prefix));
        }
    }
}

uint64_t WatArray::Freq(uint64_t c) const
{
    if (c >= alphabet_num_) return NOTFOUND;
    return occs_.Select(1, c + 2) - occs_.Select(1, c + 1) - 1;
}

uint64_t WatArray::FreqSum(uint64_t min_c, uint64_t max_c) const
{
    if (max_c > alphabet_num_ || min_c > max_c ) return NOTFOUND;
    return occs_.Select(1, max_c + 1) - occs_.Select(1, min_c + 1) - (max_c - min_c);
}

uint64_t WatArray::alphabet_num() const
{
    return alphabet_num_;
}

uint64_t WatArray::length() const
{
    return length_;
}

uint64_t WatArray::GetAlphabetNum(const std::vector<uint64_t>& array) const
{
    uint64_t alphabet_num = 0;
    for (size_t i = 0; i < array.size(); ++i)
    {
        if (array[i] >= alphabet_num)
        {
            alphabet_num = array[i] + 1;
        }
    }
    return alphabet_num;
}

uint64_t WatArray::Log2(uint64_t x) const
{
    if (x == 0) return 0;
    --x;
    uint64_t bit_num = 0;
    while (x >> bit_num)
    {
        ++bit_num;
    }
    return bit_num;
}

uint64_t WatArray::PrefixCode(uint64_t x, uint64_t len, uint64_t bit_num) const
{
    return x >> (bit_num - len);
}

uint64_t WatArray::GetMSB(uint64_t x, uint64_t pos, uint64_t len)
{
    return (x >> (len - (pos + 1))) & 1LLU;
}

uint64_t WatArray::GetLSB(uint64_t x, uint64_t pos)
{
    return (x >> pos) & 1LLU;
}

void WatArray::SetArray(const vector<uint64_t>& array)
{
    if (alphabet_num_ == 0) return;
    bit_arrays_.resize(alphabet_bit_num_, length_);

    vector<vector<uint64_t> > beg_poses;
    GetBegPoses(array, alphabet_bit_num_, beg_poses);

    for (size_t i = 0; i < array.size(); ++i)
    {
        uint64_t c = array[i];
        for (uint64_t j = 0; j < alphabet_bit_num_; ++j)
        {
            bit_arrays_[j].SetBit(GetMSB(c, j, alphabet_bit_num_),
                                  beg_poses[j][PrefixCode(c, j, alphabet_bit_num_)]++);
        }
    }

    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        bit_arrays_[i].Build();
    }
}

void WatArray::SetOccs(const vector<uint64_t>& array)
{
    vector<uint64_t> counts(alphabet_num_);
    for (size_t i = 0; i < array.size(); ++i)
    {
        ++counts[array[i]];
    }

    occs_.Init(array.size() + alphabet_num_ + 1);
    uint64_t sum = 0;
    for (size_t i = 0; i < counts.size(); ++i)
    {
        occs_.SetBit(1, sum);
        sum += counts[i] + 1;
    }
    occs_.SetBit(1, sum);
    occs_.Build();
}

void WatArray::GetBegPoses(const vector<uint64_t>& array,
                           uint64_t alpha_bit_num,
                           vector< vector<uint64_t> >& beg_poses) const
{
    beg_poses.resize(alpha_bit_num);
    for (uint64_t i = 0; i < beg_poses.size(); ++i)
    {
        beg_poses[i].resize(1 << i);
    }

    for (size_t i = 0; i < array.size(); ++i)
    {
        uint64_t c = array[i];
        for (uint64_t j = 0; j < alpha_bit_num; ++j)
        {
            ++beg_poses[j][PrefixCode(c, j, alpha_bit_num)];
        }
    }

    for (size_t i = 0; i < beg_poses.size(); ++i)
    {
        uint64_t sum = 0;
        vector<uint64_t>& beg_poses_level = beg_poses[i];
        for (size_t j = 0; j < beg_poses_level.size(); ++j)
        {
            uint64_t num = beg_poses_level[j];
            beg_poses_level[j] = sum;
            sum += num;
        }
    }
}

void WatArray::Save(ostream& os) const
{
    os.write((const char*)(&alphabet_num_), sizeof(alphabet_num_));
    os.write((const char*)(&length_), sizeof(length_));
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        bit_arrays_[i].Save(os);
    }
    occs_.Save(os);
}

void WatArray::Load(istream& is)
{
    Clear();
    is.read((char*)(&alphabet_num_), sizeof(alphabet_num_));
    alphabet_bit_num_ = Log2(alphabet_num_);
    is.read((char*)(&length_), sizeof(length_));

    bit_arrays_.resize(alphabet_bit_num_);
    for (size_t i = 0; i < bit_arrays_.size(); ++i)
    {
        bit_arrays_[i].Load(is);
        bit_arrays_[i].Build();
    }
    occs_.Load(is);
    occs_.Build();
}

}
