#include <ir/index_manager/index/BlockDataDecoder.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/**************************************************************************************************************************************************************
 * ChunkDecoder
 *
 **************************************************************************************************************************************************************/
ChunkDecoder::ChunkDecoder() :
        num_docs_(0), curr_document_offset_(-1), prev_document_offset_(0), curr_position_offset_(0), prev_decoded_doc_id_(0), num_positions_(0),
        curr_buffer_position_(NULL), decoded_(false), doc_deleted_(false)
{
}

void ChunkDecoder::reset(const uint32_t* buffer, int num_docs)
{
    num_docs_ = num_docs;
    curr_document_offset_ = -1;  // -1 signals that we haven't had an intersection yet.
    prev_document_offset_ = 0;
    curr_position_offset_ = 0;
    //prev_decoded_doc_id_ = 0;
    curr_buffer_position_ = buffer;
    decoded_ = false;

    use_internal_buffer_ = true;
    doc_ids_ = internal_doc_ids_buffer_;
    frequencies_ = internal_frequencies_buffer_;
}

void ChunkDecoder::set_doc_freq_buffer(uint32_t* doc_buffer, uint32_t* freq_buffer)
{
    use_internal_buffer_ = false;
    doc_ids_ = doc_buffer;
    frequencies_ = freq_buffer;
}

void ChunkDecoder::set_pos_buffer(uint32_t* pos_buffer)
{
    positions_ = pos_buffer;
}

void ChunkDecoder::decodeDocIds()
{
    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = doc_id_decompressor_.decompress(const_cast<uint32_t*> (curr_buffer_position_), doc_ids_, num_docs_);
    post_process_chunk(doc_ids_, num_docs_);
    curr_buffer_position_ += num_words_consumed;
    decoded_ = true;
}

void ChunkDecoder::decodeFrequencies()
{
    curr_buffer_position_ += frequency_decompressor_.decompress(const_cast<uint32_t*> (curr_buffer_position_), frequencies_, num_docs_);
}

int ChunkDecoder::decodePositions(const uint32_t* compressed_positions)
{
    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = position_decompressor_.decompress(const_cast<uint32_t*> (compressed_positions), positions_, num_positions_);
    return num_words_consumed;
}

void ChunkDecoder::post_process(BitVector* pDocFilter)
{
    int num_doc = num_docs_ - 1;
    if(! pDocFilter->hasBetween(doc_ids_[0], doc_ids_[num_doc]))
        return;

    uint32_t* pPos = positions_;

    for (int i = 0; i < num_doc; ++i)
    {
        if(pDocFilter->test(doc_ids_[i]))
        {
            --num_docs_;
            doc_ids_[i] = doc_ids_[i + 1];
            ///skip positions
            memmove (pPos, pPos + frequencies_[i], frequencies_[i]);
            frequencies_[i] = frequencies_[i + 1];
            pPos += frequencies_[i];
            doc_deleted_ = true;
        }
    }

    if(pDocFilter->test(num_doc - 1))
    {
	--num_docs_;
    }
}

void ChunkDecoder::updatePositionOffset()
{
    for (int i = prev_document_offset_; i < curr_document_offset_; ++i)
    {
        assert(frequencies_[i] != 0);  // This indicates a bug in the program.
        curr_position_offset_ += frequencies_[i];
    }
    prev_document_offset_ = curr_document_offset_;
}

uint32_t ChunkDecoder::move_to(uint32_t target)
{
    while(doc_ids_[curr_document_offset_] >= target)
    {
        ++curr_document_offset_;
    }
    return doc_ids_[curr_document_offset_];
}

/**********************************************************************************************************
 * BlockDecoder
 **********************************************************************************************************/
BlockDecoder::BlockDecoder() :curr_block_num_(0), num_chunks_(0), curr_chunk_(0), curr_block_data_(NULL)
{
    memset(chunk_properties_,0,BLOCK_HEADER_DECOMPRESSED_UPPERBOUND*sizeof(uint32_t));
}

void BlockDecoder::init(uint64_t block_num, uint32_t* block_data)
{
    curr_block_num_ = block_num;
    curr_block_data_ = block_data;

    uint32_t num_chunks;
    memcpy(&num_chunks, curr_block_data_, sizeof(num_chunks));
    num_chunks_ = num_chunks;
    assert(num_chunks_ > 0);
    curr_block_data_ += 1;

    curr_block_data_ += decodeHeader(curr_block_data_);

    chunk_decoder_.set_decoded(false);
}

int BlockDecoder::decodeHeader(uint32_t* compressed_header)
{
    int header_len = num_chunks_ * 2;
    assert((size_t)header_len <= BLOCK_HEADER_DECOMPRESSED_UPPERBOUND);

    // A word is meant to be sizeof(uint32_t) bytes in this context.
    int num_words_consumed = block_header_decompressor_.decompress(compressed_header, chunk_properties_, header_len);
    return num_words_consumed;
}


}
NS_IZENELIB_IR_END

