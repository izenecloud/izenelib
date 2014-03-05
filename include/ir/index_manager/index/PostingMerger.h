/**
* @file        PostingMerger.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   merge index at posting level
*/

#ifndef POSTINGMERGER_H
#define POSTINGMERGER_H

#include <ir/index_manager/index/RTPostingReader.h>
#include <ir/index_manager/index/EPostingReader.h>
#include <ir/index_manager/index/EPostingWriter.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/SkipListMerger.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/utility/Bitset.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///Posting Merger
class PostingMerger
{
public:
    PostingMerger(
        int skipInterval,
        int maxSkipLevel,
        CompressionType compressType,
        bool optimize,
        bool requireIntermediateFileForMerging,
        IndexLevel indexLevel,
        size_t memPoolSizeForPostingMerger
    );

    virtual ~PostingMerger();

public:
    void setCompressionType(CompressionType compressType) { compressType_ = compressType;}

    void setOutputDescriptor(OutputDescriptor* pOutputDescriptor);

    OutputDescriptor* getOutputDescriptor() {return pOutputDescriptor_;}

    void mergeWith(MemPostingReader* pInMemoryPosting);

    void mergeWith(RTDiskPostingReader* pOnDiskPosting, Bitset* pFilter);

    void mergeWith(BlockPostingReader* pPosting, Bitset* pFilter);

    void mergeWith(ChunkPostingReader* pPosting, Bitset* pFilter);

    void optimize(RTDiskPostingReader* pOnDiskPosting, Bitset* pFilter);

    fileoffset_t endMerge();

    void reset();

private:
    /** create buffer_ for merging */
    void init();

    void ensure_compressed_pos_buffer(int num_of_pos_within_chunk)
    {
        if(compressed_position_buffer_size_ < num_of_pos_within_chunk)
        {
            compressed_position_buffer_size_  = num_of_pos_within_chunk;
            compressedPos_ = (uint32_t*)realloc(compressedPos_, compressed_position_buffer_size_ * sizeof(uint32_t));
        }
    }

    /**
     * ensure the buffer size of @p positions_ is enough to store @p newPosNum decompressed position values.
     * @param newPosNum the number of position values to decompress
     */
    void ensure_decompressed_pos_buffer(int newPosNum)
    {
        PostingReader::EnsurePosBufferUpperBound(positions_, curr_position_buffer_size_,
                                                 position_buffer_pointer_ + newPosNum);
    }

    void mergeWith(RTDiskPostingReader* pOnDiskPosting);

    void mergeWith_GC(RTDiskPostingReader* pOnDiskPosting, Bitset* pFilter);

    void optimize_to_Block(RTDiskPostingReader* pOnDiskPosting, Bitset* pFilter);

    void optimize_to_Chunk(RTDiskPostingReader* pOnDiskPosting, Bitset* pFilter);

    fileoffset_t endMerge_ByteAlign();

    fileoffset_t endMerge_Block();

    fileoffset_t endMerge_Chunk();

    /**
     * Check whether @c skipInterval_ and @c maxSkipLevel_ are both positive value.
     * @return true for valid, false for invalid (no skip data to merge)
     */
    bool isSkipParamValid() const
    {
        return skipInterval_ > 0 && maxSkipLevel_ > 0;
    }

private:
    friend class FieldMerger;

    int skipInterval_;

    int maxSkipLevel_;

    CompressionType compressType_;

    TermInfo termInfo_;

    OutputDescriptor* pOutputDescriptor_; ///where merged data store

    std::string tmpPostingName_;

    IndexOutput* pTmpPostingOutput_; ///used during merging posting

    IndexInput* pTmpPostingInput_; ///used during merging posting

    VariantDataPool* pVIntDataOutput_; ///used during merging vint posting using memory

    bool requireIntermediateFileForMerging_;  // switch between pTmpPostingOutput_ and pVIntDataOutput_

    PostingDescriptor postingDesc_;

    ChunkDescriptor chunkDesc_;

    int64_t nPPostingLength_;

    int nSkipIntervalBetweenBarrels_; ///skip interval between two postings from different barrels might not be the default fixed skipinterval value

    bool bFirstPosting_;

    SkipListMerger* pSkipListMerger_;

    boost::shared_ptr<MemCache> pMemCache_; /// memory cache

    FixedBlockSkipListWriter* pFixedSkipListWriter_;

    SkipListWriter* pSkipListWriter_;

    uint32_t* compressedPos_;

    int compressed_position_buffer_size_;

    uint32_t doc_ids_[ChunkEncoder::kChunkSize];

    uint32_t frequencies_[ChunkEncoder::kChunkSize];

    int doc_ids_offset_;

    uint32_t* positions_;

    int curr_position_buffer_size_;

    int position_buffer_pointer_;

    ChunkEncoder chunk_;

    BlockEncoder blockEncoder_;

    ChunkDataPool* pPosDataPool_;

    ChunkDataPool* pDocFreqDataPool_;

    uint8_t* block_buffer_;

    uint32_t current_block_id_;

    uint32_t compressedBuffer_[CHUNK_SIZE*2];

    uint32_t internal_doc_ids_buffer_[UncompressedOutBufferUpperbound(CHUNK_SIZE)];

    uint32_t internal_freqs_buffer_[UncompressedOutBufferUpperbound(CHUNK_SIZE)];

    bool optimize_; /// converting BYTEALIGN  to BLOCK or CHUNK when TRUE

    IndexLevel indexLevel_;
};

}

NS_IZENELIB_IR_END

#endif
