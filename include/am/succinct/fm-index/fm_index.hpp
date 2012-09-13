#ifndef _FM_INDEX_FM_INDEX_HPP
#define _FM_INDEX_FM_INDEX_HPP

#include "wavelet_tree_huffman.hpp"
#include "wavelet_tree_binary.hpp"
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/sdarray/SDArray.hpp>
#include <am/succinct/sais/sais.hxx>

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
    enum
    {
        HUFFMAN = 0,
        BINARY,
        NA
    };

    typedef CharT char_type;

    FMIndex(uint32_t samplerate = 64);
    ~FMIndex();

    void clear();

    void addDoc(const char_type *text, size_t len);

    void build();
    void reconstructText(const std::vector<uint32_t> &del_docid_list);

    size_t backwardSearch(const char_type *pattern, size_t len, std::pair<size_t, size_t> &match_range) const;
    void getDocIdList(const std::pair<size_t, size_t> &match_range, std::vector<uint32_t> &docid_list) const;

    size_t longestSuffixMatch(const char_type *patter, size_t len, size_t max_docs, std::vector<uint32_t> &docid_list) const;

    size_t length() const;
    size_t allocSize() const;

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    size_t samplerate_;
    size_t length_;
    size_t alphabet_num_;

    sdarray::SDArray doc_delim_;
    rsdic::RSDic sampled_;
    std::vector<uint32_t> positions_;
    WaveletTree<char_type> *bwt_tree_;

    std::vector<char_type> temp_text_;
};

template <class CharT>
FMIndex<CharT>::FMIndex(uint32_t samplerate)
    : samplerate_(samplerate)
    , length_(), alphabet_num_()
    , bwt_tree_()
{
}

template <class CharT>
FMIndex<CharT>::~FMIndex()
{
    if (bwt_tree_) delete bwt_tree_;
}

template <class CharT>
void FMIndex<CharT>::clear()
{
    samplerate_ = 0;
    length_ = 0;
    alphabet_num_ = 0;

    std::vector<uint32_t>().swap(positions_);

    if (bwt_tree_) delete bwt_tree_;
    doc_delim_.clear();

    std::vector<char_type>().swap(temp_text_);
}

template <class CharT>
void FMIndex<CharT>::addDoc(const char_type *text, size_t len)
{
    temp_text_.insert(temp_text_.end(), text, text + len);
    temp_text_.push_back('\n');
}

template <class CharT>
void FMIndex<CharT>::build()
{
    temp_text_.push_back('\0');
    length_ = temp_text_.size();
    alphabet_num_ = WaveletTree<char_type>::getAlphabetNum(&temp_text_[0], length_);

    size_t pos = 0;
    while (temp_text_[pos] != '\n') ++pos;
    doc_delim_.add(pos + 1);
    for (size_t i = pos + 1; i < length_; ++i)
    {
        if (temp_text_[i] == '\n')
        {
            doc_delim_.add(i - pos);
            pos = i;
        }
    }
    doc_delim_.build();

    std::vector<int32_t> sa(length_);
    if (saisxx(temp_text_.begin(), sa.begin(), (int32_t)length_, (int32_t)alphabet_num_) < 0)
    {
        std::vector<char_type>().swap(temp_text_);
        return;
    }

    std::vector<char_type> bwt(length_);
    for (size_t i = 0; i < length_; ++i)
    {
        if (sa[i] == 0)
        {
            bwt[i] = temp_text_[length_ - 1];
        }
        else
        {
            bwt[i] = temp_text_[sa[i] - 1];
        }
    }

    std::vector<char_type>().swap(temp_text_);

    positions_.reserve((length_ + samplerate_ - 1) / samplerate_);
    std::vector<uint64_t> bit_seq((length_ + 63) / 64);
    for (size_t i = 0; i < length_; ++i)
    {
        if (sa[i] % samplerate_ == 0)
        {
            bit_seq[i / 64] |= 1LLU << (i % 64);
            positions_.push_back(sa[i]);
        }
    }
    sampled_.Build(bit_seq, length_);
    std::vector<uint64_t>().swap(bit_seq);

    std::vector<int32_t>().swap(sa);

    if (alphabet_num_ <= 65536)
    {
        bwt_tree_ = new WaveletTreeHuffman<char_type>(alphabet_num_);
    }
    else
    {
        bwt_tree_ = new WaveletTreeBinary<char_type>(alphabet_num_);
    }
    bwt_tree_->build(&bwt[0], length_);

    --length_;
}

template <class CharT>
void FMIndex<CharT>::reconstructText(const std::vector<uint32_t> &del_docid_list)
{
    temp_text_.resize(length_);

    size_t pos = 0;
    char_type c;

    for (size_t i = 0; i < length_; ++i)
    {
        c = bwt_tree_->access(pos, pos);
        temp_text_[length_ - i - 1] = c;
        pos += bwt_tree_->getOcc(c);
    }

    if (del_docid_list.empty()) return;

    size_t old_pos = doc_delim_.prefixSum(del_docid_list[0] - 1);
    size_t new_pos = doc_delim_.prefixSum(del_docid_list[0]) - 1;

    for (size_t i = 1; i < del_docid_list.size(); ++i)
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
                temp_text_[old_pos] = temp_text_[new_pos];
            }
        }
        new_pos = doc_delim_.prefixSum(del_docid_list[i]) - 1;
    }

    if (old_pos != new_pos)
    {
        for (; new_pos < length_; ++old_pos, ++new_pos)
        {
            temp_text_[old_pos] = temp_text_[new_pos];
        }
    }
}

template <class CharT>
size_t FMIndex<CharT>::backwardSearch(const char_type *pattern, size_t len, std::pair<size_t, size_t> &match_range) const
{
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
void FMIndex<CharT>::getDocIdList(const std::pair<size_t, size_t> &match_range, std::vector<uint32_t> &docid_list) const
{
    docid_list.reserve(match_range.second - match_range.first);

    char_type c;
    size_t pos, dist;

    for (size_t i = match_range.first; i < match_range.second; ++i)
    {
        for (pos = i, dist = 0; !sampled_.GetBit(pos); ++dist)
        {
            c = bwt_tree_->access(pos, pos);
            pos += bwt_tree_->getOcc(c);
        }

        docid_list.push_back(doc_delim_.find(positions_[sampled_.Rank1(pos)] + dist) + 1);
        if (docid_list.size() == 100) break;
    }

    std::sort(docid_list.begin(), docid_list.end());
    docid_list.erase(std::unique(docid_list.begin(), docid_list.end()), docid_list.end());

}

template <class CharT>
size_t FMIndex<CharT>::longestSuffixMatch(const char_type *pattern, size_t len, size_t max_docs, std::vector<uint32_t> &docid_list) const
{
    std::vector<std::pair<size_t, size_t> > match_ranges;
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

    size_t pos, dist;

    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = match_ranges.begin();
            it != match_ranges.end(); ++it)
    {
        for (i = it->first; i < it->second; ++i)
        {
            for (pos = i, dist = 0; !sampled_.GetBit(pos); ++dist)
            {
                assert(dist < samplerate_);
                c = bwt_tree_->access(pos, pos);
                pos += bwt_tree_->getOcc(c);
            }

            docid_list.push_back(doc_delim_.find(positions_[sampled_.Rank1(pos)] + dist) + 1);
            if (docid_list.size() == max_docs) goto EXIT;
        }
    }

EXIT:
    std::sort(docid_list.begin(), docid_list.end());
    docid_list.erase(std::unique(docid_list.begin(), docid_list.end()), docid_list.end());

    return max_match;
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
        + sizeof(positions_[0]) * positions_.size()
        + doc_delim_.allocSize() - sizeof(sdarray::SDArray)
        + sampled_.GetUsageBytes()
        + bwt_tree_->allocSize();
}

template <class CharT>
void FMIndex<CharT>::save(std::ostream &ostr) const
{
    ostr.write((const char *)&samplerate_,   sizeof(samplerate_));
    ostr.write((const char *)&length_,       sizeof(length_));
    ostr.write((const char *)&alphabet_num_, sizeof(alphabet_num_));
    ostr.write((const char *)&positions_[0], sizeof(positions_[0]) * positions_.size());

    doc_delim_.save(ostr);
    sampled_.Save(ostr);
    bwt_tree_->save(ostr);
}

template <class CharT>
void FMIndex<CharT>::load(std::istream &istr)
{
    istr.read((char *)&samplerate_,   sizeof(samplerate_));
    istr.read((char *)&length_,       sizeof(length_));
    istr.read((char *)&alphabet_num_, sizeof(alphabet_num_));
    positions_.resize((length_ + samplerate_) / samplerate_);
    istr.read((char *)&positions_[0], sizeof(positions_[0]) * positions_.size());

    doc_delim_.load(istr);
    sampled_.Load(istr);
    if (alphabet_num_ <= 65536)
    {
        bwt_tree_ = new WaveletTreeHuffman<char_type>(alphabet_num_);
    }
    else
    {
        bwt_tree_ = new WaveletTreeBinary<char_type>(alphabet_num_);
    }
    bwt_tree_->load(istr);
}

}
}

NS_IZENELIB_AM_END

#endif
