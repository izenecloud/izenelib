#ifndef _FM_INDEX_FM_INDEX_HPP
#define _FM_INDEX_FM_INDEX_HPP

#include "wavelet_tree_huffman.hpp"
#include "wavelet_tree_binary.hpp"
#include "wavelet_matrix.hpp"
#include "custom_int.hpp"
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/succinct/sais/sais.hxx>
#include <am/leveldb/Table.h>

#include <algorithm>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class FMIndex
{
public:
    typedef CharT char_type;
    typedef FMIndex<CharT> self_type;
    typedef std::pair<size_t, size_t> FilterRangeT;
    typedef std::vector<uint32_t> FilterItemT;

    FMIndex();
    ~FMIndex();

    void clear();

    void addDoc(const char_type *text, size_t len);
    void swapOrigText(std::vector<char_type> &orig_text);

    void setFilterList(std::vector<std::vector<FilterItemT> > &filter_list);

    void buildText();
    void buildFilter(size_t filter_id);

    void reconstructText(const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text);

    size_t backwardSearch(const char_type *pattern, size_t len, std::pair<size_t, size_t> &match_range) const;
    size_t longestSuffixMatch(const char_type *patter, size_t len, std::vector<std::pair<size_t, size_t> > &match_ranges) const;

    void getMatchedDocIdList(const std::pair<size_t, size_t> &match_range, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;
    void getMatchedDocIdList(const std::vector<std::pair<size_t, size_t> > &match_ranges, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;

    bool getFilterRange(size_t filter_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const;

    void getTopKDocIdList(
            const std::vector<std::pair<size_t, size_t> > &match_ranges_list,
            const std::vector<double> &max_match_list,
            size_t max_docs,
            std::vector<std::pair<double, uint32_t> > &res_list,
            std::vector<size_t> &doclen_list) const;

    void getTopKDocIdListByFilter(
            const std::vector<size_t> &prop_ids,
            const std::vector<std::vector<std::pair<size_t, size_t> > > &filter_ranges,
            const std::vector<std::pair<size_t, size_t> > &match_ranges_list,
            const std::vector<double> &max_match_list,
            size_t max_docs,
            std::vector<std::pair<double, uint32_t> > &res_list,
            std::vector<size_t> &doclen_list) const;

    size_t length() const;
    size_t allocSize() const;

    size_t bufferLength() const;
    size_t docCount() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

    void saveOriginalText(std::ostream &ostr) const;
    void loadOriginalText(std::istream &istr);

private:
    template <class T>
    WaveletTree<T> *getWaveletTree_(size_t charset_size) const
    {
        if (charset_size <= 65536)
        {
            return new WaveletTreeHuffman<T>(charset_size);
        }
        else
        {
            return new WaveletMatrix<T>(charset_size);
        }
    }

private:
    size_t length_;
    size_t alphabet_num_;

    sdarray::SDArray doc_delim_;
    std::vector<sdarray::SDArray> filter_delim_list_;

    boost::shared_ptr<WaveletTreeHuffman<char_type> > bwt_tree_;
    boost::shared_ptr<WaveletMatrix<uint32_t> > doc_array_;
    std::vector<boost::shared_ptr<WaveletMatrix<uint32_t> > > filter_array_list_;

    std::vector<char_type> temp_text_;
    std::vector<std::vector<FilterItemT> > temp_filter_list_;
};

template <class CharT>
FMIndex<CharT>::FMIndex()
    : length_(), alphabet_num_()
    , bwt_tree_()
    , doc_array_()
{
}

template <class CharT>
FMIndex<CharT>::~FMIndex()
{
}

template <class CharT>
void FMIndex<CharT>::clear()
{
    length_ = 0;
    alphabet_num_ = 0;

    doc_delim_.clear();
    filter_delim_list_.clear();

    bwt_tree_.reset();
    doc_array_.reset();

    filter_array_list_.clear();

    std::vector<char_type>().swap(temp_text_);
    std::vector<std::vector<FilterItemT> >().swap(temp_filter_list_);
}

template <class CharT>
void FMIndex<CharT>::addDoc(const char_type *text, size_t len)
{
    temp_text_.insert(temp_text_.end(), text, text + len);
    temp_text_.push_back(DOC_DELIM);
}

template <class CharT>
void FMIndex<CharT>::setFilterList(std::vector<std::vector<FilterItemT> > &filter_list)
{
    temp_filter_list_.swap(filter_list);
}

template <class CharT>
void FMIndex<CharT>::swapOrigText(std::vector<char_type> &orig_text)
{
    temp_text_.swap(orig_text);
}

template <class CharT>
void FMIndex<CharT>::buildText()
{
    temp_text_.push_back('\0');
    length_ = temp_text_.size();
    alphabet_num_ = WaveletTree<char_type>::getAlphabetNum(&temp_text_[0], length_);

    std::vector<int32_t> sa(length_);
    if (saisxx(temp_text_.begin(), sa.begin(), (int64_t)length_, (int64_t)alphabet_num_) < 0)
    {
        clear();
        return;
    }

    size_t pos = 0;
    while (temp_text_[pos] != DOC_DELIM) ++pos;
    doc_delim_.add(pos + 1);
    for (size_t i = pos + 1; i < length_; ++i)
    {
        if (temp_text_[i] == DOC_DELIM)
        {
            doc_delim_.add(i - pos);
            pos = i;
        }
    }
    doc_delim_.build();

    std::vector<char_type> bwt(length_);
    uint32_t *da = (uint32_t *)&sa[0];
    for (size_t i = 0; i < length_; ++i)
    {
        if (sa[i] == 0)
        {
            bwt[i] = temp_text_[length_ - 1];
            da[i] = docCount();
        }
        else
        {
            bwt[i] = temp_text_[sa[i] - 1];
            da[i] = doc_delim_.find(sa[i]);
        }
    }

    std::vector<char_type>().swap(temp_text_);

    bwt_tree_.reset(new WaveletTreeHuffman<char_type>(alphabet_num_));
    bwt_tree_->build(&bwt[0], length_);

    std::vector<char_type>().swap(bwt);

    doc_array_.reset(new WaveletMatrix<uint32_t>(docCount()));
    doc_array_->build(da, length_);

    std::vector<int32_t>().swap(sa);

    --length_;
}

template <class CharT>
void FMIndex<CharT>::buildFilter(size_t filter_id)
{
    filter_delim_list_.resize(temp_filter_list_.size());
    filter_array_list_.resize(temp_filter_list_.size());
    for (size_t i = 0; i < temp_filter_list_.size(); ++i)
    {
        std::vector<uint32_t> temp_docid_list;
        filter_delim_list_[i].clear();
        for (size_t j = 0; j < temp_filter_list_[i].size(); ++j)
        {
            FilterItemT &item = temp_filter_list_[i][j];
            temp_docid_list.insert(temp_docid_list.end(), item.begin(), item.end());
            filter_delim_list_[i].add(item.size());
        }
        filter_delim_list_[i].build();

        for (size_t j = 0; j < temp_docid_list.size(); ++j)
        {
            --temp_docid_list[j];
        }

        std::cout << "filter " << i << " total length: " << filter_delim_list_[i].getSum() << std::endl;
        WaveletMatrix<uint32_t> *temp_wlt = new WaveletMatrix<uint32_t>(docCount());
        temp_wlt->build(&temp_docid_list[0], temp_docid_list.size());
        filter_array_list_[i].reset(temp_wlt);
    }
    std::vector<FilterItemT>().swap(temp_filter_list_[filter_id]);
}

template <class CharT>
void FMIndex<CharT>::reconstructText(const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text)
{
    if (del_docid_list.empty() || del_docid_list[0] > doc_delim_.size()) return;

    size_t old_pos = doc_delim_.prefixSum(del_docid_list[0] - 1);
    size_t new_pos = doc_delim_.prefixSum(del_docid_list[0]) - 1;
    size_t pos = 0;

    for (size_t i = 1; i < del_docid_list.size() && del_docid_list[i] <= doc_delim_.size(); ++i)
    {
        pos = doc_delim_.prefixSum(del_docid_list[i] - 1);
        if (old_pos == new_pos)
        {
            old_pos = pos;
        }
        else
        {
            for (; new_pos < pos; ++old_pos, ++new_pos)
            {
                orig_text[old_pos] = orig_text[new_pos];
            }
        }
        new_pos = doc_delim_.prefixSum(del_docid_list[i]) - 1;
    }

    if (old_pos != new_pos)
    {
        for (; new_pos < length_; ++old_pos, ++new_pos)
        {
            orig_text[old_pos] = orig_text[new_pos];
        }
        orig_text.resize(old_pos);
    }
}

template <class CharT>
bool FMIndex<CharT>::getFilterRange(size_t filter_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const
{
    if (filter_id >= filter_delim_list_.size() || filter_id_range.second > filter_delim_list_[filter_id].size())
        return false;

    match_range.first = filter_delim_list_[filter_id].prefixSum(filter_id_range.first);
    match_range.second = filter_delim_list_[filter_id].prefixSum(filter_id_range.second);
    return true;
}

template <class CharT>
size_t FMIndex<CharT>::backwardSearch(const char_type *pattern, size_t len, std::pair<size_t, size_t> &match_range) const
{
    if (len == 0) return 0;

    size_t orig_len = len;

    char_type c = pattern[--len];
    size_t occ;

    size_t sp = bwt_tree_->getOcc(c);
    size_t ep = bwt_tree_->getOcc(c + 1);
    if (sp == ep) return 0;

    match_range.first = sp;
    match_range.second = ep;

    for (; len > 0; --len)
    {
        c = pattern[len - 1];
        occ = bwt_tree_->getOcc(c);

        sp = occ + bwt_tree_->rank(c, sp);
        ep = occ + bwt_tree_->rank(c, ep);
        if (sp == ep) break;

        match_range.first = sp;
        match_range.second = ep;
    }

    return orig_len - len;
}

template <class CharT>
size_t FMIndex<CharT>::longestSuffixMatch(const char_type *pattern, size_t len, std::vector<std::pair<size_t, size_t> > &match_ranges) const
{
    if (len == 0) return 0;

    std::pair<size_t, size_t> match_range;
    std::vector<std::pair<size_t, size_t> > prune_bounds(len);

    size_t max_match = 0;
    char_type c;
    size_t occ, sp, ep;
    size_t i, j;

    for (i = len; i > max_match; --i)
    {
        c = pattern[i - 1];

        sp = bwt_tree_->getOcc(c);
        ep = bwt_tree_->getOcc(c + 1);

        if (ep - sp <= prune_bounds[i - 1].second - prune_bounds[i - 1].first)
            goto PRUNED;

        match_range.first = sp;
        match_range.second = ep;
        prune_bounds[i - 1] = match_range;

        for (j = i - 1; j > 0; --j)
        {
            c = pattern[j - 1];
            occ = bwt_tree_->getOcc(c);

            sp = occ + bwt_tree_->rank(c, sp);
            ep = occ + bwt_tree_->rank(c, ep);

            if (sp == ep) break;

            if (ep - sp <= prune_bounds[j - 1].second - prune_bounds[j - 1].first)
                goto PRUNED;

            match_range.first = sp;
            match_range.second = ep;
            prune_bounds[j - 1] = match_range;
        }

        if (max_match < i - j)
        {
            max_match = i - j;
            match_ranges.clear();
            match_ranges.push_back(match_range);
        }
        else if (max_match == i - j)
        {
            match_ranges.push_back(match_range);
        }

PRUNED:
        assert(true);
    }

    return max_match;
}

template <class CharT>
void FMIndex<CharT>::getMatchedDocIdList(const std::pair<size_t, size_t> &match_range, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const
{
    std::vector<std::pair<size_t, size_t> > ranges;
    ranges.push_back(match_range);

    doc_array_->intersect(ranges, 1, max_docs, docid_list);

    doclen_list.resize(docid_list.size());
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        doclen_list[i] = doc_delim_.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
void FMIndex<CharT>::getMatchedDocIdList(const std::vector<std::pair<size_t, size_t> > &match_ranges, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const
{
    std::vector<std::pair<size_t, size_t> > ranges(1);
    for (size_t i = 0; i < match_ranges.size(); ++i)
    {
        ranges[0] = match_ranges[i];
        doc_array_->intersect(ranges, 1, max_docs, docid_list);

        if (docid_list.size() >= max_docs) break;
    }

    std::sort(docid_list.begin(), docid_list.end());
    docid_list.erase(std::unique(docid_list.begin(), docid_list.end()), docid_list.end());

    doclen_list.resize(docid_list.size());
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        doclen_list[i] = doc_delim_.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
size_t FMIndex<CharT>::length() const
{
    return length_;
}

template <class CharT>
size_t FMIndex<CharT>::allocSize() const
{
    return sizeof(FMIndex)
        + doc_delim_.allocSize() - sizeof(sdarray::SDArray)
        + bwt_tree_->allocSize() + doc_array_->allocSize();
}

template <class CharT>
size_t FMIndex<CharT>::bufferLength() const
{
    return temp_text_.size();
}

template <class CharT>
size_t FMIndex<CharT>::docCount() const
{
    return doc_delim_.size();
}

template <class CharT>
void FMIndex<CharT>::save(std::ostream &ostr) const
{
    ostr.write((const char *)&length_,       sizeof(length_));
    ostr.write((const char *)&alphabet_num_, sizeof(alphabet_num_));

    doc_delim_.save(ostr);

    bwt_tree_->save(ostr);
    doc_array_->save(ostr);

    size_t filter_count = filter_delim_list_.size();
    ostr.write((const char *)&filter_count, sizeof(filter_count));
    for (size_t i = 0; i < filter_count; ++i)
    {
        filter_delim_list_[i].save(ostr);
        filter_array_list_[i]->save(ostr);
    }
}

template <class CharT>
void FMIndex<CharT>::load(std::istream &istr)
{
    istr.read((char *)&length_,       sizeof(length_));
    istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));

    doc_delim_.load(istr);

    bwt_tree_.reset(new WaveletTreeHuffman<char_type>(alphabet_num_));
    bwt_tree_->load(istr);
    doc_array_.reset(new WaveletMatrix<uint32_t>(docCount()));
    doc_array_->load(istr);

    size_t filter_count = 0;
    istr.read((char *)&filter_count, sizeof(filter_count));
    filter_delim_list_.resize(filter_count);
    filter_array_list_.resize(filter_count);
    for (size_t i = 0; i < filter_count; ++i)
    {
        filter_delim_list_[i].load(istr);
        filter_array_list_[i].reset(new WaveletMatrix<uint32_t>(docCount()));
        filter_array_list_[i]->load(istr);
    }
}

template <class CharT>
void FMIndex<CharT>::saveOriginalText(std::ostream &ostr) const
{
    size_t text_len = temp_text_.size();
    ostr.write((const char *)&text_len,      sizeof(text_len));
    ostr.write((const char *)&temp_text_[0], sizeof(temp_text_[0]) * text_len);
}

template <class CharT>
void FMIndex<CharT>::loadOriginalText(std::istream &istr)
{
    size_t text_len = 0;
    istr.read((char *)&text_len,      sizeof(text_len));
    temp_text_.resize(text_len);
    istr.read((char *)&temp_text_[0], sizeof(temp_text_[0]) * text_len);
}

template <class CharT>
void FMIndex<CharT>::getTopKDocIdList(
        const std::vector<std::pair<size_t, size_t> > &match_ranges_list,
        const std::vector<double> &max_match_list,
        size_t max_docs,
        std::vector<std::pair<double, uint32_t> > &res_list,
        std::vector<size_t> &doclen_list) const
{
    std::vector<boost::tuple<size_t, size_t, double> > match_ranges(match_ranges_list.size());
    for (size_t i = 0; i < match_ranges_list.size(); ++i)
    {
        match_ranges[i].get<0>() = match_ranges_list[i].first;
        match_ranges[i].get<1>() = match_ranges_list[i].second;
        match_ranges[i].get<2>() = max_match_list[i];
    }

    doc_array_->topKUnion(match_ranges, max_docs, res_list);

    doclen_list.resize(res_list.size());
    for (size_t i = 0; i < res_list.size(); ++i)
    {
        doclen_list[i] = doc_delim_.getVal(res_list[i].second++) - 1;
    }
}

template <class CharT>
void FMIndex<CharT>::getTopKDocIdListByFilter(
        const std::vector<size_t> &prop_ids,
        const std::vector<std::vector<std::pair<size_t, size_t> > > &filter_ranges,
        const std::vector<std::pair<size_t, size_t> > &match_ranges_list,
        const std::vector<double> &max_match_list,
        size_t max_docs,
        std::vector<std::pair<double, uint32_t> > &res_list,
        std::vector<size_t> &doclen_list) const
{
    std::vector<boost::tuple<size_t, size_t, double> > match_ranges(match_ranges_list.size());
    for (size_t i = 0; i < match_ranges_list.size(); ++i)
    {
        match_ranges[i].get<0>() = match_ranges_list[i].first;
        match_ranges[i].get<1>() = match_ranges_list[i].second;
        match_ranges[i].get<2>() = max_match_list[i];
    }

    if (prop_ids.empty())
    {
        doc_array_->topKUnion(match_ranges, max_docs, res_list);
    }
    else
    {
        std::vector<FilterList<WaveletMatrix<uint32_t> > *> filters(prop_ids.size());
        for (size_t i = 0; i < prop_ids.size(); ++i)
        {
            const WaveletMatrix<uint32_t> *wlt = filter_array_list_[prop_ids[i]].get();
            filters[i] = new FilterList<WaveletMatrix<uint32_t> >(wlt, wlt->getRoot(), filter_ranges[i]);
        }

        doc_array_->topKUnionWithAuxFilters(filters, match_ranges, max_docs, res_list);
    }

    doclen_list.resize(res_list.size());
    for (size_t i = 0; i < res_list.size(); ++i)
    {
        doclen_list[i] = doc_delim_.getVal(res_list[i].second++) - 1;
    }
}

}
}

NS_IZENELIB_AM_END

#endif
