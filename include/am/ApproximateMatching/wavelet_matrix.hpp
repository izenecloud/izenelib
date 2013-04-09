#ifndef WAVELET_MATRIX_WAVELET_MATRIX_HPP_
#define WAVELET_MATRIX_WAVELET_MATRIX_HPP_

#include "bit_trie.hpp"
#include <am/succinct/fm-index/wavelet_matrix.hpp>
#include <vector>
using namespace std;
using namespace izenelib::am::succinct::fm_index;
namespace izenelib
{

class WaveletMatrix : public izenelib::am::succinct::fm_index::WaveletMatrix<uint32_t>
{
public:

    WaveletMatrix(size_t alphabet_num, bool support_select, bool dense);

    void QuantileRangeEach(uint32_t begin_pos, uint32_t end_pos, size_t i, uint32_t val,int k,vector<uint32_t>& ret,BitNode* node) const;

    void QuantileRangeAll(uint32_t begin_pos, uint32_t end_pos, vector<uint32_t>& ret,const BitTrie& filter,bool filt=false) const;

    void QuantileRangeEachWithoutFilter(uint32_t begin_pos, uint32_t end_pos, size_t i, uint32_t val,int k,vector<uint32_t>& ret) const;

};

}


#endif // WAVELET_MATRIX_WAVELET_MATRIX_HPP_
