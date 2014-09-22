#include <util/compressed_vector/VIntVector.h>

NS_IZENELIB_UTIL_BEGIN

namespace compression
{
 const double newpfor_mix_s9_compressor::exceptionThresholdRate_ = 0.1;
 const double newpfor_mix_s9_compressor::exceptionRate_ = 0.05;
}

namespace compressed_vector{

template<>
void VIntVector<false>::push_back(uint32_t val)
{
    add_v_data(val);
}


///Ugly here, because only full specialization is permitted here
///Another solution is to use inheritance
template<>
template<>
void VIntVector<false>::vector_iterator<uint32_t>::increment()
{ 
    if(vector_)
    {
        curr_val_ = read_vint32();
        if(buffer_start_ + buffer_pos_ >= data_len_) vector_ = NULL;
    }
}

template<>
template<>
void VIntVector<false>::vector_iterator<uint32_t const>::increment()
{ 
    if(vector_)
    {
        curr_val_ = read_vint32();
        if(buffer_start_ + buffer_pos_ >= data_len_) vector_ = NULL;
    }
}

}

NS_IZENELIB_UTIL_END

