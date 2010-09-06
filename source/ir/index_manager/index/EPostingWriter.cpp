#include <ir/index_manager/index/EPostingWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

const int ChunkEncoder::kChunkSize;      // Initialized in the class definition.

/************************************************************************************************************
 * BlockEncoder
 *
 *************************************************************************************************************/
const int BlockEncoder::kBlockSize;                            // Initialized in the class definition.
const int BlockEncoder::kChunkSizeLowerBound;                  // Initialized in the class definition.
const int BlockEncoder::kChunkPropertiesUpperbound;            // Initialized in the class definition.
const int BlockEncoder::kChunkPropertiesCompressedUpperbound;  // Initialized in the class definition.

BlockEncoder::BlockEncoder(const HeadCompressor& block_header_compressor) :
        block_header_compressor_(block_header_compressor), num_chunks_(0), block_data_offset_(0),
        chunk_properties_uncompressed_size_(UncompressedInBufferUpperbound(kChunkPropertiesUpperbound, block_header_compressor_.block_size())),
        chunk_properties_uncompressed_(new uint32_t[chunk_properties_uncompressed_size_]), chunk_properties_uncompressed_offset_(0),
        chunk_properties_compressed_len_(0), num_block_header_bytes_(0), num_doc_ids_bytes_(0), num_frequency_bytes_(0), num_positions_bytes_(0),
        num_wasted_space_bytes_(0)
{
}

BlockEncoder::~BlockEncoder()
{
    delete[] chunk_properties_uncompressed_;
}

// Returns true when 'chunk' fits into this block, false otherwise.
// First we calculate a crude upper bound on the size of this block if 'chunk' was included.
// If the upper bound indicates it might not fit, we have to compress the header info with 'chunk' included as well,
// in order to see if it actually fits. The upper bound is done as an optimization, so we don't have to compress the block header
// when adding each additional chunk, but only the last few chunks.
bool BlockEncoder::addChunk(const ChunkEncoder& chunk)
{
    uint32_t num_chunks = num_chunks_ + 1;

    // Store the next pair of (last doc_id, chunk size).
    int chunk_last_doc_id_idx = 2 * num_chunks - 2;
    int chunk_size_idx = 2 * num_chunks - 1;
    assert(chunk_last_doc_id_idx < chunk_properties_uncompressed_size_);
    assert(chunk_size_idx < chunk_properties_uncompressed_size_);
    chunk_properties_uncompressed_[chunk_last_doc_id_idx] = chunk.last_doc_id();
    chunk_properties_uncompressed_[chunk_size_idx] = chunk.size();

    const int kNumHeaderInts = 2 * num_chunks;

    int num_chunks_size = sizeof(num_chunks);
    int curr_block_header_size = num_chunks_size + CompressedOutBufferUpperbound((kNumHeaderInts * sizeof(uint32_t)));  // Upper bound with 'chunk' included.
    int curr_block_data_size = (block_data_offset_ + chunk.size()) * sizeof(uint32_t);
    int upper_bound_block_size = curr_block_data_size + curr_block_header_size;

    if (upper_bound_block_size > kBlockSize)
    {
        int curr_chunk_properties_compressed_len = compressHeader(chunk_properties_uncompressed_, chunk_properties_compressed_, kNumHeaderInts);

        if ((curr_block_data_size + num_chunks_size + (curr_chunk_properties_compressed_len * static_cast<int> (sizeof(*chunk_properties_compressed_)))) <= kBlockSize)
        {
            chunk_properties_compressed_len_ = curr_chunk_properties_compressed_len;
        }
        else
        {
            // TODO: Test this condition.
            // When a single chunk cannot fit into a block we have a problem.
            if (num_chunks == 1)
            {
                //GetErrorLogger().Log(string("A single chunk cannot fit into a block. There are two possible solutions.\n")
                    //                 + string("1) Decrease the maximum number of properties per document.\n") + string("2) Increase the block size."), true);
            }

            // Need to recompress the header (without the new chunk properties, since it doesn't fit).
            chunk_properties_compressed_len_ = compressHeader(chunk_properties_uncompressed_, chunk_properties_compressed_, num_chunks_ * 2);
            return false;
        }
    }

    // Chunk fits into this block, so copy chunk data to this block.
    copyChunkData(chunk);
    ++num_chunks_;
    return true;
}

void BlockEncoder::copyChunkData(const ChunkEncoder& chunk)
{
    const int kWordSize = sizeof(uint32_t);
    const int kBlockSizeWords = kBlockSize / kWordSize;
    assert(block_data_offset_ + chunk.size() <= kBlockSizeWords);

    int num_bytes;
    int num_words;

    // DocIDs.
    const uint32_t* doc_ids = chunk.compressed_doc_ids();
    int doc_ids_len = chunk.compressed_doc_ids_len();
    num_bytes = doc_ids_len * sizeof(*doc_ids);
    assert(num_bytes % kWordSize == 0);
    num_words = num_bytes / kWordSize;
    assert(block_data_offset_ + num_words <= kBlockSizeWords);
    if (doc_ids_len != 0)
    {
        memcpy(block_data_ + block_data_offset_, doc_ids, num_bytes);
        block_data_offset_ += num_words;
        num_doc_ids_bytes_ += num_bytes;
    }
    else
    {
        assert(false);  // DocIDs should always be present.
    }

    // Frequencies.
    const uint32_t* frequencies = chunk.compressed_frequencies();
    int frequencies_len = chunk.compressed_frequencies_len();
    num_bytes = frequencies_len * sizeof(*frequencies);
    assert(num_bytes % kWordSize == 0);
    num_words = num_bytes / kWordSize;
    assert(block_data_offset_ + num_words <= kBlockSizeWords);
    if (frequencies_len != 0)
    {
        memcpy(block_data_ + block_data_offset_, frequencies, num_bytes);
        block_data_offset_ += num_words;
        num_frequency_bytes_ += num_bytes;
    }
    else
    {
        assert(false);  // Frequencies should always be present.
    }

    // Positions.
    const uint32_t* positions = chunk.compressed_positions();
    int positions_len = chunk.compressed_positions_len();
    num_bytes = positions_len * sizeof(*positions);
    assert(num_bytes % kWordSize == 0);
    num_words = num_bytes / kWordSize;
    assert(block_data_offset_ + num_words <= kBlockSizeWords);
    if (positions_len != 0)
    {
        memcpy(block_data_ + block_data_offset_, positions, num_bytes);
        block_data_offset_ += num_words;
        num_positions_bytes_ += num_bytes;
    }
    else
    {
        // If we don't have positions, then we're building an index without them.
    }
}

void BlockEncoder::finalize()
{
    const int kNumHeaderInts = 2 * num_chunks_;
    chunk_properties_compressed_len_ = compressHeader(chunk_properties_uncompressed_, chunk_properties_compressed_, kNumHeaderInts);
}

// The 'header' array size needs to be a multiple of the block size used by the block header compressor.
int BlockEncoder::compressHeader(uint32_t* header, uint32_t* output, int header_len)
{
    return block_header_compressor_.compress(header, output, header_len);
}

// 'block_bytes_len' must be block sized.
void BlockEncoder::getBlockBytes(unsigned char* block_bytes, int block_bytes_len)
{
    if (chunk_properties_compressed_len_ == 0)
    {
        finalize();
    }
    assert(chunk_properties_compressed_len_ > 0);
    assert(block_bytes_len == kBlockSize);

    int block_bytes_offset = 0;
    int num_bytes;

    // Number of chunks.
    num_bytes = sizeof(num_chunks_);
    assert(block_bytes_offset + num_bytes <= block_bytes_len);
    memcpy(block_bytes + block_bytes_offset, &num_chunks_, num_bytes);
    block_bytes_offset += num_bytes;
    num_block_header_bytes_ += num_bytes;

    // Block header.
    num_bytes = chunk_properties_compressed_len_ * sizeof(*chunk_properties_compressed_);
    assert(block_bytes_offset + num_bytes <= block_bytes_len);
    memcpy(block_bytes + block_bytes_offset, chunk_properties_compressed_, num_bytes);
    block_bytes_offset += num_bytes;
    num_block_header_bytes_ += num_bytes;

    // Compressed chunk data.
    num_bytes = block_data_offset_ * sizeof(*block_data_);
    assert(block_bytes_offset + num_bytes <= block_bytes_len);
    memcpy(block_bytes + block_bytes_offset, block_data_, num_bytes);
    block_bytes_offset += num_bytes;

    // Fill remaining wasted space with 0s.
    num_bytes = block_bytes_len - block_bytes_offset;
    memset(block_bytes + block_bytes_offset, 0, num_bytes);
    num_wasted_space_bytes_ += num_bytes;
}


}

NS_IZENELIB_IR_END

