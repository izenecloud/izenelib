#ifndef _FM_INDEX_FM_EXTERNALFILTER_HPP
#define _FM_INDEX_FM_EXTERNALFILTER_HPP

#include "wavelet_tree_huffman.hpp"
#include "wavelet_tree_binary.hpp"
#include "wavelet_matrix.hpp"
#include "custom_int.hpp"
#include <am/succinct/rsdic/RSDic.hpp>
#include <am/succinct/sdarray/SDArray.hpp>
#include <algorithm>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

template <class CharT>
class FMExternalFilter
{
public:
    typedef CharT char_type;
    typedef FMExternalFilter<CharT> self_type;
    typedef std::pair<size_t, size_t> FilterRangeT;
    typedef std::vector<uint32_t> FilterItemT;

    FMExternalFilter();
    ~FMExternalFilter();

    void clear();
    void setAuxFilterList(std::vector<std::vector<FilterItemT> > &filter_list);
    inline void setDocCount(size_t doc_count)
    {
        doc_count_ = doc_count;
    }

    inline size_t filterNum() const
    {
        return filter_array_list_.size();
    }

    void build();
    bool getAuxFilterRange(size_t filter_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const;

    inline WaveletMatrix<uint32_t> *getAuxFilterArray(size_t filter_id) const
    {
        if(filter_id >= filter_array_list_.size())
            return NULL;
        return filter_array_list_[filter_id];
    }

    void save(std::ostream &ostr) const;
    void load(std::istream &istr);

private:
    DISALLOW_COPY_AND_ASSIGN(FMExternalFilter);
    size_t doc_count_;

    std::vector<sdarray::SDArray> aux_filter_delim_list_;
    std::vector<WaveletMatrix<uint32_t> *> filter_array_list_;
    std::vector<std::vector<FilterItemT> > temp_aux_filter_list_;
};

template <class CharT>
FMExternalFilter<CharT>::FMExternalFilter()
    : doc_count_(0) 
{
}

template <class CharT>
FMExternalFilter<CharT>::~FMExternalFilter()
{
    for (size_t i = 0; i < filter_array_list_.size(); ++i)
    {
        delete filter_array_list_[i];
    }
}

template <class CharT>
void FMExternalFilter<CharT>::clear()
{
    doc_count_ = 0;
    aux_filter_delim_list_.clear();
    std::vector<std::vector<FilterItemT> >().swap(temp_aux_filter_list_);

    for (size_t i = 0; i < filter_array_list_.size(); ++i)
    {
        delete filter_array_list_[i];
    }
    filter_array_list_.clear();
}

template <class CharT>
void FMExternalFilter<CharT>::setAuxFilterList(std::vector<std::vector<FilterItemT> > &filter_list)
{
    temp_aux_filter_list_.swap(filter_list);
}

template <class CharT>
void FMExternalFilter<CharT>::build()
{
    aux_filter_delim_list_.resize(temp_aux_filter_list_.size());
    for (size_t i = 0; i < temp_aux_filter_list_.size(); ++i)
    {
        std::vector<uint32_t> temp_docid_list;
        for (size_t j = 0; j < temp_aux_filter_list_[i].size(); ++j)
        {
            FilterItemT &item = temp_aux_filter_list_[i][j];
            temp_docid_list.insert(temp_docid_list.end(), item.begin(), item.end());
            aux_filter_delim_list_[i].add(item.size());
        }
        aux_filter_delim_list_[i].build();

        for (size_t j = 0; j < temp_docid_list.size(); ++j)
        {
            --temp_docid_list[j];
        }

        cout << "aux filter " << i << " total length: " << aux_filter_delim_list_[i].getSum() << endl;
        assert(doc_count_ > 0);
        filter_array_list_.push_back(new WaveletMatrix<uint32_t>(doc_count_));
        filter_array_list_.back()->build(&temp_docid_list[0], temp_docid_list.size());
    }
    std::vector<std::vector<FilterItemT> >().swap(temp_aux_filter_list_);
}

template <class CharT>
bool FMExternalFilter<CharT>::getAuxFilterRange(size_t filter_id, const FilterRangeT &filter_id_range, FilterRangeT &match_range) const
{
    if (filter_id >= aux_filter_delim_list_.size() || filter_id_range.second > aux_filter_delim_list_[filter_id].size())
        return false;
    match_range.first = aux_filter_delim_list_[filter_id].prefixSum(filter_id_range.first);
    match_range.second = aux_filter_delim_list_[filter_id].prefixSum(filter_id_range.second);
    return true;
}

template <class CharT>
void FMExternalFilter<CharT>::save(std::ostream &ostr) const
{
    size_t filter_count = aux_filter_delim_list_.size();
    ostr.write((const char *)&filter_count, sizeof(filter_count));
    for (size_t i = 0; i < filter_count; ++i)
    {
        aux_filter_delim_list_[i].save(ostr);
        filter_array_list_[i]->save(ostr);
    }
}

template <class CharT>
void FMExternalFilter<CharT>::load(std::istream &istr)
{
    size_t filter_count = 0;
    istr.read((char *)&filter_count, sizeof(filter_count));
    aux_filter_delim_list_.resize(filter_count);
    filter_array_list_.resize(filter_count);
    for (size_t i = 0; i < filter_count; ++i)
    {
        assert(doc_count_ > 0);
        aux_filter_delim_list_[i].load(istr);
        filter_array_list_[i] = new WaveletMatrix<uint32_t>(doc_count_);
        filter_array_list_[i]->load(istr);
    }
}

}  // end of namespace fm_index
} // end of namespace succinct

NS_IZENELIB_AM_END

#endif
