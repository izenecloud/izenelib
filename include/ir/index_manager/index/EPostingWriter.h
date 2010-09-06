/**
* @file        EPostingWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief E posting builder
*/
#ifndef E_POSTING_WRITER_H
#define E_POSTING_WRITER_H


#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/index/PostingWriter.h>
#include <ir/index_manager/index/Compressor.h>
#include <ir/index_manager/index/CompressParameters.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/**************************************************************************************************************************************************************
 * ChunkEncoder
 *
 * Assumes that all docIDs are in sorted
 **************************************************************************************************************************************************************/
class ChunkEncoder
{
public:
    ChunkEncoder(uint32_t* doc_ids, uint32_t* frequencies, uint32_t* positions, int num_docs,  uint32_t prev_chunk_last_doc_id, 
                const DIDCompressor& doc_id_compressor, const TFCompressor& frequency_compressor, const PosCompressor& position_compressor)
                :num_docs_(num_docs), size_(0), first_doc_id_(prev_chunk_last_doc_id), last_doc_id_(first_doc_id_),
        compressed_doc_ids_len_(0), compressed_frequencies_len_(0), compressed_positions_len_(0)
    {
        for (int i = 0; i < num_docs_; ++i)
        {
            // The docIDs are d-gap coded. We need to decode them (and add them to the last docID from the previous chunk) to find the last docID in this chunk.
            last_doc_id_ += doc_ids[i];
        }

        compressed_doc_ids_len_ = doc_id_compressor.compress(doc_ids, compressed_doc_ids_, num_docs_);		
        compressed_frequencies_len_ = frequency_compressor.compress(frequencies, compressed_frequencies_, num_docs_);

        if (positions != NULL)
        {
            int num_pos = 0;        
            for (int i = 0; i < num_docs_; ++i) 
                num_pos += frequencies[i];

            compressed_positions_len_ = position_compressor.compress(positions, compressed_positions_, num_pos);
        }

        // Calculate total compressed size of this chunk in words.
        size_ = compressed_doc_ids_len_ + compressed_frequencies_len_ + compressed_positions_len_;
    }	

    uint32_t first_doc_id() const
    {
        return first_doc_id_;
    }

    uint32_t last_doc_id() const
    {
        return last_doc_id_;
    }

    // Returns the total size of the compressed portions of this chunk in words.
    int size() const
    {
        return size_;
    }

    int num_docs() const
    {
        return num_docs_;
    }

    const uint32_t* compressed_doc_ids() const
    {
        return compressed_doc_ids_;
    }

    int compressed_doc_ids_len() const
    {
        return compressed_doc_ids_len_;
    }

    const uint32_t* compressed_frequencies() const
    {
        return compressed_frequencies_;
    }

    int compressed_frequencies_len() const
    {
        return compressed_frequencies_len_;
    }

    const uint32_t* compressed_positions() const
    {
        return compressed_positions_;
    }

    int compressed_positions_len() const
    {
        return compressed_positions_len_;
    }

    static const int kChunkSize = CHUNK_SIZE;

private:
    int num_docs_;           // Number of unique documents included in this chunk.
    int size_;               // Size of the compressed chunk in bytes.

    uint32_t first_doc_id_;  // Decoded first docID in this chunk.
    uint32_t last_doc_id_;   // Decoded last docID in this chunk.

    // These buffers are used for compression of chunks.
    uint32_t compressed_doc_ids_[CompressedOutBufferUpperbound(kChunkSize)];                     // Array of compressed docIDs.
    int compressed_doc_ids_len_;                                                                 // Actual compressed length of docIDs in number of words.
    uint32_t compressed_frequencies_[CompressedOutBufferUpperbound(kChunkSize)];                 // Array of compressed frequencies.
    int compressed_frequencies_len_;                                                             // Actual compressed length of frequencies in number of words.
    uint32_t compressed_positions_[CompressedOutBufferUpperbound(kChunkSize)];  // Array of compressed positions.
    int compressed_positions_len_;                                                               // Actual compressed length of positions in number of words.
};

/**************************************************************************************************************************************************************
 * BlockEncoder
 *
 * Block header format: 4 byte unsigned integer representing the number of chunks in this block,
 * followed by compressed list of chunk sizes and chunk last docIDs.
 **************************************************************************************************************************************************************/
class BlockEncoder
{
public:
    BlockEncoder(const HeadCompressor& block_header_compressor);

    ~BlockEncoder();

    // Attempts to add 'chunk' to the current block.
    // Returns true if 'chunk' was added, false if 'chunk' did not fit into the block.
    bool addChunk(const ChunkEncoder& chunk);

    // Calling this compresses the header.
    // The header will already be compressed if AddChunk() returned false at one point.
    // So it's only to be used in the special case if this block was not filled to capacity.
    void finalize();

    void getBlockBytes(unsigned char* block_bytes, int block_bytes_len);

    uint32_t num_chunks() const
    {
        return num_chunks_;
    }

    int num_block_header_bytes() const
    {
        return num_block_header_bytes_;
    }

    int num_doc_ids_bytes() const
    {
        return num_doc_ids_bytes_;
    }

    int num_frequency_bytes() const
    {
        return num_frequency_bytes_;
    }

    int num_positions_bytes() const
    {
        return num_positions_bytes_;
    }

    int num_wasted_space_bytes() const
    {
        return num_wasted_space_bytes_;
    }

    static const int kBlockSize = BLOCK_SIZE;

private:
    void copyChunkData(const ChunkEncoder& chunk);

    int compressHeader(uint32_t* header, uint32_t* output, int header_len);

    static const int kChunkSizeLowerBound = MIN_COMPRESSED_CHUNK_SIZE;

    // The upper bound on the number of chunk properties in a block,
    // calculated by getting the max number of chunks in a block and multiplying by 2 properties per chunk.
    static const int kChunkPropertiesUpperbound = 2 * (kBlockSize / kChunkSizeLowerBound);

    // The upper bound on the number of chunk properties in a single block (sized for proper compression for various coding policies).
    static const int kChunkPropertiesCompressedUpperbound = CompressedOutBufferUpperbound(kChunkPropertiesUpperbound);

    const HeadCompressor& block_header_compressor_;

    uint32_t num_chunks_;  // The number of chunks contained within this block.

    uint32_t block_data_[kBlockSize / sizeof(uint32_t)];  // The compressed chunk data.
    int block_data_offset_;                               // Current offset within the 'block_data_'.

    int chunk_properties_uncompressed_size_;    // Size of the 'chunk_properties_uncompressed_' buffer.
    uint32_t* chunk_properties_uncompressed_;   // Holds the chunk last docIDs and chunk sizes. Needs to be dynamically allocated.
    int chunk_properties_uncompressed_offset_;  // Current offset within the 'chunk_properties_uncompressed_' buffer.

    // This will require ~22KiB of memory. Alternative is to make dynamic allocations and resize if necessary.
    uint32_t chunk_properties_compressed_[kChunkPropertiesCompressedUpperbound];
    int chunk_properties_compressed_len_;  // The current actual size of the compressed chunk properties buffer.

    // The breakdown of bytes in this block.
    int num_block_header_bytes_;
    int num_doc_ids_bytes_;
    int num_frequency_bytes_;
    int num_positions_bytes_;
    int num_wasted_space_bytes_;
};

}

NS_IZENELIB_IR_END


#endif

