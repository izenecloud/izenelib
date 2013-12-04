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

#include <boost/scoped_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Bitset;
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
    BlockPostingReader(
        InputDescriptor* pInputDescriptor,
        const TermInfo& termInfo,
        IndexLevel type = WORDLEVEL);

    ~BlockPostingReader();

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs);

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        uint32_t* &pPPosting,
        int32_t& posBufLength,
        int32_t& posLength);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param the length of pPosting. This function is only useful for to decode positions for skipTo target
     * @return true:success,false: error or reach end
     */
    bool DecodeNextPositions(
        uint32_t* pPosting,
        int32_t length);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param posBufLength buffer length to store posting data
     * @param decodeLength the length of decoded posting wanted.Only useful for BYTE-aligned posting
     * @return true:success,false: error or reach end
     */
    bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        int32_t decodeLength,
        int32_t& nCurrentPPosting);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param pFreqs freqs array
     * @param nFreqs size of freqs array
     */
    bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        uint32_t* pFreqs,
        int32_t nFreqs,
        int32_t& nCurrentPPosting);

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    docid_t DecodeTo(
        docid_t target,
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        int32_t& decodedCount,
        int32_t& nCurrentPosting);

    /**
     * reset the base position which used in d-gap encoding
     */
    void ResetPosition(){}

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
        return df_;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF()const
    {
        return ctf_;
    }

    docid_t lastDocID()
    {
        return 0;
    }

    void setFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

    void setListingCache(ListingCache* pListingCache)
    {
        pListingCache_ = pListingCache;
    }

protected:
    void advanceToNextBlock();

    void skipToBlock(int targetBlock) ;

    void ensure_compressed_pos_buffer(int num_of_pos_within_chunk)
    {
        if(curr_pos_buffer_size_ < num_of_pos_within_chunk)
        {
            curr_pos_buffer_size_  = num_of_pos_within_chunk;
            compressedPos_ = (uint32_t*)realloc(compressedPos_, curr_pos_buffer_size_ * sizeof(uint32_t));
        }
    }

protected:
    BlockDecoder blockDecoder_;
    boost::scoped_ptr<InputDescriptor> inputDescriptorPtr_;
    boost::scoped_ptr<FixedBlockSkipListReader> skipListReaderPtr_; ///skiplist reader
    ListingCache* pListingCache_;
    Bitset* pDocFilter_;

    int start_block_id_;
    int curr_block_id_;
    size_t total_block_num_;
    int last_block_id_;

    fileoffset_t postingOffset_;
    int64_t dlength_;
    count_t df_;
    int64_t ctf_;
    fileoffset_t poffset_; ///offset of the position postings in the .pop file
    int64_t plength_;
    docid_t last_doc_id_;
    count_t num_docs_decoded_;
    int32_t curr_pos_buffer_size_;

    docid_t prev_block_last_doc_id_;

    int prev_block_id_; ///previously accessed block
    int prev_chunk_; ///previously accessed chunk

    uint32_t* urgentBuffer_; ///used  when ListingCache is enabled, and cache is missed.
    uint32_t* compressedPos_;
    int32_t skipPosCount_;

    friend class PostingMerger;
};



/**************************************************************************************
* ChunkPostingReader
**************************************************************************************/
class ChunkPostingReader:public PostingReader
{
public:
    ChunkPostingReader(
        int skipInterval,
        int maxSkipLevel,
        InputDescriptor* pInputDescriptor,
        const TermInfo& termInfo,
        IndexLevel type = WORDLEVEL);

    ~ChunkPostingReader();

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs);

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        uint32_t* &pPPosting,
        int32_t& posBufLength,
        int32_t& posLength);

    /**
     * Get the position posting data
     * @param pPosting the address to store posting data
     * @param length the length of @p pPosting. This function is only useful to decode positions for skipTo target
     * @return true for success, false for error or reach end
     */
    bool DecodeNextPositions(
        uint32_t* pPosting,
        int32_t length);

    /**
     * Get the position posting data
     * @param pPosting the address to store posting data
     * @param posBufLength buffer length to store posting data
     * @param decodeLength the length of decoded posting wanted. Only useful for BYTE-aligned posting
     * @return true for success, false for error or reach end
     */
    bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        int32_t decodeLength,
        int32_t& nCurrentPPosting);

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param pFreqs freqs array
     * @param nFreqs size of freqs array
     */
    bool DecodeNextPositions(
        uint32_t* &pPosting,
        int32_t& posBufLength,
        uint32_t* pFreqs,
        int32_t nFreqs,
        int32_t& nCurrentPPosting);

    /**
     * @pre @p nMaxDocs >= @c CHUNK_SIZE, as @c CHUNK_SIZE docs are decoded
     *      altogether each time @c ChunkDecoder::decodeDocIds() is called.
     */
    docid_t DecodeTo(
        docid_t target,
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs,
        int32_t& decodedCount,
        int32_t& nCurrentPosting);

    /**
     * reset the base position which used in d-gap encoding
     */
    void ResetPosition(){}

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
        return df_;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF()const
    {
        return ctf_;
    }

    docid_t lastDocID()
    {
        return 0;
    }

    void setFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

protected:
    void ensure_compressed_pos_buffer(int num_of_pos_within_chunk)
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
    boost::scoped_ptr<InputDescriptor> inputDescriptorPtr_;
    boost::scoped_ptr<SkipListReader> skipListReaderPtr_; ///skiplist reader
    Bitset* pDocFilter_;

    fileoffset_t postingOffset_;
    int64_t dlength_;
    count_t df_;
    int64_t ctf_;
    fileoffset_t poffset_; ///offset of the position postings in the .pop file
    int64_t plength_;
    docid_t last_doc_id_;
    count_t num_docs_decoded_;
    int32_t curr_pos_buffer_size_;

    uint32_t compressedBuffer_[CHUNK_SIZE*2];
    uint32_t* compressedPos_;
    int32_t skipPosCount_;

    friend class PostingMerger;
};

}

NS_IZENELIB_IR_END

#endif
