#ifndef _FM_INDEX_FM_DOCARRAYMANAGER_HPP
#define _FM_INDEX_FM_DOCARRAYMANAGER_HPP

#include "wavelet_tree_huffman.hpp"
#include "wavelet_tree_binary.hpp"
#include "wavelet_matrix.hpp"
#include "custom_int.hpp"
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/sdarray/SDArray.hpp>
#include <boost/shared_ptr.hpp>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class FMDocArrayMgr
{
public:
    typedef CharT char_type;
    typedef FMDocArrayMgr<CharT> self_type;
    typedef std::pair<size_t, size_t> FilterRangeT;
    typedef std::vector<FilterRangeT> FilterRangeListT;
    typedef std::vector<uint32_t> FilterItemT;
    typedef WaveletMatrix<uint32_t> DocArrayWaveletT;

    struct DocArrayItemT
    {
        DocArrayItemT()
        {
        }

        void save(std::ostream &ostr) const
        {
            doc_delim.save(ostr);
            if (doc_array_ptr)
                doc_array_ptr->save(ostr);
        }

        void load(std::istream &istr, size_t doc_count)
        {
            doc_delim.load(istr);
            if (doc_delim.size() == 0) return;
            doc_array_ptr.reset(new DocArrayWaveletT(doc_count, false, true));
            doc_array_ptr->load(istr);
        }

        void swap(DocArrayItemT& other)
        {
            doc_delim.swap(other.doc_delim);
            doc_array_ptr.swap(other.doc_array_ptr);
        }

        sdarray::SDArray doc_delim;
        boost::shared_ptr<DocArrayWaveletT> doc_array_ptr;
    };

    FMDocArrayMgr();
    ~FMDocArrayMgr();

    void clear();
    void clearMainDocArray();
    void setFilterList(std::vector<std::vector<FilterItemT> > &filter_list);

    inline size_t getDocCount() const
    {
        return doc_count_;
    }
    inline bool setDocCount(size_t doc_count)
    {
        if (doc_count_ == 0)
        {
            doc_count_ = doc_count;
        }
        else if (doc_count != doc_count_)
        {
            std::cout << "doc_count should be set only once!!" << std::endl;
            return false;
        }
        return true;
    }
    inline size_t filterNum() const
    {
        return filter_docarray_list_.size();
    }

    DocArrayItemT& addMainDocArrayItem()
    {
        main_docarray_list_.resize(main_docarray_list_.size() + 1);
        return main_docarray_list_.back();
    }
    inline size_t mainDocArrayNum() const
    {
        return main_docarray_list_.size();
    }
    void swapMainDocArray(FMDocArrayMgr& other)
    {
        std::swap(doc_count_, other.doc_count_);
        main_docarray_list_.swap(other.main_docarray_list_);
    }

    void swapFilterDocArray(size_t prop_id, FMDocArrayMgr& other)
    {
        if (prop_id >= filter_docarray_list_.size())
            return;
        filter_docarray_list_[prop_id].swap(other.filter_docarray_list_[prop_id]);
    }

    void buildFilter();

    void reconstructText(size_t match_index, const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text) const;

    bool getFilterRange(size_t prop_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const;

    void getDocLenList(const std::vector<uint32_t>& docid_list, std::vector<size_t>& doclen_list) const;
    void getMatchedDocIdList(size_t match_index, bool match_in_filter, const FilterRangeT &match_range, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;

    void getMatchedDocIdList(size_t match_index, bool match_in_filter, const FilterRangeListT &match_ranges, size_t max_docs, std::vector<uint32_t> &docid_list, std::vector<size_t> &doclen_list) const;

    void getTopKDocIdList(
            size_t match_index,
            bool match_in_filter,
            const FilterRangeListT &match_ranges_list,
            const std::vector<double> &max_match_list,
            size_t max_docs,
            std::vector<std::pair<double, uint32_t> > &res_list,
            std::vector<size_t> &doclen_list) const;

    void getTopKDocIdListByFilter(
            const std::vector<size_t> &prop_id_list,
            const std::vector<FilterRangeListT> &filter_ranges,
            size_t match_index,
            bool match_in_filter,
            const FilterRangeListT &match_ranges_list,
            const std::vector<double> &max_match_list,
            size_t max_docs,
            std::vector<std::pair<double, uint32_t> > &res_list) const;

    inline const DocArrayItemT& getFilterArrayItem(size_t prop_id) const
    {
        if (prop_id >= filter_docarray_list_.size())
            throw -1;
        return filter_docarray_list_[prop_id];
    }
    inline const DocArrayItemT& getMainDocArrayItem(size_t prop_id) const
    {
        if (prop_id >= main_docarray_list_.size())
            throw -1;
        return main_docarray_list_[prop_id];
    }
    inline const DocArrayItemT& getDocArrayItem(size_t index, bool isfilter) const
    {
        if (isfilter)
            return getFilterArrayItem(index);
        return getMainDocArrayItem(index);
    }
    inline bool checkItemIndex(size_t index, bool isfilter) const
    {
        return isfilter ? index < filterNum() : index < mainDocArrayNum();
    }

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    DISALLOW_COPY_AND_ASSIGN(FMDocArrayMgr);

    size_t doc_count_;

    std::vector<DocArrayItemT> main_docarray_list_;
    std::vector<DocArrayItemT> filter_docarray_list_;

    std::vector<std::vector<FilterItemT> > temp_filter_list_;
};

template <class CharT>
FMDocArrayMgr<CharT>::FMDocArrayMgr()
    : doc_count_(0)
{
}

template <class CharT>
FMDocArrayMgr<CharT>::~FMDocArrayMgr()
{
}

template <class CharT>
void FMDocArrayMgr<CharT>::clear()
{
    clearMainDocArray();
    filter_docarray_list_.clear();
    std::vector<std::vector<FilterItemT> >().swap(temp_filter_list_);
}

template <class CharT>
void FMDocArrayMgr<CharT>::clearMainDocArray()
{
    doc_count_ = 0;
    main_docarray_list_.clear();
}

template <class CharT>
void FMDocArrayMgr<CharT>::setFilterList(std::vector<std::vector<FilterItemT> > &filter_list)
{
    temp_filter_list_.swap(filter_list);
}

template <class CharT>
void FMDocArrayMgr<CharT>::buildFilter()
{
    filter_docarray_list_.resize(temp_filter_list_.size());
    for (size_t i = 0; i < temp_filter_list_.size(); ++i)
    {
        if (temp_filter_list_[i].size() == 0)
        {
            std::cout << "filter " << i << " is empty!" << std::endl;
            sdarray::SDArray().swap(filter_docarray_list_[i].doc_delim);
            filter_docarray_list_[i].doc_array_ptr.reset();
            continue;
        }
        std::vector<uint32_t> temp_docid_list;
        for (size_t j = 0; j < temp_filter_list_[i].size(); ++j)
        {
            FilterItemT &item = temp_filter_list_[i][j];
            temp_docid_list.insert(temp_docid_list.end(), item.begin(), item.end());
            filter_docarray_list_[i].doc_delim.add(item.size());
        }
        filter_docarray_list_[i].doc_delim.build();

        for (size_t j = 0; j < temp_docid_list.size(); ++j)
        {
            --temp_docid_list[j];
        }

        std::cout << "filter " << i << " total doc_delim length: " << filter_docarray_list_[i].doc_delim.getSum() << std::endl;
        assert(doc_count_ > 0);
        filter_docarray_list_[i].doc_array_ptr.reset(new DocArrayWaveletT(doc_count_, false, true));
        filter_docarray_list_[i].doc_array_ptr->build(&temp_docid_list[0], temp_docid_list.size());
        std::vector<FilterItemT>().swap(temp_filter_list_[i]);
    }
    std::vector<std::vector<FilterItemT> >().swap(temp_filter_list_);
}

template <class CharT>
bool FMDocArrayMgr<CharT>::getFilterRange(size_t prop_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const
{
    if (prop_id >= filter_docarray_list_.size() || filter_id_range.second > filter_docarray_list_[prop_id].doc_delim.size())
        return false;
    match_range.first = filter_docarray_list_[prop_id].doc_delim.prefixSum(filter_id_range.first);
    match_range.second = filter_docarray_list_[prop_id].doc_delim.prefixSum(filter_id_range.second);
    return true;
}

template <class CharT>
void FMDocArrayMgr<CharT>::save(std::ostream &ostr) const
{
    assert(doc_count_ > 0);
    ostr.write((const char*)&doc_count_, sizeof(doc_count_));

    size_t filter_count = filter_docarray_list_.size();
    ostr.write((const char *)&filter_count, sizeof(filter_count));
    for (size_t i = 0; i < filter_count; ++i)
    {
        filter_docarray_list_[i].save(ostr);
    }
    size_t main_count = main_docarray_list_.size();
    ostr.write((const char *)&main_count, sizeof(main_count));
    for (size_t i = 0; i < main_count; ++i)
    {
        assert(main_docarray_list_[i].doc_delim.size() == doc_count_);
        if (main_docarray_list_[i].doc_delim.size() != doc_count_)
        {
            std::cout << "the main doc delim length not match : " << main_docarray_list_[i].doc_delim.size() << std::endl;
            throw -1;
        }
        main_docarray_list_[i].save(ostr);
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::load(std::istream &istr)
{
    istr.read((char *)&doc_count_, sizeof(doc_count_));
    assert(doc_count_ > 0);

    size_t filter_count = 0;
    istr.read((char *)&filter_count, sizeof(filter_count));
    filter_docarray_list_.resize(filter_count);
    for (size_t i = 0; i < filter_count; ++i)
    {
        assert(doc_count_ > 0);
        filter_docarray_list_[i].load(istr, doc_count_);
    }
    size_t main_count = 0;
    istr.read((char *)&main_count, sizeof(main_count));
    main_docarray_list_.resize(main_count);
    for (size_t i = 0; i < main_count; ++i)
    {
        main_docarray_list_[i].load(istr, doc_count_);
        assert(main_docarray_list_[i].doc_delim.size() == doc_count_);
        if (main_docarray_list_[i].doc_delim.size() != doc_count_)
            throw -1;
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::reconstructText(size_t match_index, const std::vector<uint32_t> &del_docid_list, std::vector<char_type> &orig_text) const
{
    if (!checkItemIndex(match_index, false))
    {
        std::cout << "reconstructText main fm index not found" << std::endl;
        return;
    }
    const DocArrayItemT& doc_array_item = getDocArrayItem(match_index, false);

    const sdarray::SDArray doc_delim = doc_array_item.doc_delim;

    if (del_docid_list.empty() || del_docid_list[0] > doc_delim.size()) return;

    size_t old_pos = doc_delim.prefixSum(del_docid_list[0] - 1);
    size_t new_pos = doc_delim.prefixSum(del_docid_list[0]) - 1;
    size_t pos = 0;

    for (size_t i = 1; i < del_docid_list.size() && del_docid_list[i] <= doc_delim.size(); ++i)
    {
        pos = doc_delim.prefixSum(del_docid_list[i] - 1);
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
        new_pos = doc_delim.prefixSum(del_docid_list[i]) - 1;
    }

    size_t fm_length = doc_array_item.doc_array_ptr->length();
    if (old_pos != new_pos)
    {
        for (; new_pos < fm_length; ++old_pos, ++new_pos)
        {
            orig_text[old_pos] = orig_text[new_pos];
        }
        orig_text.resize(old_pos);
    }
}


template <class CharT>
void FMDocArrayMgr<CharT>::getMatchedDocIdList(
        size_t match_index,
        bool match_in_filter,
        const FilterRangeT &match_range,
        size_t max_docs,
        std::vector<uint32_t> &docid_list,
        std::vector<size_t> &doclen_list) const
{
    if (!checkItemIndex(match_index, match_in_filter))
        return;

    const DocArrayItemT& doc_array_item = getDocArrayItem(match_index, match_in_filter);
    std::vector<std::pair<size_t, size_t> > ranges;
    ranges.push_back(match_range);

    if (!doc_array_item.doc_array_ptr)
        return;
    doc_array_item.doc_array_ptr->intersect(ranges, 1, max_docs, docid_list);

    doclen_list.resize(docid_list.size());
    if (match_in_filter)
    {
        // in filter doc array ,the doclen is not available
        return;
    }
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        doclen_list[i] = doc_array_item.doc_delim.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::getMatchedDocIdList(
        size_t match_index,
        bool match_in_filter,
        const FilterRangeListT &match_ranges,
        size_t max_docs, std::vector<uint32_t> &docid_list,
        std::vector<size_t> &doclen_list) const
{
    if (!checkItemIndex(match_index, match_in_filter))
        return;

    const DocArrayItemT& doc_array_item = getDocArrayItem(match_index, match_in_filter);
    if (!doc_array_item.doc_array_ptr)
        return;
    std::vector<std::pair<size_t, size_t> > ranges(1);
    for (size_t i = 0; i < match_ranges.size(); ++i)
    {
        ranges[0] = match_ranges[i];
        doc_array_item.doc_array_ptr->intersect(ranges, 1, max_docs, docid_list);

        if (docid_list.size() >= max_docs) break;
    }

    std::sort(docid_list.begin(), docid_list.end());
    docid_list.erase(std::unique(docid_list.begin(), docid_list.end()), docid_list.end());

    doclen_list.resize(docid_list.size());

    if (match_in_filter)
    {
        // in filter doc array ,the doclen is not available
        return;
    }
    for (size_t i = 0; i < docid_list.size(); ++i)
    {
        doclen_list[i] = doc_array_item.doc_delim.getVal(docid_list[i]++) - 1;
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::getTopKDocIdList(
        size_t match_index,
        bool match_in_filter,
        const FilterRangeListT &match_ranges_list,
        const std::vector<double> &max_match_list,
        size_t max_docs,
        std::vector<std::pair<double, uint32_t> > &res_list,
        std::vector<size_t> &doclen_list) const
{
    if (!checkItemIndex(match_index, match_in_filter))
        return;

    const DocArrayItemT& doc_array_item = getDocArrayItem(match_index, match_in_filter);

    if (!doc_array_item.doc_array_ptr)
        return;
    std::vector<boost::tuple<size_t, size_t, double> > match_ranges(match_ranges_list.size());
    for (size_t i = 0; i < match_ranges_list.size(); ++i)
    {
        match_ranges[i].get<0>() = match_ranges_list[i].first;
        match_ranges[i].get<1>() = match_ranges_list[i].second;
        match_ranges[i].get<2>() = max_match_list[i];
    }

    doc_array_item.doc_array_ptr->topKUnion(match_ranges, max_docs, res_list);

    doclen_list.resize(res_list.size());

    if (match_in_filter)
    {
        // in filter doc array ,the doclen is not available
        return;
    }
    for (size_t i = 0; i < res_list.size(); ++i)
    {
        doclen_list[i] = doc_array_item.doc_delim.getVal(res_list[i].second++) - 1;
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::getTopKDocIdListByFilter(
        const std::vector<size_t> &prop_id_list,
        const std::vector<FilterRangeListT> &filter_ranges,
        size_t match_index,
        bool match_in_filter,
        const FilterRangeListT &match_ranges_list,
        const std::vector<double> &max_match_list,
        size_t max_docs,
        std::vector<std::pair<double, uint32_t> > &res_list) const
{
    if (!checkItemIndex(match_index, match_in_filter))
        return;

    const DocArrayItemT& doc_array_item = getDocArrayItem(match_index, match_in_filter);
    if (!doc_array_item.doc_array_ptr)
        return;
    std::vector<boost::tuple<size_t, size_t, double> > match_ranges(match_ranges_list.size());
    for (size_t i = 0; i < match_ranges_list.size(); ++i)
    {
        match_ranges[i].get<0>() = match_ranges_list[i].first;
        match_ranges[i].get<1>() = match_ranges_list[i].second;
        match_ranges[i].get<2>() = max_match_list[i];
    }
    if (prop_id_list.empty())
    {
        doc_array_item.doc_array_ptr->topKUnion(match_ranges, max_docs, res_list);
    }
    else
    {
        std::vector<FilterList<DocArrayWaveletT> *> aux_filters;
        aux_filters.reserve(prop_id_list.size());
        for (size_t i = 0; i < prop_id_list.size(); ++i)
        {
            if (!checkItemIndex(prop_id_list[i], true))
            {
                std::cout << "filter index out of bound! " << std::endl;
                for (size_t j = 0; j < aux_filters.size(); ++j)
                    delete aux_filters[j];
                return;
            }
            const DocArrayWaveletT *wlt = getDocArrayItem(prop_id_list[i], true).doc_array_ptr.get();
            if (!wlt)
                continue;
            aux_filters.push_back(new FilterList<DocArrayWaveletT>(wlt, wlt->getRoot(), filter_ranges[i]));
        }

        doc_array_item.doc_array_ptr->topKUnionWithAuxFilters(aux_filters, match_ranges, max_docs, res_list);
    }
    for (size_t i = 0; i < res_list.size(); ++i)
    {
        res_list[i].second++;
    }
}

template <class CharT>
void FMDocArrayMgr<CharT>::getDocLenList(const std::vector<uint32_t>& docid_list, std::vector<size_t>& doclen_list) const
{
    doclen_list.resize(docid_list.size());
    for (size_t j = 0; j < main_docarray_list_.size(); ++j)
    {
        const DocArrayItemT& item = getDocArrayItem(j, false);
        for (size_t i = 0; i < docid_list.size(); ++i)
        {
            if (docid_list[i] - 1 >= item.doc_delim.size())
            {
                std::cout << "docid is invalid : %d " << docid_list[i] << std::endl;
                continue;
            }
            doclen_list[i] += item.doc_delim.getVal(docid_list[i] - 1) - 1;
        }
    }
}


}  // end of namespace fm_index
} // end of namespace succinct

NS_IZENELIB_AM_END

#endif
