/**
* @file        BlockDataDecoder.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Decompress block Posting data
*/

#ifndef BLOCK_DATA_DECODER_H
#define BLOCK_DATA_DECODER_H

#include <ir/index_manager/utility/system.h>

#include <ir/index_manager/index/Compressor.h>
#include <ir/index_manager/index/CompressParameters.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/utility/Bitset.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/*********************************************************************************************************
 * ChunkDecoder
 *
 * Responsible for decoding the data contained within a chunk and maintains some state during list traversal.
 *********************************************************************************************************/
class ChunkDecoder
{
public:
    ChunkDecoder();

    void reset(const uint32_t* buffer, int num_docs);

    void decodeDocIds();

    void decodeFrequencies(bool computePos = true);

    int decodePositions(const uint32_t* compressed_positions);

    void set_doc_freq_buffer(uint32_t* doc_buffer, uint32_t* freq_buffer);

    void set_pos_buffer(uint32_t* pos_buffer);

    void updatePositionOffset();

    uint32_t doc_id(int doc_id_idx) const
    {
        return doc_ids_[doc_id_idx];
    }

    uint32_t frequencies(int doc_id_idx) const
    {
        return frequencies_[doc_id_idx];
    }

    void set_doc_id(int doc_id_idx, uint32_t doc_id)
    {
        doc_ids_[doc_id_idx] = doc_id;
    }

    uint32_t current_frequency() const
    {
        assert(curr_document_offset_ < num_docs_);
        assert(frequencies_[curr_document_offset_] != 0);
        return frequencies_[curr_document_offset_];
    }

    const uint32_t* current_positions() const
    {
        assert(curr_position_offset_ < num_positions_);
        return positions_ + curr_position_offset_;
    }

    const uint32_t* doc_ids() const
    {
        return doc_ids_ + curr_document_offset_;
    }

    const uint32_t* frequencies() const
    {
        return frequencies_ + curr_document_offset_;
    }

    // Set the offset into the 'doc_ids_' and 'frequencies_' arrays.
    void set_curr_document_offset(int doc_offset)
    {
        curr_document_offset_ = doc_offset;
    }

    void set_curr_position_offset(int pos_offset)
    {
        curr_position_offset_ = pos_offset;
    }

    // Returns the number of documents in this chunk.
    int num_docs() const
    {
        return num_docs_;
    }

    int size_of_positions() const
    {
        return num_positions_;
    }

    int curr_document_offset() const
    {
        return curr_document_offset_;
    }

    int curr_position_offset() const
    {
        return curr_position_offset_;
    }

    uint32_t prev_decoded_doc_id() const
    {
        return prev_decoded_doc_id_;
    }

    void set_prev_decoded_doc_id(uint32_t decoded_doc_id)
    {
        prev_decoded_doc_id_ = decoded_doc_id;
    }

    bool decoded() const
    {
        return decoded_;
    }

    void set_decoded(bool decoded)
    {
        decoded_ = decoded;
        if(!decoded) doc_deleted_ = false;
    }

    bool pos_decoded() const
    {
        return pos_decoded_;
    }

    void set_pos_decoded(bool pos_decoded)
    {
        pos_decoded_ = pos_decoded;
    }

    bool has_deleted_doc() const
    {
        return doc_deleted_;
    }

    uint32_t move_to(uint32_t target, int32_t& currentBufferPointer, bool computePos = false);

    /// deal with deleted documents
    void post_process(Bitset* pDocFilter);

private:
    void post_process_chunk(uint32_t* block, int size)
    {
        block[0] += prev_decoded_doc_id_;

        for(int i=1; i<size; ++i)
        {
            block[i] = block[i] + block[i-1];
        }
        prev_decoded_doc_id_ = block[size - 1];
    }

private:
    int num_docs_;  // The number of documents in this chunk.

    // These buffers are used for decompression of chunks.
    uint32_t* doc_ids_;
    uint32_t* frequencies_;
    uint32_t* positions_;
    // but rather during query processing, if necessary at all.

    int curr_document_offset_;      // The offset into the 'doc_ids_' array of the current document we're processing.
    int prev_document_offset_;      // The position we last stopped at when updating the 'curr_position_offset_'.
    int curr_position_offset_;      // The offset into the 'positions_' array for the current document being processed.
    uint32_t prev_decoded_doc_id_;  // The previously decoded doc id. This is used during query processing when decoding docID gaps.

    int num_positions_;             // Total number of decoded positions in this list.
    // Necessary because we don't want to decode the frequencies/contexts/positions until we're certain we actually need them.

    const uint32_t* curr_buffer_position_;  // Pointer to the raw data of stuff we have to decode next.

    bool decoded_;  // True when we have decoded the docIDs.

    bool pos_decoded_; // True when we have decoded the positions;

    bool doc_deleted_; // True if there are docIDs that are deleted

    // Decompressors for various portions of the chunk.
    DocIDCompressor doc_id_decompressor_;
    TermFreqCompressor frequency_decompressor_;
    TermPosCompressor position_decompressor_;
};

/**************************************************************************************************************************************************************
 * BlockDecoder
 *
 * Responsible for decoding a block from a particular inverted list.
 **************************************************************************************************************************************************************/
class BlockDecoder
{
public:
    BlockDecoder();

    void init(uint64_t block_num, uint32_t* block_data);

    int decodeHeader(uint32_t* compressed_header);

    // Returns the last fully decoded docID of the ('chunk_idx'+1)th chunk in the block.
    uint32_t chunk_last_doc_id(int chunk_idx) const
    {
        int idx = 2 * chunk_idx;
        assert(idx < (num_chunks_ * 2));
        return chunk_properties_[idx];
    }

    // Returns the size of the ('chunk_idx'+1)th chunk in the block in words (where a word is sizeof(uint32_t)).
    uint32_t chunk_size(int chunk_idx) const
    {
        int idx = 2 * (chunk_idx + 1) - 1;
        assert(idx < (num_chunks_ * 2));
        return chunk_properties_[idx];
    }

    // Returns true if the position of current chunk has been loaded from index file.
    bool curr_chunk_pos_loaded(int chunk_idx) const
    {
        return chunk_pos_properties_[chunk_idx];
    }

    void set_curr_chunk_pos_loaded(int chunk_idx)
    {
        chunk_pos_properties_[chunk_idx] = true;
    }

    // Returns true if the current chunk has been decoded (the docIDs were decoded).
    bool curr_chunk_decoded() const
    {
        return chunk_decoder_.decoded();
    }

    // Returns the total number of chunks in this block.
    int num_chunks() const
    {
        return num_chunks_;
    }

    // Returns a pointer to the start of the next raw chunk that needs to be decoded.
    const uint32_t* curr_block_data() const
    {
        return curr_block_data_;
    }

    // Returns the index of the current chunk being traversed.
    int curr_chunk() const
    {
        return curr_chunk_;
    }

    // Updates 'curr_block_data_' to point to the next chunk in the raw data block for decoding.
    void advance_curr_chunk()
    {
        chunk_decoder_.set_prev_decoded_doc_id(chunk_last_doc_id(curr_chunk_));
        curr_block_data_ += chunk_size(curr_chunk_);
        ++curr_chunk_;
    }

private:
    //decompressed block header
    uint32_t chunk_properties_[BLOCK_HEADER_DECOMPRESSED_UPPERBOUND];
    bool chunk_pos_properties_[BLOCK_HEADER_DECOMPRESSED_UPPERBOUND/2];

    uint64_t curr_block_num_; // The current block number.
    int num_chunks_; // The total number of chunks this block holds, regardless of which list it is.
    int curr_chunk_; // The current actual chunk number we're up to within a block.
    uint32_t* curr_block_data_; // Points to the start of the next chunk to be decoded.
    ChunkDecoder chunk_decoder_; // Decoder for the current chunk we're processing.
    BlockHeadCompressor block_header_decompressor_; // Decompressor for the block header.
    friend class BlockPostingReader;
    friend class PostingMerger;
};

}

NS_IZENELIB_IR_END

#endif
