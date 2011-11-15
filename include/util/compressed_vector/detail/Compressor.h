#ifndef IZENELIB_UTIL_COMPRESSED_VECTOR_DETAIL_COMPRESSOR_H
#define IZENELIB_UTIL_COMPRESSED_VECTOR_DETAIL_COMPRESSOR_H

#include <types.h>

#include <ir/index_manager/index/Compressor.h>
#include <ir/index_manager/index/CompressParameters.h>


#define CHUNK_ALLOC_LOWER_LIMIT 128
#define CHUNK_ALLOC_UPPER_LIMIT 524299 // 512k


NS_IZENELIB_UTIL_BEGIN

namespace compressed_vector{ namespace detail{

using namespace izenelib::ir::indexmanager;
typedef izenelib::ir::indexmanager::DocIDCompressor IDCompressor;

#pragma pack(push,1)
struct DataChunk
{
    int32_t size;
    DataChunk* next;
    uint8_t data[1];
};
#pragma pack(pop)


/**************************************************************************************************************
 * ChunkEncoder
 *
 * Assumes that all integers are in sorted
 ***************************************************************************************************************/
class ChunkEncoder
{
public:
    ChunkEncoder(IDCompressor* int_compressor)
        :int_compressor_(int_compressor),num_ints_(0), size_in_bytes_(0), first_int_(0), last_int_(0), last_int_of_last_chunk_(0)
    {
    }

    ~ChunkEncoder()
    {
    }

    void encode(uint32_t* ints, int num_ints)
    {
        num_ints_ = num_ints;
        assert(num_ints>0);
        if(first_int_ == 0) first_int_ = ints[0];
        last_int_ = ints[num_ints - 1];
        pre_process_chunk(ints, num_ints);
        ints[0] -= last_int_of_last_chunk_;
        size_in_bytes_ = int_compressor_->compress(ints, compressed_ints_, num_ints_);		
        last_int_of_last_chunk_ = last_int_;
    }	

    void reset()
    {
        num_ints_ = 0;
        size_in_bytes_ = 0;
        first_int_ = 0;
        last_int_ = 0;
        last_int_of_last_chunk_ = 0;
    }

    uint32_t first_int() const
    {
        return first_int_;
    }

    uint32_t last_int() const
    {
        return last_int_;
    }

    // Returns the total size of the compressed portions of this chunk in words.positions are not included
    int size_in_bytes() const
    {
        return size_in_bytes_;
    }

    int num_ints() const
    {
        return num_ints_;
    }

    const uint32_t* compressed_ints() const
    {
        return compressed_ints_;
    }

    static const int kChunkSize = CHUNK_SIZE;

private:
    void pre_process_chunk(uint32_t* chunk, int size)
    {
        for(int i=size-1; i>0; --i)
        {
            chunk[i] = chunk[i] - chunk[i-1]; 
        }
    }

private:
    IDCompressor* int_compressor_;

    int num_ints_;
    int size_in_bytes_;               // Size of the compressed chunk in bytes.

    uint32_t first_int_;  // Decoded first integer in this chunk.
    uint32_t last_int_;   // Decoded last integer in this chunk.

    uint32_t last_int_of_last_chunk_;  //last integer of last chunk

    // These buffers are used for compression of chunks.
    uint32_t compressed_ints_[CompressedOutBufferUpperbound(kChunkSize)];                     // Array of compressed integers.
};

/*********************************************************************************************************
 * ChunkDecoder
 *
 * Responsible for decoding the data contained within a chunk and maintains some state during list traversal.
 *********************************************************************************************************/
class ChunkDecoder
{
public:
    ChunkDecoder(IDCompressor* int_compressor);

    void reset(const uint32_t* buffer, int num_ints);

    void decode();

    void set_buffer(uint32_t* buffer);
	
    const uint32_t* data() const
    {
        return data_ + curr_offset_;
    }

    void set_curr_offset(int offset)
    {
        curr_offset_ = offset;
    }

    // Returns the number of documents in this chunk.
    int num_ints() const
    {
        return num_ints_;
    }

    int curr_document_offset() const
    {
        return curr_offset_;
    }

    uint32_t prev_decoded_int() const
    {
        return prev_decoded_int_;
    }

    void set_prev_decoded_int(uint32_t v)
    {
        prev_decoded_int_ = v;
    }

private:
    void post_process_chunk(uint32_t* block, int size)
    {
        block[0] += prev_decoded_int_;
    
        for(int i=1; i<size; ++i)
        {
            block[i] = block[i] + block[i-1];
        }
        prev_decoded_int_ = block[size - 1];
    }	

private:
    // Decompressors for various portions of the chunk.
    IDCompressor* int_compressor_;

    int num_ints_;  // The number of documents in this chunk.

    // These buffers are used for decompression of chunks.
    uint32_t* data_;
    // but rather during query processing, if necessary at all.

    int curr_offset_;      // The offset into the 'data_' array of the current document we're processing.
    int prev_offset_;      // The position we last stopped at when updating the 'curr_position_offset_'.
    uint32_t prev_decoded_int_;  // The previously decoded doc id. This is used during query processing when decoding integer gaps.

    const uint32_t* curr_buffer_position_;  // Pointer to the raw data of stuff we have to decode next.
};


}}
NS_IZENELIB_UTIL_END

#endif

