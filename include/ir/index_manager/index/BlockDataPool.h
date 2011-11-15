/**
* @file        BlockDataPool.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Compressed block Posting data
*/

#ifndef BLOCK_DATA_POOL_H
#define BLOCK_DATA_POOL_H

#include <ir/index_manager/utility/system.h>

#include <ir/index_manager/index/Compressor.h>
#include <ir/index_manager/index/CompressParameters.h>

#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/utility/Utilities.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

#include <boost/shared_ptr.hpp>
NS_IZENELIB_IR_BEGIN

namespace indexmanager{


/**************************************************************************************************************
 * ChunkEncoder
 *
 * Assumes that all docIDs are in sorted
 ***************************************************************************************************************/
class ChunkEncoder
{
public:
    ChunkEncoder()
        :num_docs_(0), size_(0), first_doc_id_(0), last_doc_id_(0), last_doc_id_of_last_chunk_(0)
    {
        curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
        compressed_positions_ = new uint32_t[curr_position_buffer_size_];
    }

    ~ChunkEncoder()
    {
        delete [] compressed_positions_;
    }

    void encode(uint32_t* doc_ids, uint32_t* frequencies, uint32_t* positions, int num_docs)
    {
        num_docs_ = num_docs;
        assert(num_docs>0);
        if(first_doc_id_ == 0) first_doc_id_ = doc_ids[0];
        last_doc_id_ = doc_ids[num_docs - 1];
        pre_process_chunk(doc_ids, num_docs);
        doc_ids[0] -= last_doc_id_of_last_chunk_;
        compressed_doc_ids_len_ = doc_id_compressor_.compress(doc_ids, compressed_doc_ids_, num_docs_);		
        compressed_frequencies_len_ = frequency_compressor_.compress(frequencies, compressed_frequencies_, num_docs_);
        last_doc_id_of_last_chunk_ = last_doc_id_;

        if (positions != NULL)
        {
            int num_pos = 0;
            uint32_t* pos = positions;
            for (int i = 0; i < num_docs_; ++i) 
            {
                pre_process_chunk(pos,frequencies[i]);
                pos += frequencies[i];
            }
            num_pos = pos - positions;
            assert(num_pos>0);

            if(curr_position_buffer_size_ < num_pos)
            {
                delete [] compressed_positions_;
                curr_position_buffer_size_ = num_pos;
                compressed_positions_ = new uint32_t[curr_position_buffer_size_];
            }

            compressed_positions_len_ = position_compressor_.compress(positions, compressed_positions_, num_pos);
            assert(compressed_positions_len_ < curr_position_buffer_size_);
        }

        // Calculate total compressed size of this chunk in words.
        size_ = compressed_doc_ids_len_ + compressed_frequencies_len_;
    }	

    void reset()
    {
        num_docs_ = 0;
        size_ = 0;
        first_doc_id_ = 0;
        last_doc_id_ = 0;
        last_doc_id_of_last_chunk_ = 0;
    }

    uint32_t first_doc_id() const
    {
        return first_doc_id_;
    }

    uint32_t last_doc_id() const
    {
        return last_doc_id_;
    }

    // Returns the total size of the compressed portions of this chunk in words.positions are not included
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
    void pre_process_chunk(uint32_t* chunk, int size)
    {
        for(int i=size-1; i>0; --i)
        {
            chunk[i] = chunk[i] - chunk[i-1]; 
        }
    }

private:
    DocIDCompressor doc_id_compressor_;
    TermFreqCompressor frequency_compressor_;
    TermPosCompressor position_compressor_;

    int num_docs_;           // Number of unique documents included in this chunk.
    int size_;               // Size of the compressed chunk in bytes.

    uint32_t first_doc_id_;  // Decoded first docID in this chunk.
    uint32_t last_doc_id_;   // Decoded last docID in this chunk.

    uint32_t last_doc_id_of_last_chunk_;  //last docID of last chunk

    // These buffers are used for compression of chunks.
    uint32_t compressed_doc_ids_[CompressedOutBufferUpperbound(kChunkSize)];                     // Array of compressed docIDs.
    int compressed_doc_ids_len_;                                                                 // Actual compressed length of docIDs in number of words.
    uint32_t compressed_frequencies_[CompressedOutBufferUpperbound(kChunkSize)];                 // Array of compressed frequencies.
    int compressed_frequencies_len_;                                                             // Actual compressed length of frequencies in number of words.
    uint32_t* compressed_positions_;  // Array of compressed positions.
    int compressed_positions_len_;                                                               // Actual compressed length of positions in number of words.
    int curr_position_buffer_size_;
};

/**************************************************************************************************************
 * ChunkDataPool
 *
 * Data pool to manage compressed chunks
 ***************************************************************************************************************/

#pragma pack(push,1)
struct ChunkData
{
    int32_t size;
    ChunkData* next;
    uint8_t data[1];
};
#pragma pack(pop)

class ChunkDataPool
{
public:
    ChunkDataPool(boost::shared_ptr<MemCache> pMemCache);

    ~ChunkDataPool();
public:
    /* add compressed position data */
    bool addPOSChunk(const ChunkEncoder& chunk);

    /* add compressed did and tf data */
    bool addDFChunk(const ChunkEncoder& chunk);

    /* add all compressed data */
    bool addChunk(const ChunkEncoder& chunk);

    /* write all data contained to disk */
    void write(IndexOutput* pOutput);

    /** get the real size of the list */
    uint32_t getLength();

    /* reset the list for using at next time*/
    void reset();

    /** truncation the tail chunk,let chunk size=real used size of this chunk */
    void truncTailChunk();

private:
    /* add block */
    void add_chunk_();

    /* add chunk length  */
    void add_len_of_len_(uint32_t i);
private:
    boost::shared_ptr<MemCache> pMemCache_;
    ChunkData* pHeadChunk_; ///Posting list header
    ChunkData* pTailChunk_; ///Posting list tail
    uint32_t nTotalSize_; ///Total size
    uint32_t nPosInCurChunk_;
    uint32_t nTotalUsed_; ///Total Unused size

    friend class PostingMerger;
    friend class BlockPostingWriter;
    friend class ChunkPostingWriter;

    static int32_t UPTIGHT_ALLOC_MEMSIZE;
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
    BlockEncoder();

    ~BlockEncoder();

    // Attempts to add 'chunk' to the current block.
    // Returns true if 'chunk' was added, false if 'chunk' did not fit into the block.
    bool addChunk(const ChunkEncoder& chunk);

    void getBlockBytes(unsigned char* block_bytes );

    void reset();

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

    int num_wasted_space_bytes() const
    {
        return num_wasted_space_bytes_;
    }

    int num_doc_ids() const
    {
        return num_doc_ids_;
    }

    static const int kBlockSize = BLOCK_SIZE;

private:
    void copyChunkData(const ChunkEncoder& chunk);

    int compressHeader(uint32_t* header, uint32_t* output, int header_len);

    static const int kChunkSizeLowerBound = MIN_COMPRESSED_CHUNK_SIZE;

    // The upper bound on the number of chunk data in a block,
    static const int kChunkDataUpperbound = kBlockSize / kChunkSizeLowerBound;

    // The upper bound on the number of chunk data in a single block (sized for proper compression for various coding policies).
    static const int kChunkDataCompressedUpperbound = CompressedOutBufferUpperbound(kChunkDataUpperbound);

    BlockHeadCompressor block_header_compressor_;

    uint32_t num_chunks_;  // The number of chunks contained within this block.

    uint32_t block_data_[kBlockSize / sizeof(uint32_t)];  // The compressed chunk data.
    int block_data_offset_;                               // Current offset within the 'block_data_'.

    int chunk_data_uncompressed_size_;    // Size of the 'chunk_data_uncompressed_' buffer.
    uint32_t* chunk_data_uncompressed_;   // Holds the chunk last docIDs and chunk sizes. Needs to be dynamically allocated.
    int chunk_data_uncompressed_offset_;  // Current offset within the 'chunk_data_uncompressed_' buffer.

    uint32_t chunk_data_compressed_[kChunkDataCompressedUpperbound];
    int chunk_data_compressed_len_;  // The current actual size of the compressed chunk data buffer.

    // The breakdown of bytes in this block.
    int num_block_header_bytes_;
    int num_doc_ids_bytes_;
    int num_frequency_bytes_;
    int num_wasted_space_bytes_;

    int num_doc_ids_;

    uint32_t first_doc_id_;
    uint32_t last_doc_id_;

    friend class BlockDataPool;
    friend class BlockPostingWriter;
    friend class PostingMerger;
};

/**************************************************************************************************************
 * BlockDataPool
 *
 * Data pool to manage block data
 ***************************************************************************************************************/

#pragma pack(push,1)
struct BlockData
{
    BlockData* next;
    uint8_t data[1];
};
#pragma pack(pop)

class BlockDataPool
{
public:
    BlockDataPool(boost::shared_ptr<MemCache> pMemCache);

    ~BlockDataPool();
public:
    bool addChunk(const ChunkEncoder& chunk);

    /* write all data contained to disk */
    void write(IndexOutput* pOutput);

    /** get the real size of the list */
    uint32_t getLength();

    /* reset the list for using at next time*/
    void reset();

    uint32_t num_doc_of_curr_block();

private:
    /* add block */
    void addBlock();

    /* copy compressed data from block encoder to memory pool */
    void copyBlockData();
private:
    BlockEncoder blockEncoder_;
    boost::shared_ptr<MemCache> pMemCache_;
    BlockData* pHeadBlock_; ///Posting list header
    BlockData* pTailBlock_; ///Posting list tail
    uint32_t nTotalSize_; ///Total size
    uint64_t total_num_block_header_bytes_;
    uint64_t total_num_doc_ids_bytes_;
    uint64_t total_num_frequency_bytes_;
    uint64_t total_num_wasted_space_bytes_;
    uint32_t num_doc_of_curr_block_;

    friend class PostingMerger;
    friend class BlockPostingWriter;

    static int32_t UPTIGHT_ALLOC_MEMSIZE;
 };


}
NS_IZENELIB_IR_END

#endif

