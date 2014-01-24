#include <ir/index_manager/index/BlockDataDecoder.h>
//#define DEBUG 1

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/**************************************************************************************************************************************************************
 * ChunkDecoder
 *
 **************************************************************************************************************************************************************/
ChunkDecoder::ChunkDecoder() :
        num_docs_(0), curr_document_offset_(0), prev_document_offset_(0), curr_position_offset_(0), prev_decoded_doc_id_(0), num_positions_(0),
        curr_buffer_position_(NULL), decoded_(false), pos_decoded_(false), doc_deleted_(false)
{
}

void ChunkDecoder::reset(const uint32_t* buffer, int num_docs)
{
    num_docs_ = num_docs;
    curr_document_offset_ = 0;
    prev_document_offset_ = 0;
    curr_position_offset_ = 0;
    curr_buffer_position_ = buffer;
    decoded_ = false;
    pos_decoded_ = false;
}

void ChunkDecoder::set_doc_freq_buffer(uint32_t* doc_buffer, uint32_t* freq_buffer)
{
    doc_ids_ = doc_buffer;
    frequencies_ = freq_buffer;
}

void ChunkDecoder::set_pos_buffer(uint32_t* pos_buffer)
{
    positions_ = pos_buffer;
}

void ChunkDecoder::decodeDocIds()
{
    int num_words_consumed = doc_id_decompressor_.decompress(const_cast<uint32_t*> (curr_buffer_position_), doc_ids_, num_docs_);
    post_process_chunk(doc_ids_, num_docs_);
    curr_buffer_position_ += num_words_consumed;
    decoded_ = true;
#ifdef DEBUG
    cout<<this<<" doc ids: ";
    for (int i = 0; i < num_docs_; ++i)
        cout<<doc_ids_[i]<<",";
    cout<<endl;
#endif
}

void ChunkDecoder::decodeFrequencies(bool computePos)
{
    curr_buffer_position_ += frequency_decompressor_.decompress(const_cast<uint32_t*> (curr_buffer_position_), frequencies_, num_docs_);

    if (computePos)
    {
        num_positions_ = 0;
        for (int i = 0; i < num_docs_; ++i)
        {
            num_positions_ += frequencies_[i];
        }
    }
}

int ChunkDecoder::decodePositions(const uint32_t* compressed_positions)
{
    int num_words_consumed = position_decompressor_.decompress(const_cast<uint32_t*> (compressed_positions), positions_, num_positions_);

    uint32_t* pos = positions_;
    for (int i = 0; i < num_docs_; ++i)
    {
        for (uint32_t j = 1; j < frequencies_[i]; ++j)
        {
            pos[j] += pos[j-1];
        }
        pos += frequencies_[i];
    }
    pos_decoded_ = true;
    return num_words_consumed;
}

void ChunkDecoder::post_process(Bitset* pDocFilter)
{
    assert(num_docs_ > 0);

    if (!pDocFilter->any(doc_ids_[0], doc_ids_[num_docs_ - 1] + 1))
        return;

    uint32_t srcFreq = 0;
    uint32_t destFreq = 0;

    int dest = 0; // copy to the destination
    for (int i = 0; i < num_docs_; ++i)
    {
        if (!pDocFilter->test(doc_ids_[i]))
        {
            // avoid copy if destination is the same to source
            if (dest != i)
            {
                doc_ids_[dest] = doc_ids_[i];
                frequencies_[dest] = frequencies_[i];

                // copy positions
                if (pos_decoded_)
                    memmove(positions_ + destFreq, positions_ + srcFreq, frequencies_[i] * sizeof(uint32_t));
            }

            if (pos_decoded_)
                destFreq += frequencies_[dest];

            ++dest;
        }

        if (pos_decoded_)
            srcFreq += frequencies_[i];
    }

    if (dest < num_docs_)
    {
        doc_deleted_ = true;
        num_docs_ = dest;

        if (pos_decoded_)
            num_positions_ = destFreq;
    }
}

void ChunkDecoder::updatePositionOffset()
{
    curr_position_offset_ = 0;
    for (int i = 0; i < curr_document_offset_; ++i)
    {
        assert(frequencies_[i] != 0);  // This indicates a bug in the program.
        curr_position_offset_ += frequencies_[i];
    }
    prev_document_offset_ = curr_document_offset_;
}

uint32_t ChunkDecoder::move_to(uint32_t target, int32_t& currentBufferPointer, bool computePos)
{
    while (doc_ids_[curr_document_offset_] < target && curr_document_offset_ < num_docs_ - 1)
    {
        ++curr_document_offset_;
    }
    if (computePos)
    {
        curr_position_offset_ = 0;
        for (int i = 0; i < curr_document_offset_; ++i)
        {
            curr_position_offset_ += frequencies_[i];
        }
        prev_document_offset_ = curr_document_offset_;
    }
#ifdef DEBUG
    cout<<this<<"!! move to "<<target<<" "<<doc_ids_[curr_document_offset_]<<" curr_position_offset_ "<<curr_position_offset_<<endl;
#endif
    currentBufferPointer = curr_document_offset_;
    return doc_ids_[curr_document_offset_] >= target ? doc_ids_[curr_document_offset_] : -1;
}

/**********************************************************************************************************
 * BlockDecoder
 **********************************************************************************************************/
BlockDecoder::BlockDecoder() :curr_block_num_(0), num_chunks_(0), curr_chunk_(0), curr_block_data_(NULL)
{
    memset(chunk_properties_,0,BLOCK_HEADER_DECOMPRESSED_UPPERBOUND*sizeof(uint32_t));
    memset(chunk_pos_properties_,0,BLOCK_HEADER_DECOMPRESSED_UPPERBOUND/2*sizeof(bool));
}

void BlockDecoder::init(uint64_t block_num, uint32_t* block_data)
{
    curr_block_num_ = block_num;
    curr_block_data_ = block_data;

    uint32_t num_chunks;
    memcpy(&num_chunks, curr_block_data_, sizeof(num_chunks));
    num_chunks_ = num_chunks;
    curr_block_data_ += 1;
    curr_block_data_ += decodeHeader(curr_block_data_);
    memset(chunk_pos_properties_,0,BLOCK_HEADER_DECOMPRESSED_UPPERBOUND/2*sizeof(bool));

    curr_chunk_ = 0;
    chunk_decoder_.reset(NULL,0);
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
