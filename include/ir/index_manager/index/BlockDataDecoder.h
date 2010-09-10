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

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

/**************************************************************************************************************************************************************
 * ChunkDecoder
 *
 * Responsible for decoding the data contained within a chunk and maintains some state during list traversal.
 **************************************************************************************************************************************************************/
class ChunkDecoder
{
public:
    ChunkDecoder();

    void reset(const uint32_t* buffer, int num_docs);

    void decodeDocIds();

    int decodeFrequencies(const uint32_t* compressed_frequencies);

    int decodePositions(const uint32_t* compressed_positions);

    // Update the offset of the properties for the current document.
    // Requires 'frequencies_' to have been already decoded.
    void UpdatePropertiesOffset();

    uint32_t doc_id(int doc_id_idx) const
    {
        return doc_ids_[doc_id_idx];
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

    // Set the offset into the 'doc_ids_' and 'frequencies_' arrays.
    // This allows us to quickly get to the frequency information for the current
    // document being processed as well as speed up finding the next document to process in this chunk.
    void set_curr_document_offset(int doc_offset)
    {
        assert(doc_offset >= 0 && doc_offset >= curr_document_offset_);
        curr_document_offset_ = doc_offset;
    }

    // Returns the number of documents in this chunk.
    int num_docs() const
    {
        return num_docs_;
    }

    // Returns true when the document properties have been decoded.
    bool decoded_properties() const
    {
        return decoded_properties_;
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

    void update_prev_decoded_doc_id(uint32_t doc_id_offset)
    {
        prev_decoded_doc_id_ += doc_id_offset;
    }

    bool decoded() const
    {
        return decoded_;
    }

    void set_decoded(bool decoded)
    {
        decoded_ = decoded;
    }

    // The maximum number of documents that can be contained within a chunk.
    static const int kChunkSize = CHUNK_SIZE;

private:
    int num_docs_;  // The number of documents in this chunk.

    // These buffers are used for decompression of chunks.
    uint32_t doc_ids_[UncompressedOutBufferUpperbound(kChunkSize)];
    uint32_t frequencies_[UncompressedOutBufferUpperbound(kChunkSize)];
    uint32_t* positions_;
    // but rather during query processing, if necessary at all.

    int curr_document_offset_;      // The offset into the 'doc_ids_' array of the current document we're processing.
    int prev_document_offset_;      // The position we last stopped at when updating the 'curr_position_offset_'.
    int curr_position_offset_;      // The offset into the 'positions_' array for the current document being processed.
    uint32_t prev_decoded_doc_id_;  // The previously decoded doc id. This is used during query processing when decoding docID gaps.

    int num_positions_;             // Total number of decoded positions in this list.
    bool decoded_properties_;       // True when we have decoded the document properties (that is, frequencies, positions, etc).
    // Necessary because we don't want to decode the frequencies/contexts/positions until we're certain we actually need them.

    const uint32_t* curr_buffer_position_;  // Pointer to the raw data of stuff we have to decode next.

    bool decoded_;  // True when we have decoded the docIDs.

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

    void init(uint64_t block_num, int starting_chunk, uint32_t* block_data);

    // Decompresses the block header starting at the 'compressed_header' location in memory.
    // The last docIDs and sizes of chunks will be stored internally in this class after decoding,
    // and the memory freed during destruction.
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

    // Returns a pointer to the current chunk decoder, that is, the one currently being traversed.
    ChunkDecoder* curr_chunk_decoder()
    {
        return &curr_chunk_decoder_;
    }

    // Returns true if the current chunk has been decoded (the docIDs were decoded).
    bool curr_chunk_decoded() const
    {
        return curr_chunk_decoder_.decoded();
    }

    // Returns the total number of chunks in this block.
    // This includes any chunks that are not part of the inverted list for this particular term.
    int num_chunks() const
    {
        return num_chunks_;
    }

    // Returns the actual number of chunks stored in this block, that are actually part of the inverted list for this particular term.
    int num_actual_chunks() const
    {
        return num_chunks_ - starting_chunk_;
    }

    // Returns a pointer to the start of the next raw chunk that needs to be decoded.
    const uint32_t* curr_block_data() const
    {
        return curr_block_data_;
    }

    // The index of the starting chunk in this block for the inverted list for this particular term.
    int starting_chunk() const
    {
        return starting_chunk_;
    }

    // Returns the index of the current chunk being traversed.
    int curr_chunk() const
    {
        return curr_chunk_;
    }

    // Updates 'curr_block_data_' to point to the next chunk in the raw data block for decoding.
    void advance_curr_chunk()
    {
        curr_block_data_ += chunk_size(curr_chunk_);
        ++curr_chunk_;
    }

    static const int kBlockSize = BLOCK_SIZE;

private:
    static const int kChunkSizeLowerBound = MIN_COMPRESSED_CHUNK_SIZE;

    // The upper bound on the number of chunk properties in a single block (with upperbounds for proper decompression by various coding policies),
    // calculated by getting the max number of chunks in a block and multiplying by 2 properties per chunk.
    static const int kChunkPropertiesDecompressedUpperbound = UncompressedOutBufferUpperbound(2 * (kBlockSize / kChunkSizeLowerBound));

    // For each chunk in the block corresponding to this current BlockDecoder, holds the last docIDs and sizes,
    // where the size is in words, and a word is sizeof(uint32_t). The last docID is always followed by the size, for every chunk, in this order.
    uint32_t chunk_properties_[kChunkPropertiesDecompressedUpperbound];  // This will require ~11KiB of memory. Alternative is to make dynamic allocations and resize if necessary.

    uint64_t curr_block_num_;                             // The current block number.
    int num_chunks_;                                      // The total number of chunks this block holds, regardless of which list it is.
    int starting_chunk_;                                  // The chunk number of the first chunk in the block of the associated list.
    int curr_chunk_;                                      // The current actual chunk number we're up to within a block.
    uint32_t* curr_block_data_;                           // Points to the start of the next chunk to be decoded.
    ChunkDecoder curr_chunk_decoder_;                     // Decoder for the current chunk we're processing.
    BlockHeadCompressor block_header_decompressor_;       // Decompressor for the block header.
};


}
NS_IZENELIB_IR_END

#endif

