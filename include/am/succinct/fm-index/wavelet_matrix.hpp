#ifndef _FM_INDEX_WAVELET_MATRIX_HPP
#define _FM_INDEX_WAVELET_MATRIX_HPP

#include "wavelet_tree.hpp"
#include "wavelet_tree_node.hpp"
#include "range_list.hpp"
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/interval_heap.hpp>

#include <deque>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT, class BitmapT>
class WaveletMatrix : public WaveletTree<CharT>
{
public:
    typedef CharT char_type;
    typedef WaveletMatrix<CharT, BitmapT> self_type;
    typedef WaveletTreeNode<BitmapT> node_type;
    typedef std::vector<FilterItem<self_type> *, boost::stl_allocator<FilterItem<self_type> *> > filter_list_type;

    WaveletMatrix(size_t alphabet_num, bool support_select);
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
            const filter_list_type &filters,
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
            boost::auto_alloc& alloc) const;

    void topKUnionWithFilters(
            const filter_list_type &filters,
            const range_list_type &patterns,
            const head_list_type &synonyms,
            size_t thres,
            size_t topK,
            std::vector<std::pair<double, char_type> > &results,
            boost::auto_alloc& alloc) const;

    size_t beginOcc(char_type c) const;
    size_t endOcc(char_type c) const;

    node_type *getRoot() const;

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
    std::vector<node_type *> nodes_;
};

template <class CharT, class BitmapT>
WaveletMatrix<CharT, BitmapT>::WaveletMatrix(uint64_t alphabet_num, bool support_select)
    : WaveletTree<CharT>(alphabet_num, support_select)
{
}

template <class CharT, class BitmapT>
WaveletMatrix<CharT, BitmapT>::~WaveletMatrix()
{
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]) delete nodes_[i];
    }
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::build(const char_type *char_seq, size_t len)
{
    if (this->alphabet_num_ == 0) return;

    zero_counts_.resize(this->alphabet_bit_num_);

    nodes_.resize(this->alphabet_bit_num_);

    std::vector<size_t> prev_begin_pos(1), node_begin_pos(1);

    char_type *temp_seq = new char_type[len];
    char_type *curr = const_cast<char_type *>(char_seq);
    char_type *next = temp_seq;

    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < this->alphabet_bit_num_; ++i)
    {
        nodes_[i] = new node_type(this->support_select_);
        nodes_[i]->resize(len);

        size_t zero_count = 0;

        for (size_t j = 0; j < len; ++j)
        {
            if (curr[j] & bit_mask)
            {
                nodes_[i]->setBit(j);
            }
            else
            {
                //nodes_[i]->unsetBit(j);
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

        bit_mask >>= 1;
    }

    size_t curr_char = 0;
    char_type tmp_char = 0;
    size_t curr_count = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (curr[i] == tmp_char)
        {
            ++curr_count;
        }
        else
        {
            occ_.add(curr_count);

            tmp_char = SuccinctUtils::bitReverse(curr[i], this->alphabet_bit_num_);
            for (++curr_char; curr_char < tmp_char; ++curr_char)
            {
                occ_.add(0);
            }
            tmp_char = curr[i];

            curr_count = 1;
        }
    }

    occ_.add(curr_count);
    for (++curr_char; curr_char <= 1ULL << this->alphabet_bit_num_; ++curr_char)
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

template <class CharT, class BitmapT>
CharT WaveletMatrix<CharT, BitmapT>::access(size_t pos) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]->access(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask >>= 1;
    }

    return c;
}

template <class CharT, class BitmapT>
CharT WaveletMatrix<CharT, BitmapT>::access(size_t pos, size_t &rank) const
{
    if (pos >= length()) return -1;

    char_type c = 0;
    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        if (nodes_[i]->access(pos, pos))
        {
            c |= bit_mask;
            pos += zero_counts_[i];
        }

        bit_mask >>= 1;
    }

    rank = pos - occ_.prefixSum(SuccinctUtils::bitReverse(c, this->alphabet_bit_num_));

    return c;
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::rank(char_type c, size_t pos) const
{
    pos = std::min(pos, length());

    char_type bit_mask = (char_type)1 << (this->alphabet_bit_num_ - 1);

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

        bit_mask >>= 1;
    }

    return pos - occ_.prefixSum(SuccinctUtils::bitReverse(c, this->alphabet_bit_num_));
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::select(char_type c, size_t rank) const
{
    size_t pos = rank + occ_.prefixSum(SuccinctUtils::bitReverse(c, this->alphabet_bit_num_));

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

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::intersect(
        const std::vector<std::pair<size_t, size_t> > &patterns,
        size_t thres,
        size_t max_count,
        std::vector<char_type> &results) const
{
    if (thres > patterns.size()) return;
    if (thres > 0) thres = patterns.size() - thres;

    results.reserve(max_count);
    doIntersect_(patterns, thres, max_count, 0, 0, results);
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::doIntersect_(
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

    const node_type *node = nodes_[level];

    size_t zero_count = zero_counts_[level];

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = patterns.begin();
            it != patterns.end(); ++it)
    {
        size_t rank_start = node->rank1(it->first);
        size_t rank_end = node->rank1(it->second);

        if (has_zeros)
        {
            if (it->first - rank_start < it->second - rank_end)
            {
                zero_ranges.push_back(std::make_pair(it->first - rank_start, it->second - rank_end));
            }
            else if (zero_thres-- == 0)
            {
                if (!has_ones) return;
                has_zeros = false;
            }
        }

        if (has_ones)
        {
            if (rank_start < rank_end)
            {
                one_ranges.push_back(std::make_pair(rank_start + zero_count, rank_end + zero_count));
            }
            else if (one_thres-- == 0)
            {
                if (!has_zeros) return;
                has_ones = false;
            }
        }
    }

    if (has_zeros)
    {
        doIntersect_(zero_ranges, zero_thres, max_count, level + 1, symbol, results);
    }

    if (has_ones)
    {
        doIntersect_(one_ranges, one_thres, max_count, level + 1, symbol | (char_type)1 << (this->alphabet_bit_num_ - level - 1), results);
    }
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::topKUnion(
        const range_list_type &patterns,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    PatternList *top_ranges = BOOST_NEW(alloc, PatternList)((char_type)0, nodes_[0], patterns);
    float score = top_ranges->score();
    if (score == 0.0f)
    {
        //delete top_ranges;
        return;
    }

    typedef boost::tuple<float, uint32_t, PatternList *> state_type;
    state_type top_state(score, 0U, top_ranges);

    interval_heap<state_type> state_heap(max_queue_size + 1);
    state_heap.insert(top_state);

    results.reserve(topK);

    std::vector<PatternList *, boost::stl_allocator<PatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<state_type, boost::stl_allocator<state_type> > top_queue(alloc);

    PatternList *zero_ranges, *one_ranges;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_state = top_queue.back();
            top_queue.pop_back();
        }
        else if (!state_heap.empty())
        {
            top_state = state_heap.get_max();
            state_heap.pop_max();
        }
        else
        {
            break;
        }

        top_ranges = top_state.get<2>();

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_state.get<0>(), top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        size_t level = top_state.get<1>();
        size_t zero_end = zero_counts_[level];
        const node_type *node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, PatternList)(top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, PatternList)(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_, top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_);
            recyc_queue.pop_back();
        }

        const range_list_type &patterns = top_ranges->patterns_;
        range_list_type::const_iterator pit = patterns.begin();
        range_list_type::const_iterator pitEnd = pit + thres;

        for (; pit != pitEnd; ++pit)
        {
            size_t rank_start = node->rank1(pit->get<0>());
            size_t rank_end = node->rank1(pit->get<1>());

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
            size_t rank_start = node->rank1(pit->get<0>());
            size_t rank_end = node->rank1(pit->get<1>());

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
            state_type zero_state(zero_ranges->score(), level + 1, zero_ranges);
            if (zero_state.get<0>() == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_state.get<0>() == top_state.get<0>() || (top_queue.empty() && (state_heap.empty() || !(zero_state < state_heap.get_max()))))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_state);
                }
                else
                {
                    results.push_back(std::make_pair(zero_state.get<0>(), zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                state_heap.insert(zero_state);
            }
        }

        if (one_ranges)
        {
            state_type one_state(one_ranges->score(), level + 1, one_ranges);
            if (one_state.get<0>() == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_state.get<0>() == top_state.get<0>() || (top_queue.empty() && (state_heap.empty() || !(one_state < state_heap.get_max()))))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_state);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front().get<2>());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + state_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(state_heap.get_min().get<2>());
                        state_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_state.get<0>(), one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() + state_heap.size() < max_queue_size)
            {
                state_heap.insert(one_state);
            }
            else if (top_queue.size() == max_queue_size)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (state_heap.get_min() < one_state)
            {
                recyc_queue.push_back(state_heap.get_min().get<2>());
                state_heap.replace_min(one_state);
            }
            else
            {
                recyc_queue.push_back(one_ranges);
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    interval_heap<PatternList *>::container_type ranges_list = state_heap.get_container();
    for (size_t i = 0; i < state_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first;
        delete ranges_list[i].second;
    }
    if (state_heap.size() % 2)
    {
        delete ranges_list[state_heap.size() / 2].first;
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

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::topKUnionWithFilters(
        const filter_list_type &filters,
        const range_list_type &patterns,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    FilteredPatternList<self_type> *top_ranges = BOOST_NEW(alloc, FilteredPatternList<self_type>)((char_type)0, nodes_[0], filters, patterns, alloc);
    float score = top_ranges->score();
    if (score == 0.0f)
    {
        //delete top_ranges;
        return;
    }

    typedef boost::tuple<float, uint32_t, FilteredPatternList<self_type> *> state_type;
    state_type top_state(score, 0U, top_ranges);

    interval_heap<state_type> state_heap(max_queue_size + 1);
    state_heap.insert(top_state);

    results.reserve(topK);

    size_t max_filter_size = 0;
    for (size_t i = 0; i < filters.size(); ++i)
    {
        max_filter_size = std::max(max_filter_size, filters[i]->segments_.size());
    }

    std::vector<FilteredPatternList<self_type> *, boost::stl_allocator<FilteredPatternList<self_type> *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<state_type, boost::stl_allocator<state_type> > top_queue(alloc);

    FilteredPatternList<self_type> *zero_ranges, *one_ranges;
    FilterItem<self_type> *zero_filter = NULL, *one_filter = NULL;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_state = top_queue.back();
            top_queue.pop_back();
        }
        else if (!state_heap.empty())
        {
            top_state = state_heap.get_max();
            state_heap.pop_max();
        }
        else
        {
            break;
        }

        top_ranges = boost::get<2>(top_state);

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(boost::get<0>(top_state), top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        size_t level = boost::get<1>(top_state);
        const node_type *node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, FilteredPatternList<self_type>)(top_ranges->sym_, node->left_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, FilteredPatternList<self_type>)(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_);
            recyc_queue.pop_back();
        }

        for (typename filter_list_type::const_iterator it = top_ranges->filters_.begin();
                it != top_ranges->filters_.end(); ++it)
        {
            size_t zero_end = (*it)->tree_->zero_counts_[level];
            node = (*it)->node_;

            if (zero_ranges)
            {
                zero_filter = zero_ranges->getFilter((*it)->tree_, node->left_, max_filter_size);
            }
            if (one_ranges)
            {
                one_filter = one_ranges->getFilter((*it)->tree_, node->right_, max_filter_size);
            }

            for (range_list_type::const_iterator fit = (*it)->segments_.begin();
                    fit != (*it)->segments_.end(); ++fit)
            {
                size_t rank_start = node->rank1(fit->get<0>());
                size_t rank_end = node->rank1(fit->get<1>());

                if (zero_ranges)
                {
                    zero_filter->addSegment(boost::make_tuple(fit->get<0>() - rank_start, fit->get<1>() - rank_end, fit->get<2>()));
                }
                if (one_ranges)
                {
                    one_filter->addSegment(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, fit->get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addFilter(zero_filter))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addFilter(one_filter))
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

        size_t zero_end = zero_counts_[level];
        node = top_ranges->node_;

        const range_list_type &patterns = top_ranges->patterns_;
        range_list_type::const_iterator pit = patterns.begin();
        range_list_type::const_iterator pitEnd = pit + thres;

        for (; pit != pitEnd; ++pit)
        {
            size_t rank_start = node->rank1(pit->get<0>());
            size_t rank_end = node->rank1(pit->get<1>());

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
            size_t rank_start = node->rank1(pit->get<0>());
            size_t rank_end = node->rank1(pit->get<1>());

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
            state_type zero_state(zero_ranges->score(), level + 1, zero_ranges);
            if (boost::get<0>(zero_state) == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (boost::get<0>(zero_state) == boost::get<0>(top_state) || (top_queue.empty() && (state_heap.empty() || !(zero_state < state_heap.get_max()))))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_state);
                }
                else
                {
                    results.push_back(std::make_pair(boost::get<0>(zero_state), zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                state_heap.insert(zero_state);
            }
        }

        if (one_ranges)
        {
            state_type one_state(one_ranges->score(), level + 1, one_ranges);
            if (boost::get<0>(one_state) == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (boost::get<0>(one_state) == boost::get<0>(top_state) || (top_queue.empty() && (state_heap.empty() || !(one_state < state_heap.get_max()))))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_state);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(boost::get<2>(top_queue.front()));
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + state_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(boost::get<2>(state_heap.get_min()));
                        state_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(boost::get<0>(one_state), one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() + state_heap.size() < max_queue_size)
            {
                state_heap.insert(one_state);
            }
            else if (top_queue.size() == max_queue_size)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (state_heap.get_min() < one_state)
            {
                recyc_queue.push_back(boost::get<2>(state_heap.get_min()));
                state_heap.replace_min(one_state);
            }
            else
            {
                recyc_queue.push_back(one_ranges);
            }
        }

        recyc_queue.push_back(top_ranges);
    }
    /*
    typename interval_heap<FilteredPatternList<self_type> *>::container_type ranges_list = state_heap.get_container();
    for (size_t i = 0; i < state_heap.size() / 2; ++i)
    {
        delete ranges_list[i].first;
        delete ranges_list[i].second;
    }
    if (state_heap.size() % 2)
    {
        delete ranges_list[state_heap.size() / 2].first;
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

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::topKUnion(
        const range_list_type &patterns,
        const head_list_type &synonyms,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    SynonymPatternList *top_ranges = BOOST_NEW(alloc, SynonymPatternList)((char_type)0, nodes_[0], patterns, synonyms);
    float score = top_ranges->score();
    if (score == 0.0f)
    {
        //delete top_ranges;
        return;
    }

    typedef boost::tuple<float, uint32_t, SynonymPatternList *> state_type;
    state_type top_state(score, 0U, top_ranges);

    interval_heap<state_type> state_heap(max_queue_size + 1);
    state_heap.insert(top_state);

    results.reserve(topK);

    std::vector<SynonymPatternList *, boost::stl_allocator<SynonymPatternList *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<state_type, boost::stl_allocator<state_type> > top_queue(alloc);

    SynonymPatternList *zero_ranges, *one_ranges;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_state = top_queue.back();
            top_queue.pop_back();
        }
        else if (!state_heap.empty())
        {
            top_state = state_heap.get_max();
            state_heap.pop_max();
        }
        else
        {
            break;
        }

        top_ranges = top_state.get<2>();

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(top_state.get<0>(), top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        size_t level = top_state.get<1>();
        size_t zero_end = zero_counts_[level];
        const node_type *node = top_ranges->node_;

        if (recyc_queue.empty())
        {

            zero_ranges = BOOST_NEW(alloc, SynonymPatternList)(top_ranges->sym_, node->left_, top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, SynonymPatternList)(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_, top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_);
            recyc_queue.pop_back();
        }

        const range_list_type &patterns = top_ranges->patterns_;
        const head_list_type &synonyms = top_ranges->synonyms_;

        for (size_t i = 0; i < thres; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                size_t rank_start = node->rank1(patterns[j].get<0>());
                size_t rank_end = node->rank1(patterns[j].get<1>());

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
                size_t rank_start = node->rank1(patterns[j].get<0>());
                size_t rank_end = node->rank1(patterns[j].get<1>());

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
            state_type zero_state(zero_ranges->score(), level + 1, zero_ranges);
            if (zero_state.get<0>() == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (zero_state.get<0>() == top_state.get<0>() || (top_queue.empty() && (state_heap.empty() || !(zero_state < state_heap.get_max()))))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_state);
                }
                else
                {
                    results.push_back(std::make_pair(zero_state.get<0>(), zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                state_heap.insert(zero_state);
            }
        }

        if (one_ranges)
        {
            state_type one_state(one_ranges->score(), level + 1, one_ranges);
            if (one_state.get<0>() == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (one_state.get<0>() == top_state.get<0>() || (top_queue.empty() && (state_heap.empty() || !(one_state < state_heap.get_max()))))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_state);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(top_queue.front().get<2>());
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + state_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(state_heap.get_min().get<2>());
                        state_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(one_state.get<0>(), one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() + state_heap.size() < max_queue_size)
            {
                state_heap.insert(one_state);
            }
            else if (top_queue.size() == max_queue_size)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (state_heap.get_min() < one_state)
            {
                recyc_queue.push_back(state_heap.get_min().get<2>());
                state_heap.replace_min(one_state);
            }
            else
            {
                recyc_queue.push_back(one_ranges);
            }
        }

        recyc_queue.push_back(top_ranges);
    }
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::topKUnionWithFilters(
        const filter_list_type &filters,
        const range_list_type &patterns,
        const head_list_type &synonyms,
        size_t thres,
        size_t topK,
        std::vector<std::pair<double, char_type> > &results,
        boost::auto_alloc& alloc) const
{
    if (topK == 0) return;

    size_t max_queue_size = std::max(topK, DEFAULT_TOP_K);

    FilteredSynonymPatternList<self_type> *top_ranges = BOOST_NEW(alloc, FilteredSynonymPatternList<self_type>)((char_type)0, nodes_[0], filters, patterns, synonyms, alloc);
    float score = top_ranges->score();
    if (score == 0.0f)
    {
        //delete top_ranges;
        return;
    }

    typedef boost::tuple<float, uint32_t, FilteredSynonymPatternList<self_type> *> state_type;
    state_type top_state(score, 0U, top_ranges);

    interval_heap<state_type> state_heap(max_queue_size + 1);
    state_heap.insert(top_state);

    results.reserve(topK);

    size_t max_filter_size = 0;
    for (size_t i = 0; i < filters.size(); ++i)
    {
        max_filter_size = std::max(max_filter_size, filters[i]->segments_.size());
    }

    std::vector<FilteredSynonymPatternList<self_type> *, boost::stl_allocator<FilteredSynonymPatternList<self_type> *> > recyc_queue(alloc);
    recyc_queue.reserve(max_queue_size + 1);
    std::deque<state_type, boost::stl_allocator<state_type> > top_queue(alloc);

    FilteredSynonymPatternList<self_type> *zero_ranges, *one_ranges;
    FilterItem<self_type> *zero_filter = NULL, *one_filter = NULL;

    while (results.size() < topK)
    {
        if (!top_queue.empty())
        {
            top_state = top_queue.back();
            top_queue.pop_back();
        }
        else if (!state_heap.empty())
        {
            top_state = state_heap.get_max();
            state_heap.pop_max();
        }
        else
        {
            break;
        }

        top_ranges = boost::get<2>(top_state);

        if (!top_ranges->node_)
        {
            results.push_back(std::make_pair(boost::get<0>(top_state), top_ranges->sym_));
            recyc_queue.push_back(top_ranges);
            continue;
        }

        size_t level = boost::get<1>(top_state);
        const node_type *node = top_ranges->node_;

        if (recyc_queue.empty())
        {
            zero_ranges = BOOST_NEW(alloc, FilteredSynonymPatternList<self_type>)(top_ranges->sym_, node->left_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            zero_ranges = recyc_queue.back();
            zero_ranges->reset(top_ranges->sym_, node->left_);
            recyc_queue.pop_back();
        }

        if (recyc_queue.empty())
        {
            one_ranges = BOOST_NEW(alloc, FilteredSynonymPatternList<self_type>)(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_, top_ranges->filters_.capacity(), top_ranges->patterns_.capacity(), top_ranges->synonyms_.capacity(), alloc);
        }
        else
        {
            one_ranges = recyc_queue.back();
            one_ranges->reset(top_ranges->sym_ | (char_type)1 << (this->alphabet_bit_num_ - level - 1), node->right_);
            recyc_queue.pop_back();
        }

        for (typename filter_list_type::const_iterator it = top_ranges->filters_.begin();
                it != top_ranges->filters_.end(); ++it)
        {
            size_t zero_end = (*it)->tree_->zero_counts_[level];
            node = (*it)->node_;

            if (zero_ranges)
            {
                zero_filter = zero_ranges->getFilter((*it)->tree_, node->left_, max_filter_size);
            }
            if (one_ranges)
            {
                one_filter = one_ranges->getFilter((*it)->tree_, node->right_, max_filter_size);
            }

            for (range_list_type::const_iterator fit = (*it)->segments_.begin();
                    fit != (*it)->segments_.end(); ++fit)
            {
                size_t rank_start = node->rank1(fit->get<0>());
                size_t rank_end = node->rank1(fit->get<1>());

                if (zero_ranges)
                {
                    zero_filter->addSegment(boost::make_tuple(fit->get<0>() - rank_start, fit->get<1>() - rank_end, fit->get<2>()));
                }
                if (one_ranges)
                {
                    one_filter->addSegment(boost::make_tuple(rank_start + zero_end, rank_end + zero_end, fit->get<2>()));
                }
            }

            if (zero_ranges && !zero_ranges->addFilter(zero_filter))
            {
                recyc_queue.push_back(zero_ranges);
                zero_ranges = NULL;
                if (!one_ranges) break;
            }
            if (one_ranges && !one_ranges->addFilter(one_filter))
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

        size_t zero_end = zero_counts_[level];
        node = top_ranges->node_;

        const range_list_type &patterns = top_ranges->patterns_;
        const head_list_type &synonyms = top_ranges->synonyms_;

        for (size_t i = 0; i < thres; ++i)
        {
            for (size_t j = synonyms[i]; j < synonyms[i + 1]; ++j)
            {
                size_t rank_start = node->rank1(patterns[j].get<0>());
                size_t rank_end = node->rank1(patterns[j].get<1>());

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
                size_t rank_start = node->rank1(patterns[j].get<0>());
                size_t rank_end = node->rank1(patterns[j].get<1>());

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
            state_type zero_state(zero_ranges->score(), level + 1, zero_ranges);
            if (boost::get<0>(zero_state) == 0.0)
            {
                recyc_queue.push_back(zero_ranges);
            }
            else if (boost::get<0>(zero_state) == boost::get<0>(top_state) || (top_queue.empty() && (state_heap.empty() || !(zero_state < state_heap.get_max()))))
            {
                if (zero_ranges->node_)
                {
                    top_queue.push_back(zero_state);
                }
                else
                {
                    results.push_back(std::make_pair(boost::get<0>(zero_state), zero_ranges->sym_));
                    recyc_queue.push_back(zero_ranges);
                }
            }
            else
            {
                state_heap.insert(zero_state);
            }
        }

        if (one_ranges)
        {
            state_type one_state(one_ranges->score(), level + 1, one_ranges);
            if (boost::get<0>(one_state) == 0.0)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (boost::get<0>(one_state) == boost::get<0>(top_state) || (top_queue.empty() && (state_heap.empty() || !(one_state < state_heap.get_max()))))
            {
                if (one_ranges->node_)
                {
                    top_queue.push_back(one_state);

                    if (top_queue.size() > max_queue_size)
                    {
                        recyc_queue.push_back(boost::get<2>(top_queue.front()));
                        top_queue.pop_front();
                    }
                    else if (top_queue.size() + state_heap.size() > max_queue_size)
                    {
                        recyc_queue.push_back(boost::get<2>(state_heap.get_min()));
                        state_heap.pop_min();
                    }
                }
                else
                {
                    results.push_back(std::make_pair(boost::get<0>(one_state), one_ranges->sym_));
                    recyc_queue.push_back(one_ranges);
                }
            }
            else if (top_queue.size() + state_heap.size() < max_queue_size)
            {
                state_heap.insert(one_state);
            }
            else if (top_queue.size() == max_queue_size)
            {
                recyc_queue.push_back(one_ranges);
            }
            else if (state_heap.get_min() < one_state)
            {
                recyc_queue.push_back(boost::get<2>(state_heap.get_min()));
                state_heap.replace_min(one_state);
            }
            else
            {
                recyc_queue.push_back(one_ranges);
            }
        }

        recyc_queue.push_back(top_ranges);
    }
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::beginOcc(char_type c) const
{
    c = SuccinctUtils::bitReverse(c, this->alphabet_bit_num_);
    if (c <= occ_.size()) return occ_.prefixSum(c);
    return occ_.getSum();
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::endOcc(char_type c) const
{
    c = SuccinctUtils::bitReverse(c, this->alphabet_bit_num_) + 1;
    if (c <= occ_.size()) return occ_.prefixSum(c);
    return occ_.getSum();
}

template <class CharT, class BitmapT>
WaveletTreeNode<BitmapT> *WaveletMatrix<CharT, BitmapT>::getRoot() const
{
    if (nodes_.empty()) return NULL;
    else return nodes_[0];
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::length() const
{
    return nodes_.empty() ? 0 : nodes_[0]->length();
}

template <class CharT, class BitmapT>
size_t WaveletMatrix<CharT, BitmapT>::allocSize() const
{
    size_t sum = sizeof(self_type) + occ_.allocSize() + sizeof(zero_counts_[0]) * zero_counts_.size();
    for (size_t i = 0; i < nodes_.size(); ++i)
        sum += nodes_[i]->allocSize();

    return sum;
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::save(std::ostream &ostr) const
{
    WaveletTree<CharT>::save(ostr);
    occ_.save(ostr);

    ostr.write((const char *)&zero_counts_[0], sizeof(zero_counts_[0]) * zero_counts_.size());

    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        nodes_[i]->save(ostr);
    }
}

template <class CharT, class BitmapT>
void WaveletMatrix<CharT, BitmapT>::load(std::istream &istr)
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
        nodes_[i] = new node_type(this->support_select_);
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
