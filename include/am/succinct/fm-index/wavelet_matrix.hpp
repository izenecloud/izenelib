#ifndef _FM_INDEX_WAVELET_MATRIX_HPP
#define _FM_INDEX_WAVELET_MATRIX_HPP

#include "wavelet_tree.hpp"
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/interval_heap.hpp>

#include <deque>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class WaveletMatrix : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;
    typedef WaveletMatrix<CharT> self_type;
    typedef std::vector<FilterList<self_type> *, boost::stl_allocator<FilterList<self_type> *> > aux_filter_list_type;

    WaveletMatrix(size_t alphabet_num, bool support_select, bool dense);
    ~WaveletMatrix();

    void build(const char_type *char_seq, size_t len);

    char_type access(size_t pos) const;
    char_type access(size_t pos, size_t &rank) const;

    size_t rank(char_type c, size_t pos) const;
    size_t select(char_type c, size_t rank) const;

    void intersect(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &results) const;

    void topKUnion(
            const range_list_type &patterns,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    void topKUnionWithFilters(
            const range_list_type &filters,
            const range_list_type &patterns,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    void topKUnionWithAuxFilters(
            const aux_filter_list_type &aux_filters,
            const range_list_type &patterns,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    void topKUnion(
            const range_list_type &patterns,
            const head_list_type &synonyms,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc ) const;

    void topKUnionWithAuxFilters(
            const aux_filter_list_type &aux_filters,
            const range_list_type &patterns,
            const head_list_type &synonyms,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    size_t getOcc(char_type c) const;
    WaveletTreeNode *getRoot() const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    void doIntersect_(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            size_t level,
            char_type symbol,
            std::vector<char_type> &results) const;

protected:
    sdarray::SDArray occ_;
    std::vector<size_t> zero_counts_;
    std::vector<WaveletTreeNode *> nodes_;
};

template <class CharT>
WaveletMatrix<CharT>::WaveletMatrix(uint64_t alphabet_num, bool support_select, bool dense)
    : WaveletTree<CharT>(alphabet_num, support_select, dense)
{
}

template <class CharT>
WaveletMatrix<CharT>::~WaveletMatrix()
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }
}

template <class CharT>
void WaveletMatrix<CharT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    zero_counts_.resize(this->alphabet_bit_num_);

    nodes_.resize(this->alphabet_bit_num_);

    std::vector<size_t> prev_begin_pos(1), node_begin_pos(1);

    char_type *temp_seq = new char_type[len];
    char_type *curr = const_cast<char_type *>(char_seq);
    char_type *next = temp_seq;

    for (size_t i = 0; i < this->alphabet_bit_num_; ++i)
    {
        nodes_[i] = new WaveletTreeNode(this->support_select_, this->dense_);
        nodes_[i]->resize(len);

        size_t zero_count = 0;
        char_type bit_mask = (char_type)1 << i;

        for (size_t j = 0; j < len; ++j)
        {
            if (curr[j] & bit_mask)
            {
                nodes_[i]->setBit(j);
            }
            else
            {
                nodes_[i]->unsetBit(j);
                next[zero_count++] = curr[j];
            }
        }

        nodes_[i]->build();
        zero_counts_[i] = zero_count;

        for (size_t j = 0; j < len; ++j)
        {
            if (curr[j] & bit_mask)
            {
                next[zero_count++] = curr[j];
            }
        }
        std::swap(curr, next);
    }

    size_t curr_char = 0;
    size_t curr_count = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (curr[i] == curr_char)
        {
            ++curr_count;
        }
        else
        {
            occ_.add(curr_count);
            for (++curr_char; curr_char < curr[i]; ++curr_char)
            {
                occ_.add(0);
            }
            curr_count = 1;
        }
    }

    occ_.add(curr_count);
    for (++curr_char; curr_char <= 1LLU << this->alphabet_bit_num_; ++curr_char)
    {
        occ_.add(0);
    }

    occ_.build();

    delete[] temp_seq;

    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
    }
}

template <class CharT>
CharT WaveletMatrix<CharT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]->access(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask <<= 1;
    }

    return c;
}

template <class CharT>
CharT WaveletMatrix<CharT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]->access(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask <<= 1;
    }

    rank = pos - occ_.prefixSum(c);

    return c;
}

template <class CharT>
size_t WaveletMatrix<CharT>::rank(char_type c, size_t pos) const
{
    pos = std::min(pos, length());

    char_type bit_mask = 1;

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (c & bit_mask)
        {
            pos = nodes_[i]->rank1(pos) + zero_counts_[i];
        }
        else
        {
            pos -= nodes_[i]->rank1(pos);
        }

        bit_mask <<= 1;
    }

    return pos - occ_.prefixSum(c);
}

template <class CharT>
size_t WaveletMatrix<CharT>::select(char_type c, size_t rank) const
{
    size_t pos = rank + occ_.prefixSum(c);

    for (size_t i = nodes_.size() - 1; i < nodes_.size(); --i)
    {
        if (pos >= zero_counts_[i])
        {
            pos = nodes_[i]->select1(pos - zero_counts_[i]);
        }
        else
        {
            pos = nodes_[i]->select0(pos);
        }

        if (pos == (size_t)-1) return -1;
    }

    return pos;
}

template <class CharT>
void WaveletMatrix<CharT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        std::vector<char_type> &results) const
{
    if (thres > patterns.size()) return;
    if (thres > 0) thres = patterns.size() - thres;

    doIntersect_(patterns, thres, max_count, 0, 0, results);
}

template <class CharT>
void WaveletMatrix<CharT>::doIntersect_(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        size_t level,
        char_type symbol,
        std::vector<char_type> &results) const
{
    if (results.size() >= max_count) return;

    if (level == this->alphabet_bit_num_)
    {
        results.push_back(symbol);
        return;
    }

    std::vector<std::pair<size_t, size_t> > zero_ranges, one_ranges;
    zero_ranges.reserve(patterns.size());
    one_ranges.reserve(patterns.size());

    size_t zero_thres = thres, one_thres = thres;
    bool has_zeros = true, has_ones = true;

    const WaveletTreeNode *node = nodes_[level];

    size_t rank_start, rank_end;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        rank_start = node->rank1(it->first);
        rank_end = node->rank1(it->second);

        if (has_zeros)
        {
            if (it->first - rank_start >= it->second - rank_end)
            {
                if (zero_thres == 0)
                {
                    if (!has_ones) return;
                    has_zeros = false;
                }
                else
                {
                    --zero_thres;
                }
            }
            else
            {
                zero_ranges.push_back(std::make_pair(it->first - rank_start, it->second - rank_end));
            }
        }

        if (has_ones)
        {
            if (rank_start >= rank_end)
            {
                if (one_thres == 0)
                {
                    if (!has_zeros) return;
                    has_ones = false;
                }
                else
                {
                    --one_thres;
                }
            }
            else
            {
                one_ranges.push_back(std::make_pair(rank_start + zero_counts_[level], rank_end + zero_counts_[level]));
            }
        }
    }

    if (has_zeros)
    {
        doIntersect_(zero_ranges, zero_thres, max_count, level + 1, symbol, results);
    }

    if (has_ones)
    {
        symbol |= (char_type)1 << level;

        doIntersect_(one_ranges, one_thres, max_count, level + 1, symbol, results);
    }
}

template <class CharT>
void WaveletMatrix<CharT>::topKUnion(
        const range_list_type &patterns,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc ) const
{
    if (topK == 0) return;
    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<PatternList *> ranges_heap(max_queue_size + 1);
    ranges_heap.insert(BOOST_NEW(alloc, PatternList)(0, (char_type)0, nodes_[0], patterns));

    if (ranges_heap.get_max()->score_ == 0.0)
    {
        //delete ranges_heap.get_max();
        return;
    }

    results.reserve(topK);

    std::vector<PatternList *, boost::stl_allocator<PatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<PatternList *, boost::stl_allocator<PatternList *> > top_queue(alloc);

    PatternList *top_ranges;
    PatternList *zero_ranges, *one_ranges;
    size_t level, rank_start, rank_end, zero_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back();
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max();
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        level = top_ranges->level_;
        zero_end = zero_counts_[level];
        node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, PatternList)(level + 1, top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(level + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, PatternList)(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_);
            recyc_queue.pop_back();
        }

        const range_list_type &patterns = top_ranges->patterns_;
        range_list_type::const_iterator pit = patterns.begin();
        range_list_type::const_iterator pitEnd = pit + thres;

        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges && !zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>())))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>())))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        pitEnd = patterns.end();
        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges)
            {
                zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>()));
            }
            if (one_ranges)
            {
                one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>()));
            }
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_ranges);
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(zero_ranges);
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_ranges);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min());
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min()->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(one_ranges);

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min());
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    interval_heap<PatternList *>::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first;
        delete ranges_list[i].second;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i];
    }*/
}

template <class CharT>
void WaveletMatrix<CharT>::topKUnionWithFilters(
        const range_list_type &filters,
        const range_list_type &patterns,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<FilteredPatternList *> ranges_heap(max_queue_size + 1);
    ranges_heap.insert(BOOST_NEW(alloc, FilteredPatternList)(0, (char_type)0, nodes_[0], filters, patterns));

    if (ranges_heap.get_max()->score_ == 0.0)
    {
        //delete ranges_heap.get_max();
        return;
    }

    results.reserve(topK);

    std::vector<FilteredPatternList *, boost::stl_allocator<FilteredPatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<FilteredPatternList *, boost::stl_allocator<FilteredPatternList *> > top_queue(alloc);

    FilteredPatternList *top_ranges;
    FilteredPatternList *zero_ranges, *one_ranges;
    size_t level, rank_start, rank_end, zero_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back();
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max();
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        level = top_ranges->level_;
        zero_end = zero_counts_[level];
        node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, FilteredPatternList)(level + 1, top_ranges->sym_, node->left_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(level + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, FilteredPatternList)(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_);
            recyc_queue.pop_back();
        }

        for (range_list_type::const_iterator it = top_ranges->filters_.begin();
                it != top_ranges->filters_.end(); ++it)
        {
            rank_start = node->rank1(it->get<0>());
            rank_end = node->rank1(it->get<1>());

            zero_ranges->addFilter(boost::make_tuple(it->get<0>() - rank_start, it->get<1>() - rank_end, it->get<2>()));
            one_ranges->addFilter(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, it->get<2>()));
        }

        if (zero_ranges->filters_.empty())
        {
            recyc_queue.push_back(zero_ranges);
            zero_ranges = NULL;
        }
        if (one_ranges->filters_.empty())
        {
            recyc_queue.push_back(one_ranges);
            one_ranges = NULL;
            if (!zero_ranges)
            {
                recyc_queue.push_back(top_ranges);
                continue;
            }
        }

        const range_list_type &patterns = top_ranges->patterns_;
        range_list_type::const_iterator pit = patterns.begin();
        range_list_type::const_iterator pitEnd = pit + thres;

        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges && !zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>())))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>())))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        pitEnd = patterns.end();
        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges)
            {
                zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>()));
            }
            if (one_ranges)
            {
                one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>()));
            }
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_ranges);
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(zero_ranges);
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_ranges);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min());
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min()->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(one_ranges);

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min());
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    interval_heap<FilteredPatternList *>::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first;
        delete ranges_list[i].second;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i];
    }*/
}

template <class CharT>
void WaveletMatrix<CharT>::topKUnionWithAuxFilters(
        const aux_filter_list_type &aux_filters,
        const range_list_type &patterns,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<AuxFilteredPatternList<self_type> *> ranges_heap(max_queue_size + 1);
    ranges_heap.insert(BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(0, (char_type)0, nodes_[0], aux_filters, patterns, alloc));

    if (ranges_heap.get_max()->score_ == 0.0)
    {
        //delete ranges_heap.get_max();
        return;
    }

    results.reserve(topK);

    size_t max_filter_size = 0;
    for (size_t i = 0; i < aux_filters.size(); ++i)
    {
        max_filter_size = std::max(max_filter_size, aux_filters[i]->filters_.size());
    }

    std::vector<AuxFilteredPatternList<self_type> *, boost::stl_allocator<AuxFilteredPatternList<self_type> *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<AuxFilteredPatternList<self_type> *, boost::stl_allocator<AuxFilteredPatternList<self_type> *> > top_queue(alloc);

    AuxFilteredPatternList<self_type> *top_ranges;
    AuxFilteredPatternList<self_type> *zero_ranges, *one_ranges;
    FilterList<self_type> *zero_filter, *one_filter;
    zero_filter = one_filter = NULL;
    size_t level, rank_start, rank_end, zero_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back();
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max();
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        level = top_ranges->level_;
        node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(level + 1, top_ranges->sym_, node->left_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(level + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, AuxFilteredPatternList<self_type>)(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_);
            recyc_queue.pop_back();
        }

        for (typename aux_filter_list_type::const_iterator it = top_ranges->aux_filters_.begin();
                it != top_ranges->aux_filters_.end(); ++it)
        {
            zero_end = (*it)->tree_->zero_counts_[level];
            node = (*it)->node_;

            if (zero_ranges)
            {
                zero_filter = zero_ranges->getAuxFilter((*it)->tree_, node->left_, max_filter_size);
            }
            if (one_ranges)
            {
                one_filter = one_ranges->getAuxFilter((*it)->tree_, node->right_, max_filter_size);
            }

            for (range_list_type::const_iterator fit = (*it)->filters_.begin();
                    fit != (*it)->filters_.end(); ++fit)
            {
                rank_start = node->rank1(fit->get<0>());
                rank_end = node->rank1(fit->get<1>());

                if (zero_ranges)
                {
                    zero_filter->addFilter(boost::make_tuple(fit->get<0>() - rank_start, fit->get<1>() - rank_end, fit->get<2>()));
                }
                if (one_ranges)
                {
                    one_filter->addFilter(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, fit->get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addAuxFilter(zero_filter))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addAuxFilter(one_filter))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        zero_end = zero_counts_[level];
        node = top_ranges->node_;

        const range_list_type &patterns = top_ranges->patterns_;
        range_list_type::const_iterator pit = patterns.begin();
        range_list_type::const_iterator pitEnd = pit + thres;

        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges && !zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>())))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>())))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        pitEnd = patterns.end();
        for (; pit != pitEnd; ++pit)
        {
            rank_start = node->rank1(pit->get<0>());
            rank_end = node->rank1(pit->get<1>());

            if (zero_ranges)
            {
                zero_ranges->addPattern(boost::make_tuple(pit->get<0>() - rank_start, pit->get<1>() - rank_end, pit->get<2>()));
            }
            if (one_ranges)
            {
                one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, pit->get<2>()));
            }
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_ranges);
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(zero_ranges);
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_ranges);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min());
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min()->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(one_ranges);

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min());
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    typename interval_heap<AuxFilteredPatternList<self_type> *>::container_type ranges_list = ranges_heap.get_container();
    for (size_t i = 0; i < ranges_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first;
        delete ranges_list[i].second;
    }
    if (ranges_heap.size() % 2)
    {
        delete ranges_list[ranges_heap.size() / 2].first;
    }

    for (size_t i = 0; i < recyc_queue.size(); ++i)
    {
        delete recyc_queue[i];
    }

    for (size_t i = 0; i < top_queue.size(); ++i)
    {
        delete top_queue[i];
    }*/
}

template <class CharT>
void WaveletMatrix<CharT>::topKUnion(
        const range_list_type &patterns,
        const head_list_type &synonyms,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc ) const
{
    if (topK == 0) return;
    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);
    interval_heap<SynonymPatternList *> ranges_heap(max_queue_size + 1);
    ranges_heap.insert(BOOST_NEW(alloc, SynonymPatternList)(0, (char_type)0, nodes_[0], patterns, synonyms));

    if (ranges_heap.get_max()->score_ == 0.0)
    {
        //delete ranges_heap.get_max();
        return;
    }

    results.reserve(topK);

    std::vector<SynonymPatternList *, boost::stl_allocator<SynonymPatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<SynonymPatternList *, boost::stl_allocator<SynonymPatternList *> > top_queue(alloc);

    SynonymPatternList *top_ranges;
    SynonymPatternList *zero_ranges, *one_ranges;
    size_t level, rank_start, rank_end, zero_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back();
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max();
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        level = top_ranges->level_;
        zero_end = zero_counts_[level];
        node = top_ranges->node_;

        if (recyc_queue.empty())
        {

            zero_ranges = BOOST_NEW(alloc, SynonymPatternList)
            (level + 1, top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(level + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, SynonymPatternList)(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_, top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_);
            recyc_queue.pop_back();
        }

        const range_list_type &patterns = top_ranges->patterns_;
        const head_list_type &synonyms = top_ranges->synonyms_;

        for (size_t i = 0; i < thres; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                rank_start = node->rank1(patterns[j].get<0>());
                rank_end = node->rank1(patterns[j].get<1>());

                if (zero_ranges)
                {
                    zero_ranges->addPattern(boost::make_tuple(patterns[j].get<0>() - rank_start, patterns[j].get<1>() - rank_end, patterns[j].get<2>()));
                }
                if (one_ranges)
                {
                    one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, patterns[j].get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addSynonym())
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addSynonym())
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        for (size_t i = thres; i < synonyms.size() - 1; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                rank_start = node->rank1(patterns[j].get<0>());
                rank_end = node->rank1(patterns[j].get<1>());

                if (zero_ranges)
                {
                    zero_ranges->addPattern(boost::make_tuple(patterns[j].get<0>() - rank_start, patterns[j].get<1>() - rank_end, patterns[j].get<2>()));
                }
                if (one_ranges)
                {
                    one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, patterns[j].get<2>()));
                }
            }

            if (zero_ranges) zero_ranges->addSynonym();
            if (one_ranges) one_ranges->addSynonym();
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_ranges);
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(zero_ranges);
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_ranges);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min());
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min()->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(one_ranges);

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min());
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }

}

template <class CharT>
void WaveletMatrix<CharT>::topKUnionWithAuxFilters(
        const aux_filter_list_type &aux_filters,
        const range_list_type &patterns,
        const head_list_type &synonyms,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    interval_heap<AuxFilteredSynonymPatternList<self_type> *> ranges_heap(max_queue_size + 1);
    ranges_heap.insert(BOOST_NEW(alloc, AuxFilteredSynonymPatternList<self_type>)(0, (char_type)0, nodes_[0], aux_filters, patterns, synonyms, alloc));

    if (ranges_heap.get_max()->score_ == 0.0)
    {
        //delete ranges_heap.get_max();
        return;
    }

    results.reserve(topK);

    size_t max_filter_size = 0;
    for (size_t i = 0; i < aux_filters.size(); ++i)
    {
        max_filter_size = std::max(max_filter_size, aux_filters[i]->filters_.size());
    }

    std::vector<AuxFilteredSynonymPatternList<self_type> *, boost::stl_allocator<AuxFilteredSynonymPatternList<self_type> *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<AuxFilteredSynonymPatternList<self_type> *, boost::stl_allocator<AuxFilteredSynonymPatternList<self_type> *> > top_queue(alloc);

    AuxFilteredSynonymPatternList<self_type> *top_ranges;
    AuxFilteredSynonymPatternList<self_type> *zero_ranges, *one_ranges;
    FilterList<self_type> *zero_filter, *one_filter;
    zero_filter = one_filter = NULL;
    size_t level, rank_start, rank_end, zero_end;
    const WaveletTreeNode *node;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_ranges = top_queue.back();
            top_queue.pop_back();
        }
        else if (!ranges_heap.empty())
        {
            top_ranges = ranges_heap.get_max();
            ranges_heap.pop_max();
        }
        else
        {
            break;
        }

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_ranges->score_, top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        level = top_ranges->level_;
        node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, AuxFilteredSynonymPatternList<self_type>)(level + 1, top_ranges->sym_, node->left_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(level + 1, top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, AuxFilteredSynonymPatternList<self_type>)(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_, top_ranges->aux_filters_.capacity(), top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(level + 1, top_ranges->sym_ | (char_type)1 << level, node->right_);
            recyc_queue.pop_back();
        }

        for (typename aux_filter_list_type::const_iterator it = top_ranges->aux_filters_.begin();
                it != top_ranges->aux_filters_.end(); ++it)
        {
            zero_end = (*it)->tree_->zero_counts_[level];
            node = (*it)->node_;

            if (zero_ranges)
            {
                zero_filter = zero_ranges->getAuxFilter((*it)->tree_, node->left_, max_filter_size);
            }
            if (one_ranges)
            {
                one_filter = one_ranges->getAuxFilter((*it)->tree_, node->right_, max_filter_size);
            }

            for (range_list_type::const_iterator fit = (*it)->filters_.begin();
                    fit != (*it)->filters_.end(); ++fit)
            {

                rank_start = node->rank1(fit->get<0>());
                rank_end = node->rank1(fit->get<1>());

                if (zero_ranges)
                {
                    zero_filter->addFilter(boost::make_tuple(fit->get<0>() - rank_start, fit->get<1>() - rank_end, fit->get<2>()));
                }
                if (one_ranges)
                {
                    one_filter->addFilter(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, fit->get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addAuxFilter(zero_filter))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addAuxFilter(one_filter))
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        zero_end = zero_counts_[level];
        node = top_ranges->node_;

        const range_list_type &patterns = top_ranges->patterns_;
        const head_list_type &synonyms = top_ranges->synonyms_;

        for (size_t i = 0; i < thres; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                rank_start = node->rank1(patterns[j].get<0>());
                rank_end = node->rank1(patterns[j].get<1>());

                if (zero_ranges)
                {
                    zero_ranges->addPattern(boost::make_tuple(patterns[j].get<0>() - rank_start, patterns[j].get<1>() - rank_end, patterns[j].get<2>()));
                }
                if (one_ranges)
                {
                    one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, patterns[j].get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addSynonym())
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addSynonym())
            {
                recyc_queue.push_back(one_ranges);
                one_ranges = NULL;
                if (!zero_ranges) break;
            }
        }

        if (!zero_ranges && !one_ranges)
        {
            recyc_queue.push_back(top_ranges);
            continue;
        }

        for (size_t i = thres; i < synonyms.size() - 1; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                rank_start = node->rank1(patterns[j].get<0>());
                rank_end = node->rank1(patterns[j].get<1>());

                if (zero_ranges)
                {
                    zero_ranges->addPattern(boost::make_tuple(patterns[j].get<0>() - rank_start, patterns[j].get<1>() - rank_end, patterns[j].get<2>()));
                }
                if (one_ranges)
                {
                    one_ranges->addPattern(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, patterns[j].get<2>()));
                }
            }

            if (zero_ranges) zero_ranges->addSynonym();
            if (one_ranges) one_ranges->addSynonym();
        }

        if (zero_ranges)
        {
            zero_ranges->calcScore();
            if (zero_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || zero_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_ranges);
                }
                else
                {
                    results.push_back(std::make_pair(zero_ranges->score_, zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                ranges_heap.insert(zero_ranges);
            }
        }

        if (one_ranges)
        {
            one_ranges->calcScore();
            if (one_ranges->score_ == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_ranges->score_ == top_ranges->score_ || (top_queue.empty() && (ranges_heap.empty() || one_ranges->score_ >= ranges_heap.get_max()->score_)))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_ranges);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + ranges_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(ranges_heap.get_min());
                        ranges_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_ranges->score_, one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() == max_queue_size || (top_queue.size() + ranges_heap.size() == max_queue_size && one_ranges->score_ < ranges_heap.get_min()->score_))
            {
                recyc_queue.push_back(one_ranges);
            }
            else
            {
                ranges_heap.insert(one_ranges);

                if (top_queue.size() + ranges_heap.size() > max_queue_size)
                {
                    recyc_queue.push_back(ranges_heap.get_min());
                    ranges_heap.pop_min();
                }
            }
        }

        recyc_queue.push_back(top_ranges);
    }

}


template <class CharT>
size_t WaveletMatrix<CharT>::getOcc(char_type c) const
{
    if (c <= occ_.size()) return occ_.prefixSum(c);
    return occ_.getSum();
}

template <class CharT>
WaveletTreeNode *WaveletMatrix<CharT>::getRoot() const
{
    if (nodes_.empty()) return NULL;
    else return nodes_[0];
}

template <class CharT>
size_t WaveletMatrix<CharT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT>
size_t WaveletMatrix<CharT>::allocSize() const
{
    size_t sum = sizeof(WaveletMatrix<char_type>) + occ_.allocSize() + sizeof(zero_counts_[0]) * zero_counts_.size();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->allocSize();

    return sum;
}

template <class CharT>
void WaveletMatrix<CharT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);
    occ_.save(ostr);

    ostr.write((const char *)&zero_counts_[0], sizeof(zero_counts_[0]) * zero_counts_.size());

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT>
void WaveletMatrix<CharT>::load(std::istream &istr)
{
    WaveletTree<CharT>::load(istr);
    occ_.load(istr);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }

    zero_counts_.resize(this->alphabet_bit_num_);
    istr.read((char *)&zero_counts_[0], sizeof(zero_counts_[0]) * zero_counts_.size());

    nodes_.resize(this->alphabet_bit_num_);
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i] = new WaveletTreeNode(this->support_select_, this->dense_);
        nodes_[i]->load(istr);
    }
    for (size_t i = 1; i < nodes_.size(); ++i)
    {
        nodes_[i]->parent_ = nodes_[i - 1];
        nodes_[i - 1]->left_ = nodes_[i];
        nodes_[i - 1]->right_ = nodes_[i];
    }
}

}
}

NS_IZENELIB_AM_END

#endif
