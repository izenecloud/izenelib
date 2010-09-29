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
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
#define POSTINGMERGE_BUFFERSIZE 32768
///Posting Merger
class PostingMerger
{
public:
    PostingMerger(int skipInterval, int maxSkipLevel, CompressionType compressType, bool optimize, MemCache* pMemCache);

    virtual ~PostingMerger();

public:
    void setCompressionType(CompressionType compressType) { compressType_ = compressType;}

    void setOutputDescriptor(OutputDescriptor* pOutputDescriptor);

    OutputDescriptor* getOutputDescriptor() {return pOutputDescriptor_;}

    void mergeWith(MemPostingReader* pInMemoryPosting);

    void mergeWith(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter);

    void mergeWith(BlockPostingReader* pPosting,BitVector* pFilter);

    void mergeWith(ChunkPostingReader* pPosting,BitVector* pFilter);

    void optimize(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter);

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

    void ensure_pos_buffer(int num_of_pos_within_chunk)
    {
        if((curr_position_buffer_size_ - position_buffer_pointer_) <= num_of_pos_within_chunk)
        {
            curr_position_buffer_size_  = (num_of_pos_within_chunk + position_buffer_pointer_) << 1;
            positions_ = (uint32_t*)realloc(positions_, curr_position_buffer_size_ * sizeof(uint32_t));
        }
    }

    void mergeWith(RTDiskPostingReader* pOnDiskPosting);

    void mergeWith_GC(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter);

    void optimize_to_Block(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter);
	
    void optimize_to_Chunk(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter);

    fileoffset_t endMerge_ByteAlign();

    fileoffset_t endMerge_Block();

    fileoffset_t endMerge_Chunk();

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

    PostingDescriptor postingDesc_;

    ChunkDescriptor chunkDesc_;

    int64_t nPPostingLength_;

    int nSkipIntervalBetweenBarrels_; ///skip interval between two postings from different barrels might not be the default fixed skipinterval value

    bool bFirstPosting_;

    SkipListMerger* pSkipListMerger_;

    MemCache* pMemCache_; /// memory cache

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

    bool optimize_; /// converting BYTEALIGN  to BLOCK or CHUNK when TRUE

    bool ownMemCache_;
};

}

NS_IZENELIB_IR_END

#endif
