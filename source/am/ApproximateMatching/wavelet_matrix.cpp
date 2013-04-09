
#include <am/ApproximateMatching/wavelet_matrix.hpp>
using namespace std;
using namespace izenelib::am::succinct::fm_index;
namespace izenelib
{
WaveletMatrix::WaveletMatrix(size_t alphabet_num, bool support_select, bool dense):izenelib::am::succinct::fm_index::WaveletMatrix<uint32_t>(alphabet_num, support_select, dense)
{

}
void WaveletMatrix::QuantileRangeAll(uint32_t begin_pos, uint32_t end_pos, vector<uint32_t>& ret,const BitTrie& filter,bool filt) const
{
    uint32_t val;
    if ((end_pos > length() || begin_pos > end_pos))
    {
        //pos = NOTFOUND;
        //val = NOTFOUND;
        return;
    }

    val = 0;

    size_t i = 0;
    //uint32_t k=0;Schema
    //cout<<"alphabet_bit_num_"<<alphabet_bit_num_<<endl;
    if(filt)
    QuantileRangeEach(begin_pos, end_pos+1, i,val, end_pos - begin_pos+1,ret,filter.Root_);
    else
    QuantileRangeEachWithoutFilter(begin_pos, end_pos+1, i,val, end_pos - begin_pos+1,ret);
    //QuantileRangeEach_NonRecursive(begin_pos, end_pos, i,beg_node ,end_node, val,ret);


}
void WaveletMatrix::QuantileRangeEach(uint32_t begin_pos, uint32_t end_pos, size_t i,uint32_t val,int k,vector<uint32_t>& ret,BitNode* node) const
{

    if(i==this->alphabet_bit_num_)
    {
        ret.push_back(val);
    }
    else
    {
        const WaveletTreeNode* ba = this->nodes_[i];
        uint32_t begin_zero, end_zero;
        begin_zero = ba->rank0(begin_pos);
        end_zero = ba->rank0(end_pos);
        uint32_t zero_bits = end_zero - begin_zero;
        if (zero_bits>0)
        {
            if(node->ZNext_)
                QuantileRangeEach(begin_zero, end_zero, i+1, val  ,zero_bits,ret,node->ZNext_);//
        }
        if (k-zero_bits>0)
        {
            begin_pos += this->zero_counts_[i] - begin_zero;
            end_pos += this->zero_counts_[i] - end_zero;
            if(node->ONext_)
                QuantileRangeEach(begin_pos, end_pos, i+1, val +(1<<i),k-zero_bits, ret,node->ONext_);//
        }

    }

}
void WaveletMatrix::QuantileRangeEachWithoutFilter(uint32_t begin_pos, uint32_t end_pos, size_t i, uint32_t val,int k,vector<uint32_t>& ret) const
{

    if(i==this->alphabet_bit_num_)
    {
        ret.push_back(val);
    }
    else
    {
        const WaveletTreeNode* ba = this->nodes_[i];
        uint32_t begin_zero, end_zero;
        begin_zero = ba->rank0(begin_pos);
        end_zero = ba->rank0(end_pos);
        uint32_t zero_bits = end_zero - begin_zero;
        if (zero_bits>0)
        {

                QuantileRangeEachWithoutFilter(begin_zero, end_zero, i+1, val  ,zero_bits,ret);//
        }
        if (k-zero_bits>0)
        {
            begin_pos += this->zero_counts_[i] - begin_zero;
            end_pos += this->zero_counts_[i] - end_zero;
                QuantileRangeEachWithoutFilter(begin_pos, end_pos, i+1, val +(1<<i),k-zero_bits, ret);//
        }

    }

}
}
