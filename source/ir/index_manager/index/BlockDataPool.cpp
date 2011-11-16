#include <ir/index_manager/index/BlockDataPool.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

const int ChunkEncoder::kChunkSize;      // Initialized in the class definition.

/************************************************************************************************************
 * ChunkDataPool
 *
 *************************************************************************************************************/

int32_t ChunkDataPool::UPTIGHT_ALLOC_MEMSIZE = 10*1024*1024;

ChunkDataPool::ChunkDataPool(boost::shared_ptr<MemCache> pMemCache)
    :pMemCache_(pMemCache)
    ,pHeadChunk_(NULL)
    ,pTailChunk_(NULL)
    ,nTotalSize_(0)
    ,nPosInCurChunk_(0)
    ,nTotalUsed_(0)
{
}

ChunkDataPool::~ChunkDataPool()
{
}

bool ChunkDataPool::addDFChunk(const ChunkEncoder& chunk)
{
    if (pTailChunk_ == NULL)
    {
        add_chunk_();
        return addDFChunk(chunk);
    }

    // Docids.
    const uint32_t* doc_ids = chunk.compressed_doc_ids();
    int doc_ids_len = chunk.compressed_doc_ids_len() * sizeof(uint32_t);

    // Frequencies.
    const uint32_t* frequencies = chunk.compressed_frequencies();
    int frequencies_len = chunk.compressed_frequencies_len() * sizeof(uint32_t);

    uint8_t len_of_docid_len = IndexOutput::getVIntLength(doc_ids_len);
    uint8_t len_of_freq_len = IndexOutput::getVIntLength(frequencies_len);

    int left = pTailChunk_->size - nPosInCurChunk_;

    if (left < (doc_ids_len + len_of_docid_len + frequencies_len + len_of_freq_len))
    {
        pTailChunk_->size = nPosInCurChunk_;///the real size
        add_chunk_();
        return addDFChunk(chunk);
    }

    {
        add_len_of_len_(doc_ids_len);
        memcpy(pTailChunk_->data + nPosInCurChunk_, doc_ids, doc_ids_len);
        nTotalUsed_ += doc_ids_len;
        nPosInCurChunk_ += doc_ids_len;
    }

    {
        add_len_of_len_(frequencies_len);
        memcpy(pTailChunk_->data + nPosInCurChunk_, frequencies, frequencies_len);
        nTotalUsed_ += frequencies_len;
        nPosInCurChunk_ += frequencies_len;
    }
    return true;
}

bool ChunkDataPool::addPOSChunk(const ChunkEncoder& chunk)
{
    if (pTailChunk_ == NULL)
    {
        add_chunk_();
        return addPOSChunk(chunk);
    }

    // Positions.
    const uint32_t* positions = chunk.compressed_positions();
    int positions_len = chunk.compressed_positions_len();
    int num_bytes = positions_len * sizeof(uint32_t);

    uint8_t len_of_len = IndexOutput::getVIntLength(num_bytes);

    int left = pTailChunk_->size - nPosInCurChunk_;

    if (left < (num_bytes + len_of_len))
    {
        pTailChunk_->size = nPosInCurChunk_;///the real size
        add_chunk_();
        return addPOSChunk(chunk);
    }

    {
        add_len_of_len_(num_bytes);
        memcpy(pTailChunk_->data + nPosInCurChunk_, positions, num_bytes);
        nTotalUsed_ += num_bytes;
        nPosInCurChunk_ += num_bytes;
    }

    return true;
}

bool ChunkDataPool::addChunk(const ChunkEncoder& chunk)
{
    if (pTailChunk_ == NULL)
    {
        add_chunk_();
        return addChunk(chunk);
    }

    // Docids.
    const uint32_t* doc_ids = chunk.compressed_doc_ids();
    int doc_ids_len = chunk.compressed_doc_ids_len() * sizeof(uint32_t);

    // Frequencies.
    const uint32_t* frequencies = chunk.compressed_frequencies();
    int frequencies_len = chunk.compressed_frequencies_len() * sizeof(uint32_t);

    uint8_t len_of_docid_len = IndexOutput::getVIntLength(doc_ids_len);

    uint8_t len_of_freq_len = IndexOutput::getVIntLength(frequencies_len);

    // Positions.
    const uint32_t* positions = chunk.compressed_positions();
    int positions_len = chunk.compressed_positions_len() * sizeof(uint32_t);

    uint8_t len_of_pos_len = IndexOutput::getVIntLength(positions_len);

    int left = pTailChunk_->size - nPosInCurChunk_;

    if (left < (doc_ids_len + len_of_docid_len + frequencies_len + len_of_freq_len + positions_len + len_of_pos_len))
    {
        pTailChunk_->size = nPosInCurChunk_;///the real size
        add_chunk_();
        return addChunk(chunk);
    }

    {
        add_len_of_len_(doc_ids_len);
        memcpy(pTailChunk_->data + nPosInCurChunk_, doc_ids, doc_ids_len);
        nTotalUsed_ += doc_ids_len;
        nPosInCurChunk_ += doc_ids_len;
    }

    {
        add_len_of_len_(frequencies_len);
        memcpy(pTailChunk_->data + nPosInCurChunk_, frequencies, frequencies_len);
        nTotalUsed_ += frequencies_len;
        nPosInCurChunk_ += frequencies_len;
    }

    {
        add_len_of_len_(positions_len);
        memcpy(pTailChunk_->data + nPosInCurChunk_, positions, positions_len);
        nTotalUsed_ += positions_len;
        nPosInCurChunk_ += positions_len;
    }

    return true;
}

void ChunkDataPool::add_len_of_len_(uint32_t ui)
{
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk_->data[nPosInCurChunk_++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
        nTotalUsed_++;
    }
    pTailChunk_->data[nPosInCurChunk_++] = (uint8_t)ui;
    nTotalUsed_++;
}

void ChunkDataPool::add_chunk_()
{
    int32_t factor = max(32,(int32_t)(nTotalSize_ + 0.5));
    int32_t chunkSize = (int32_t)Utilities::LOG2_UP(factor);

    uint8_t* begin = pMemCache_->getMemByLogSize(chunkSize);
    ///allocate memory failed,decrease chunk size
    if (!begin)
    {
        ///into UPTIGHT state
        begin = pMemCache_->getMemByLogSize(chunkSize);
        ///allocation failed again, grow memory cache.
        if (!begin)
        {
            MemCache* pUrgentMemCache = pMemCache_->grow(UPTIGHT_ALLOC_MEMSIZE);
            size_t urgentChunkSize = min((int32_t)Utilities::LOG2_DOWN(UPTIGHT_ALLOC_MEMSIZE),chunkSize);
  
            begin  = pUrgentMemCache->getMemByLogSize(urgentChunkSize);
            if (!begin)
            {
                SF1V5_THROW(ERROR_OUTOFMEM,"ChunkDataPool:add_chunk_() : Allocate memory failed.");
            }
            chunkSize = urgentChunkSize;
        }
    }

    ChunkData* pChunk = (ChunkData*)begin;
    pChunk->size = (int32_t)(POW_TABLE[chunkSize] - sizeof(ChunkData*) - sizeof(int32_t));
    pChunk->next = NULL;
 
    if (pTailChunk_)
        pTailChunk_->next = pChunk;
    pTailChunk_ = pChunk;
    if (!pHeadChunk_)
        pHeadChunk_ = pTailChunk_;
    nTotalSize_ += pChunk->size;

    nPosInCurChunk_ = 0;
}

void ChunkDataPool::write(IndexOutput* pOutput)
{
    ChunkData* pChunk = pHeadChunk_;
    while (pChunk)
    {
        pOutput->write((const char*)pChunk->data, pChunk->size);
        pChunk = pChunk->next;
    }
}

void ChunkDataPool::truncTailChunk()
{
    pTailChunk_->size = nPosInCurChunk_;
}

uint32_t ChunkDataPool::getLength()
{
    return nTotalUsed_;
}

void ChunkDataPool::reset()
{
    pHeadChunk_ = pTailChunk_ = NULL;
    nTotalSize_ = nPosInCurChunk_ = nTotalUsed_ = 0;
}


/************************************************************************************************************
 * BlockEncoder
 *
 *************************************************************************************************************/
const int BlockEncoder::kBlockSize;                            // Initialized in the class definition.
const int BlockEncoder::kChunkSizeLowerBound;                  // Initialized in the class definition.
const int BlockEncoder::kChunkDataUpperbound;            // Initialized in the class definition.
const int BlockEncoder::kChunkDataCompressedUpperbound;  // Initialized in the class definition.

BlockEncoder::BlockEncoder() 
    :chunk_data_uncompressed_size_(UncompressedInBufferUpperbound(kChunkDataUpperbound, block_header_compressor_.block_size()))
    ,chunk_data_uncompressed_(new uint32_t[chunk_data_uncompressed_size_])
{
    reset();
}

BlockEncoder::~BlockEncoder()
{
    delete[] chunk_data_uncompressed_;
}

// Returns true when 'chunk' fits into this block, false otherwise.
bool BlockEncoder::addChunk(const ChunkEncoder& chunk)
{
    uint32_t num_chunks = num_chunks_ + 1;

    // Store the next pair of (last doc_id, chunk size).
    int chunk_last_doc_id_idx = 2 * num_chunks - 2;
    int chunk_size_idx = 2 * num_chunks - 1;
    assert(chunk_last_doc_id_idx < chunk_data_uncompressed_size_);
    assert(chunk_size_idx < chunk_data_uncompressed_size_);
    chunk_data_uncompressed_[chunk_last_doc_id_idx] = chunk.last_doc_id();
    chunk_data_uncompressed_[chunk_size_idx] = chunk.size();

    const int kNumHeaderInts = 2 * num_chunks;

    int num_chunks_size = sizeof(num_chunks);
    int curr_block_header_size = num_chunks_size + CompressedOutBufferUpperbound((kNumHeaderInts * sizeof(uint32_t)));  // Upper bound with 'chunk' included.
    int curr_block_data_size = (block_data_offset_ + chunk.size()) * sizeof(uint32_t);
    int upper_bound_block_size = curr_block_data_size + curr_block_header_size;

    if (upper_bound_block_size > kBlockSize)
    {
        int curr_chunk_properties_compressed_len = compressHeader(chunk_data_uncompressed_, chunk_data_compressed_, kNumHeaderInts);

        if ((curr_block_data_size + num_chunks_size + (curr_chunk_properties_compressed_len * static_cast<int>(sizeof(uint32_t)))) <= kBlockSize)
        {
            chunk_data_compressed_len_ = curr_chunk_properties_compressed_len;
        }
        else
        {
            // Need to recompress the header (without the new chunk data, since it doesn't fit).
            chunk_data_compressed_len_ = compressHeader(chunk_data_uncompressed_, chunk_data_compressed_, num_chunks_ * 2);
            return false;
        }
    }

    // Chunk fits into this block, so copy chunk data to this block.
    copyChunkData(chunk);
    ++num_chunks_;
    return true;
}

void BlockEncoder::reset()
{
    num_chunks_ = 0;
    block_data_offset_ = 0;
    chunk_data_compressed_len_ = 0;
    num_block_header_bytes_ = 0;
    num_doc_ids_bytes_ = 0; 
    num_frequency_bytes_ = 0;
    num_wasted_space_bytes_ = 0;
    num_doc_ids_ = 0;
    first_doc_id_ = 0;
    last_doc_id_ = 0;
}

void BlockEncoder::copyChunkData(const ChunkEncoder& chunk)
{
    const int kWordSize = sizeof(uint32_t);
    const int kBlockSizeWords = kBlockSize / kWordSize;
    assert(block_data_offset_ + chunk.size() <= kBlockSizeWords);

    int num_bytes;
    int num_words;

    num_doc_ids_ += chunk.num_docs();

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

    if(first_doc_id_ == 0) first_doc_id_ = chunk.first_doc_id();
    last_doc_id_ = chunk.last_doc_id();
}

// The 'header' array size needs to be a multiple of the block size used by the block header compressor.
int BlockEncoder::compressHeader(uint32_t* header, uint32_t* output, int header_len)
{
    return block_header_compressor_.compress(header, output, header_len);
}

void BlockEncoder::getBlockBytes(unsigned char* block_bytes )
{
    if (chunk_data_compressed_len_ == 0)
    {
        const int kNumHeaderInts = 2 * num_chunks_;
        chunk_data_compressed_len_ = compressHeader(chunk_data_uncompressed_, chunk_data_compressed_, kNumHeaderInts);
    }
    assert(chunk_data_compressed_len_ > 0);
 
    int block_bytes_offset = 0;
    int num_bytes;
    // Number of chunks.
    num_bytes = sizeof(num_chunks_);
    assert(block_bytes_offset + num_bytes <= kBlockSize);
    memcpy(block_bytes + block_bytes_offset, &num_chunks_, num_bytes);
    block_bytes_offset += num_bytes;
    num_block_header_bytes_ += num_bytes;

    // Block header.
    num_bytes = chunk_data_compressed_len_ * sizeof(*chunk_data_compressed_);
    assert(block_bytes_offset + num_bytes <= kBlockSize);
    memcpy(block_bytes + block_bytes_offset, chunk_data_compressed_, num_bytes);
    block_bytes_offset += num_bytes;
    num_block_header_bytes_ += num_bytes;

    // Compressed chunk data.
    num_bytes = block_data_offset_ * sizeof(*block_data_);
    assert(block_bytes_offset + num_bytes <= kBlockSize);
    memcpy(block_bytes + block_bytes_offset, block_data_, num_bytes);
    block_bytes_offset += num_bytes;

    // Fill remaining wasted space with 0s.
    num_bytes = kBlockSize - block_bytes_offset;
    memset(block_bytes + block_bytes_offset, 0, num_bytes);
    num_wasted_space_bytes_ += num_bytes;
}

//////////////////////////////////////////////////////////////////////////
///BlockDataPool
int32_t BlockDataPool::UPTIGHT_ALLOC_MEMSIZE = 10*1024*1024;

BlockDataPool::BlockDataPool(boost::shared_ptr<MemCache> pMemCache)
    :pMemCache_(pMemCache)
    ,pHeadBlock_(NULL)
    ,pTailBlock_(NULL)
    ,nTotalSize_(0)
{
}

BlockDataPool::~BlockDataPool()
{
}

bool BlockDataPool::addChunk(const ChunkEncoder& chunk)
{
    if (pTailBlock_ == NULL)
    {
        addBlock();
        return addChunk(chunk);
    }

    if(!blockEncoder_.addChunk(chunk))
    {
        return false;
    }
    return true;
}

void BlockDataPool::copyBlockData()
{
    blockEncoder_.getBlockBytes(pTailBlock_->data);
    total_num_block_header_bytes_ += blockEncoder_.num_block_header_bytes();
    total_num_doc_ids_bytes_ += blockEncoder_.num_doc_ids_bytes();
    total_num_frequency_bytes_ += blockEncoder_.num_frequency_bytes();
    total_num_wasted_space_bytes_ += blockEncoder_.num_wasted_space_bytes();
    num_doc_of_curr_block_ = blockEncoder_.num_doc_ids();
}

void BlockDataPool::write(IndexOutput* pOutput)
{
    BlockData* pChunk = pHeadBlock_;
    while (pChunk)
    {
        pOutput->write((const char*)pChunk->data, BlockEncoder::kBlockSize);
        pChunk = pChunk->next;
    }
}

void BlockDataPool::addBlock()
{
    uint8_t* begin = pMemCache_->getMemByRealSize(BlockEncoder::kBlockSize+sizeof(BlockData*));
    ///allocate memory failed,decrease chunk size
    if (!begin)
    {
        MemCache* pUrgentMemCache = pMemCache_->grow(UPTIGHT_ALLOC_MEMSIZE);
  
        begin  = pUrgentMemCache->getMemByRealSize(BlockEncoder::kBlockSize+sizeof(BlockData*));
        if (!begin)
        {
            SF1V5_THROW(ERROR_OUTOFMEM,"BlockDataPool:addBlock() : Allocate memory failed.");
        }
    }

    BlockData* pChunk = (BlockData*)begin;
    pChunk->next = NULL;
 
    if (pTailBlock_)
        pTailBlock_->next = pChunk;
    pTailBlock_ = pChunk;
    if (!pHeadBlock_)
        pHeadBlock_ = pTailBlock_;
    nTotalSize_ += BlockEncoder::kBlockSize;
    blockEncoder_.reset();
}

uint32_t BlockDataPool::getLength()
{
    return nTotalSize_;
}

void BlockDataPool::reset()
{
    pHeadBlock_ = pTailBlock_ = NULL;
    nTotalSize_ = 0;
    total_num_block_header_bytes_ = 0;
    total_num_doc_ids_bytes_ = 0;
    total_num_frequency_bytes_ = 0;
    total_num_wasted_space_bytes_ = 0;
    blockEncoder_.reset();
}

uint32_t BlockDataPool::num_doc_of_curr_block()
{
    return num_doc_of_curr_block_;
}


}
NS_IZENELIB_IR_END

