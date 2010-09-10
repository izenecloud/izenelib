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
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class OutputDescriptor;
class InputDescriptor;
class FixedBlockSkipListReader;
/**************************************************************************************
* BlockPostingReader
**************************************************************************************/
class BlockPostingReader:public PostingReader
{
public:
    BlockPostingReader(InputDescriptor* pInputDescriptor,const TermInfo& termInfo);

    ~BlockPostingReader();

    /**
     * Get the posting data
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @return decoded posting count
     */
    int32_t decodeNext(uint32_t* pPosting,int32_t length);

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

protected:
    BlockDecoder blockDecoder_;
    BitVector* pDocFilter_;
    InputDescriptor* pInputDescriptor_;
    FixedBlockSkipListReader* pSkipListReader_; ///skiplist reader
    ListingCache* pListingCache_;

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

    uint32_t* urgentBuffer_;
    friend class PostingMerger;
};


}

NS_IZENELIB_IR_END


#endif

