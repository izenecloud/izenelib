/**
* @file        EPostingReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief E mode posting reader and decoder
*/

#ifndef E_POSTING_READER_H
#define E_POSTING_READER_H

#include <ir/index_manager/index/PostingReader.h>
#include <ir/index_manager/index/BlockDataDecoder.h>
#include <ir/index_manager/index/ListingCache.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class OutputDescriptor;
class InputDescriptor;
class FixedBlockSkipListReader;
class SkipListReader;
/**************************************************************************************
* BlockPostingReader
**************************************************************************************/
class BlockPostingReader:public PostingReader
{
public:
    BlockPostingReader(InputDescriptor* pInputDescriptor,const TermInfo& termInfo,IndexType type = WORD_LEVEL);

    ~BlockPostingReader();

    /**
     * Get the posting data
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @return decoded posting count
     */
    int32_t decodeNext(uint32_t* pPosting,int32_t length);

    /**
     * Get the posting data. 
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @param pPPosing the address to store position posting data
     * @return decoded posting count
     */
    int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& pLength);

    /**
     * Get the position posting data
    	 * @param pPosing the address to store posting data
     * @param the length of pPosting
     * @return true:success,false: error or reach end
     */
    bool decodeNextPositions(uint32_t* pPosting,int32_t length);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param pFreqs freqs array
     * @param nFreqs size of freqs array
     */
    bool decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs);

    /**
     * Set file pointer after skipping
     */
    void seekTo(FixedBlockSkipListReader* pSkipListReader);

    /**
     * Decode postings to target docID
     * @param docID target docID 
     * @return last decoded docID
     */
    docid_t decodeTo(docid_t docID);

    /**
     * reset the base position which used in d-gap encoding
     */
    void resetPosition();

    /**
     * reset the content of Posting list.
     */
    void reset();

    /**
     * reset to a new posting
     */
    void reset(const TermInfo& termInfo);

    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq()const
    {
        return 0;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF()const
    {
        return 0;
    }

    /*
     * get current tf
    */
    count_t getCurTF() const
    {
        return 0;
    }

    docid_t lastDocID()
    {
        return 0;
    }

    void setFilter(BitVector* pFilter) 
    { 
        pDocFilter_ = pFilter;
    }

    void setListingCache(ListingCache* pListingCache)
    {
        pListingCache_ = pListingCache;
    }

protected:
    void advanceToNextBlock();

    void skipToBlock(size_t targetBlock) ;

    void ensure_pos_buffer(int num_of_pos_within_chunk)
    {
        if(curr_pos_buffer_size_ < num_of_pos_within_chunk)
        {
            curr_pos_buffer_size_  = num_of_pos_within_chunk;
            compressedPos_ = (uint32_t*)realloc(compressedPos_, curr_pos_buffer_size_ * sizeof(uint32_t));
        }
    }
protected:
    BlockDecoder blockDecoder_;
    InputDescriptor* pInputDescriptor_;
    FixedBlockSkipListReader* pSkipListReader_; ///skiplist reader
    ListingCache* pListingCache_;
    BitVector* pDocFilter_;

    size_t start_block_id_;
    size_t curr_block_id_;
    size_t total_block_num_;
    size_t last_block_id_;

    fileoffset_t postingOffset_;
    int64_t dlength_;
    count_t df_;
    int64_t ctf_;
    fileoffset_t poffset_; ///offset of the position postings in the .pop file
    int64_t plength_;
    docid_t last_doc_id_;
    count_t num_docs_left_;
    count_t num_docs_decoded_;
    loc_t last_pos_decoded_; 		///the latest decoded position posting
    int32_t num_pos_decoded_;		///decoded position posting count
    int32_t num_pos_chunk_skipped_;
    int32_t curr_pos_buffer_size_;

    docid_t prev_block_last_doc_id_;
	
    uint32_t* urgentBuffer_;
    uint32_t* compressedPos_;
    uint32_t* doc_freq_decode_buffer_;

    friend class PostingMerger;
};



/**************************************************************************************
* ChunkPostingReader
**************************************************************************************/
class ChunkPostingReader:public PostingReader
{
public:
    ChunkPostingReader(int skipInterval, int maxSkipLevel, InputDescriptor* pInputDescriptor,const TermInfo& termInfo,IndexType type = WORD_LEVEL);

    ~ChunkPostingReader();

    /**
     * Get the posting data
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @return decoded posting count
     */
    int32_t decodeNext(uint32_t* pPosting,int32_t length);

    /**
     * Get the posting data. 
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @param pPPosing the address to store position posting data
     * @return decoded posting count
     */
    int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& pLength);

    /**
     * Get the position posting data
    	 * @param pPosing the address to store posting data
     * @param the length of pPosting
     * @return true:success,false: error or reach end
     */
    bool decodeNextPositions(uint32_t* pPosting,int32_t length);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param pFreqs freqs array
     * @param nFreqs size of freqs array
     */
    bool decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs);

    /**
     * Set file pointer after skipping
     */
    void seekTo(SkipListReader* pSkipListReader);

    /**
     * Decode postings to target docID
     * @param docID target docID 
     * @return last decoded docID
     */
    docid_t decodeTo(docid_t docID);

    /**
     * reset the base position which used in d-gap encoding
     */
    void resetPosition();

    /**
     * reset the content of Posting list.
     */
    void reset();

    /**
     * reset to a new posting
     */
    void reset(const TermInfo& termInfo);

    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq()const
    {
        return 0;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF()const
    {
        return 0;
    }

    /*
     * get current tf
    */
    count_t getCurTF() const
    {
        return 0;
    }

    docid_t lastDocID()
    {
        return 0;
    }

    void setFilter(BitVector* pFilter) 
    { 
        pDocFilter_ = pFilter;
    }

protected:
    void ensure_pos_buffer(int num_of_pos_within_chunk)
    {
        if(curr_pos_buffer_size_ < num_of_pos_within_chunk)
        {
            curr_pos_buffer_size_  = num_of_pos_within_chunk;
            compressedPos_ = (uint32_t*)realloc(compressedPos_, curr_pos_buffer_size_ * sizeof(uint32_t));
        }
    }
protected:
    int skipInterval_;
    int maxSkipLevel_;
    ChunkDecoder chunkDecoder_;
    InputDescriptor* pInputDescriptor_;
    SkipListReader* pSkipListReader_; ///skiplist reader
    BitVector* pDocFilter_;

    size_t start_block_id_;
    size_t curr_block_id_;
    size_t total_block_num_;
    size_t last_block_id_;

    fileoffset_t postingOffset_;
    int64_t dlength_;
    count_t df_;
    int64_t ctf_;
    fileoffset_t poffset_; ///offset of the position postings in the .pop file
    int64_t plength_;
    docid_t last_doc_id_;
    count_t num_docs_left_;
    count_t num_docs_decoded_;
    loc_t last_pos_decoded_; 		///the latest decoded position posting
    int32_t num_pos_decoded_;		///decoded position posting count
    int32_t num_pos_chunk_skipped_;
    int32_t curr_pos_buffer_size_;

    docid_t prev_block_last_doc_id_;
	
    uint32_t compressedBuffer_[CHUNK_SIZE*2];
    uint32_t* compressedPos_;

    friend class PostingMerger;
};

}

NS_IZENELIB_IR_END


#endif

