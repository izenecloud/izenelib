#include <util/compressed_vector/detail/Compressor.h>

NS_IZENELIB_UTIL_BEGIN

namespace compressed_vector{ namespace detail{

/**************************************************************************************************************************************************************
 * ChunkDecoder
 *
 **************************************************************************************************************************************************************/
ChunkDecoder::ChunkDecoder(IDCompressor* int_compressor) 
    :int_compressor_(int_compressor), num_ints_(0), curr_offset_(0), prev_offset_(0), prev_decoded_int_(0),
     curr_buffer_position_(NULL)
{
}

void ChunkDecoder::reset(const uint32_t* buffer, int num_docs)
{
    num_ints_ = num_docs;
    curr_offset_ = 0;
    prev_offset_ = 0;
    curr_buffer_position_ = buffer;
}

void ChunkDecoder::set_buffer(uint32_t* buffer)
{
    data_ = buffer;
}

void ChunkDecoder::decode()
{
    int num_words_consumed = int_compressor_->decompress(const_cast<uint32_t*> (curr_buffer_position_), data_, num_ints_);
    post_process_chunk(data_, num_ints_);
    curr_buffer_position_ += num_words_consumed;
}


}}
NS_IZENELIB_UTIL_END

