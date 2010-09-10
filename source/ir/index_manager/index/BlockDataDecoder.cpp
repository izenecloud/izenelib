#include <ir/index_manager/index/BlockDataDecoder.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/**************************************************************************************************************************************************************
 * ChunkDecoder
 *
 **************************************************************************************************************************************************************/
const int ChunkDecoder::kChunkSize;      // Initialized in the class definition.

ChunkDecoder::ChunkDecoder() :
        num_docs_(0), curr_document_offset_(-1), prev_document_offset_(0), curr_position_offset_(0), prev_decoded_doc_id_(0), num_positions_(0),
        decoded_properties_(false), curr_buffer_position_(NULL), decoded_(false)
{
}

void ChunkDecoder::reset(const uint32_t* buffer, int num_docs)
{
    num_docs_ = num_docs;
    curr_document_offset_ = -1;  // -1 signals that we haven't had an intersection yet.
    prev_document_offset_ = 0;
    curr_position_offset_ = 0;
    prev_decoded_doc_id_ = 0;
    decoded_properties_ = false;
    curr_buffer_position_ = buffer;
    decoded_ = false;
}

void ChunkDecoder::decodeDocIds()
{
    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = doc_id_decompressor_.decompress(const_cast<uint32_t*> (curr_buffer_position_), doc_ids_, num_docs_);
    curr_buffer_position_ += num_words_consumed;
    decoded_ = true;
}

int ChunkDecoder::decodeFrequencies(const uint32_t* compressed_frequencies)
{
    int num_words_consumed = frequency_decompressor_.decompress(const_cast<uint32_t*> (compressed_frequencies), frequencies_, num_docs_);
    return num_words_consumed;
}

int ChunkDecoder::decodePositions(const uint32_t* compressed_positions)
{
    num_positions_ = 0;
    // Count the number of positions we have in total by summing up all the frequencies.
    for (int i = 0; i < num_docs_; ++i)
    {
        assert(frequencies_[i] != 0);  // This condition indicates a bug in the program.
        num_positions_ += frequencies_[i];
    }

    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = position_decompressor_.decompress(const_cast<uint32_t*> (compressed_positions), positions_, num_positions_);
    return num_words_consumed;
}

void ChunkDecoder::UpdatePropertiesOffset()
{
    assert(decoded_properties_ == true);
    for (int i = prev_document_offset_; i < curr_document_offset_; ++i)
    {
        assert(frequencies_[i] != 0);  // This indicates a bug in the program.
        curr_position_offset_ += frequencies_[i];
    }
    prev_document_offset_ = curr_document_offset_;
}

/**************************************************************************************************************************************************************
 * BlockDecoder
 *
 **************************************************************************************************************************************************************/
const int BlockDecoder::kBlockSize;                              // Initialized in the class definition.
const int BlockDecoder::kChunkSizeLowerBound;                    // Initialized in the class definition.
const int BlockDecoder::kChunkPropertiesDecompressedUpperbound;  // Initialized in the class definition.

BlockDecoder::BlockDecoder() 
	:curr_block_num_(0), num_chunks_(0), starting_chunk_(0), curr_chunk_(0), curr_block_data_(NULL)
{
}

void BlockDecoder::init(uint64_t block_num, int starting_chunk, uint32_t* block_data)
{
    curr_block_num_ = block_num;
    starting_chunk_ = starting_chunk;
    curr_chunk_ = starting_chunk_;
    curr_block_data_ = block_data;

    uint32_t num_chunks;
    memcpy(&num_chunks, curr_block_data_, sizeof(num_chunks));
    num_chunks_ = num_chunks;
    assert(num_chunks_ > 0);
    curr_block_data_ += 1;

    curr_block_data_ += decodeHeader(curr_block_data_);

    // Adjust our block data pointer to point to the starting chunk of this particular list.
    for (int i = 0; i < starting_chunk_; ++i)
    {
        curr_block_data_ += chunk_size(i);
    }

    curr_chunk_decoder_.set_decoded(false);
}

int BlockDecoder::decodeHeader(uint32_t* compressed_header)
{
    int header_len = num_chunks_ * 2;  // Number of integer entries we have to decompress (two integers per chunk).
    if (header_len > kChunkPropertiesDecompressedUpperbound)
        assert(false);

    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = block_header_decompressor_.decompress(compressed_header, chunk_properties_, header_len);
    return num_words_consumed;
}


}
NS_IZENELIB_IR_END

