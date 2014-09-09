#ifndef _FM_INDEX_FM_INDEX_HPP
#define _FM_INDEX_FM_INDEX_HPP

#include "wavelet_tree_huffman.hpp"
#include "wavelet_matrix.hpp"
#include "custom_int.hpp"
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/succinct/sais/sais.hxx>

#include <algorithm>
#include <boost/shared_ptr.hpp>


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
    typedef std::pair<size_t, size_t> MatchRangeT;
    typedef std::vector<MatchRangeT> MatchRangeListT;
    typedef WaveletMatrix<uint32_t, dense::DBitV> docarray_type;
    typedef WaveletTreeHuffman<char_type, rsdic::RSDic> bwt_type;

    FMIndex();
    ~FMIndex();

    void clear();

    void addDoc(const char_type *text, size_t len);
    void swapOrigText(std::vector<char_type> &orig_text);

    void build();
    void reconstructText(const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text) const;

    size_t backwardSearch(const char_type *pattern, size_t len, MatchRangeT &match_range) const;
    size_t longestSuffixMatch(const char_type *patter, size_t len, MatchRangeListT &match_ranges) const;
    size_t length() const;
    //size_t allocSize() const;

    size_t bufferLength() const;
    size_t docCount() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

    void saveOriginalText(std::ostream &ostr) const;
    void loadOriginalText(std::istream &istr);

    void getMatchedDocIdList(const MatchRangeT &match_range, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;

    void getMatchedDocIdList(const MatchRangeListT &match_ranges, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;

    void getTopKDocIdList(
            const MatchRangeListT &raw_range_list,
            const std::vector<double> &score_list,
            size_t thres,
            size_t max_docs,
            std::vector<std::pair<double, uint32_t> > &res_list,
            std::vector<size_t> &doclen_list) const;

    void getDocLenList(const std::vector<uint32_t>& docid_list, std::vector<size_t>& doclen_list) const;

    sdarray::SDArray& getDocDelim()
    {
        return doc_delim_;
    }
    boost::shared_ptr<docarray_type>& getDocArray()
    {
        return doc_array_;
    }

private:
    size_t length_;
    size_t alphabet_num_;

    sdarray::SDArray doc_delim_;
    boost::shared_ptr<docarray_type> doc_array_;
    boost::shared_ptr<bwt_type> bwt_tree_;

    std::vector<char_type> temp_text_;
};

template <class CharT>
FMIndex<CharT>::FMIndex()
    : length_(), alphabet_num_()
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

    bwt_tree_.reset();
    doc_array_.reset();
    std::vector<char_type>().swap(temp_text_);
}

template <class CharT>
void FMIndex<CharT>::addDoc(const char_type *text, size_t len)
{
    temp_text_.insert(temp_text_.end(), text, text + len);
    temp_text_.push_back(DOC_DELIM);
}

template <class CharT>
void FMIndex<CharT>::swapOrigText(std::vector<char_type> &orig_text)
{
    temp_text_.swap(orig_text);
}

template <class CharT>
size_t FMIndex<CharT>::docCount() const
{
    return doc_delim_.size();
}

template <class CharT>
void FMIndex<CharT>::build()
{
    if (temp_text_.empty())
    {
        std::cout << "empty text list!" << std::endl;
        clear();
        return;
    }
    temp_text_.push_back('\0');
    length_ = temp_text_.size();
    alphabet_num_ = WaveletTree<char_type>::getAlphabetNum(&temp_text_[0], length_);

    std::vector<int40_t> sa(length_);
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
            da[i] = 0;
        }
        else
        {
            bwt[i] = temp_text_[sa[i] - 1];
            da[i] = doc_delim_.find(sa[i]);
        }
    }

    std::vector<char_type>().swap(temp_text_);

    bwt_tree_.reset(new bwt_type(alphabet_num_, false));
    bwt_tree_->build(&bwt[0], length_);

    std::vector<char_type>().swap(bwt);

    doc_array_.reset(new docarray_type(docCount(), false));
    doc_array_->build(da, length_);

    std::vector<int40_t>().swap(sa);

    --length_;
}

template <class CharT>
void FMIndex<CharT>::reconstructText(const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text) const
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
size_t FMIndex<CharT>::backwardSearch(const char_type *pattern, size_t len, MatchRangeT &match_range) const
{
    if (len == 0 || !bwt_tree_) return 0;

    size_t orig_len = len;

    char_type c = pattern[--len];
    size_t occ;

    size_t sp = bwt_tree_->beginOcc(c);
    size_t ep = bwt_tree_->endOcc(c);
    if (sp == ep) return 0;

    match_range.first = sp;
    match_range.second = ep;

    for (; len > 0; --len)
    {
        c = pattern[len - 1];
        occ = bwt_tree_->beginOcc(c);

        sp = occ + bwt_tree_->rank(c, sp);
        ep = occ + bwt_tree_->rank(c, ep);
        if (sp == ep) break;

        match_range.first = sp;
        match_range.second = ep;
    }

    return orig_len - len;
}

template <class CharT>
size_t FMIndex<CharT>::longestSuffixMatch(const char_type *pattern, size_t len, MatchRangeListT &match_ranges) const
{
    if (len == 0 || !bwt_tree_) return 0;

    std::pair<size_t, size_t> match_range;
    std::vector<std::pair<size_t, size_t> > prune_bounds(len);

    size_t max_match = 0;
    char_type c;
    size_t occ, sp, ep;
    size_t i, j;

    for (i = len; i > max_match; --i)
    {
        c = pattern[i - 1];

        sp = bwt_tree_->beginOcc(c);
        ep = bwt_tree_->endOcc(c);

        if (ep - sp <= prune_bounds[i - 1].second - prune_bounds[i - 1].first)
            goto PRUNED;

        match_range.first = sp;
        match_range.second = ep;
        prune_bounds[i - 1] = match_range;

        for (j = i - 1; j > 0; --j)
        {
            c = pattern[j - 1];
            occ = bwt_tree_->beginOcc(c);

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
size_t FMIndex<CharT>::length() const
{
    return length_;
}

//template <class CharT>
//size_t FMIndex<CharT>::allocSize() const
//{
//    return sizeof(FMIndex)
//        + doc_delim_.allocSize() - sizeof(sdarray::SDArray)
//        + bwt_tree_->allocSize() + doc_array_->allocSize();
//}

template <class CharT>
size_t FMIndex<CharT>::bufferLength() const
{
    return temp_text_.size();
}

template <class CharT>
void FMIndex<CharT>::save(std::ostream &ostr) const
{
    ostr.write((const char *)&length_,       sizeof(length_));
    ostr.write((const char *)&alphabet_num_, sizeof(alphabet_num_));

    if (bwt_tree_)
    {
        assert(length_ > 0 && alphabet_num_ > 0);
        bwt_tree_->save(ostr);
    }

    doc_delim_.save(ostr);
    // the doc array may be managed by FMDocArrayMgr, so
    // need check whether the doc array is valid.
    assert((doc_delim_.size() == 0 && !doc_array_) ||
            (doc_delim_.size() > 0 && doc_array_));

    if (doc_array_)
        doc_array_->save(ostr);
}

template <class CharT>
void FMIndex<CharT>::load(std::istream &istr)
{
    istr.read((char *)&length_,       sizeof(length_));
    istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));

    if (length_ > 0 && alphabet_num_ > 0)
    {
        bwt_tree_.reset(new bwt_type(alphabet_num_, false));
        bwt_tree_->load(istr);
    }
    doc_delim_.load(istr);

    if (docCount() > 0)
    {
        doc_array_.reset(new docarray_type(docCount(), false));
        doc_array_->load(istr);
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
void FMIndex<CharT>::getMatchedDocIdList(
    const MatchRangeT &match_range,
    size_t max_docs,
    std::vector<uint32_t> &docid_list,
    std::vector<size_t> &doclen_list) const
{
    if (!doc_array_ || docCount() == 0)
        return;

    std::vector<std::pair<size_t, size_t> > ranges;
    ranges.push_back(match_range);

    doc_array_->intersect(ranges, 1, max_docs, docid_list);

    doclen_list.resize(docid_list.size());
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        if (docid_list[i] >= doc_delim_.size())
        {
            std::cout << "docid is invalid : %d " << docid_list[i] << std::endl;
            continue;
        }
        doclen_list[i] = doc_delim_.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
void FMIndex<CharT>::getMatchedDocIdList(
    const MatchRangeListT &match_ranges,
    size_t max_docs, std::vector<uint32_t> &docid_list,
    std::vector<size_t> &doclen_list) const
{
    if (!doc_array_ || docCount() == 0)
        return;

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
        if (docid_list[i] >= doc_delim_.size())
        {
            std::cout << "docid is invalid : %d " << docid_list[i] << std::endl;
            continue;
        }
        doclen_list[i] = doc_delim_.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
void FMIndex<CharT>::getTopKDocIdList(
    const MatchRangeListT &raw_range_list,
    const std::vector<double> &score_list,
    size_t thres,
    size_t max_docs,
    std::vector<std::pair<double, uint32_t> > &res_list,
    std::vector<size_t> &doclen_list) const
{
    if (!doc_array_ || docCount() == 0)
        return;

    boost::auto_alloc alloc;
    range_list_type range_list(alloc);
    range_list.resize(raw_range_list.size());

    for (size_t i = 0; i < raw_range_list.size(); ++i)
    {
        range_list[i].get<0>() = raw_range_list[i].first;
        range_list[i].get<1>() = raw_range_list[i].second;
        range_list[i].get<2>() = score_list[i];
    }

    doc_array_->topKUnion(range_list, thres, max_docs, res_list, alloc);

    doclen_list.resize(res_list.size());
    for (size_t i = 0; i < res_list.size(); ++i)
    {
        if (res_list[i].second >= doc_delim_.size())
        {
            std::cout << "docid is invalid : %d " << res_list[i].second << std::endl;
            continue;
        }
        doclen_list[i] = doc_delim_.getVal(res_list[i].second++) - 1;
    }
}

template <class CharT>
void FMIndex<CharT>::getDocLenList(const std::vector<uint32_t>& docid_list, std::vector<size_t>& doclen_list) const
{
    doclen_list.resize(docid_list.size());
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        if (docid_list[i] - 1 >= doc_delim_.size())
        {
            std::cout << "docid is invalid : %d " << docid_list[i] << std::endl;
            continue;
        }
        doclen_list[i] = doc_delim_.getVal(docid_list[i] - 1) - 1;
    }
}

}
}

NS_IZENELIB_AM_END

#endif
