/**
* @file        RTPostingReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief real time posting reader and decoder
*/

#ifndef RT_POSTING_READER_H
#define RT_POSTING_READER_H

#include <ir/index_manager/index/PostingReader.h>
#include <ir/index_manager/index/VariantDataPool.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class SkipListReader;
class OutputDescriptor;
class InputDescriptor;

class RTPostingWriter;
/**
*MemPostingReader
*/
class MemPostingReader:public PostingReader
{
public:
    struct DecodeState
    {
        VariantDataChunk* decodingDChunk;
        int32_t decodingDChunkPos;
        docid_t lastDecodedDocID;		///the latest decoded doc id
        int32_t decodedDocCount;
        VariantDataChunk* decodingPChunk;
        int32_t decodingPChunkPos;
        loc_t lastDecodedPos; 		///the latest decoded offset
        loc_t lastDecodedCharPos;        ///the latest decoded char offset
        int32_t decodedPosCount;
    };
public:
    MemPostingReader(RTPostingWriter* pPostingWriter);

    virtual ~MemPostingReader();
public:
    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq() const;
    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF() const;
    /*
     * get current tf
    */
    count_t getCurTF() const;

    /** get last added doc id */
    docid_t lastDocID();

    int32_t getSkipLevel();

    count_t getDPostingLen();

    count_t getPPostingLen();

    void setFilter(BitVector* pFilter) 
    { 
        pDocFilter_ = pFilter;
    }

    /**
     * Get the posting data
     * @param ppState the state of decode process
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
     int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength);

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
    bool decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting);

    /**
     * Decode postings to target docID
     * @param docID target docID 
     * @param pPosting posting buffer that store decoded doc. 
     * @length buffer length for pPosting
     * @decodedCount decoded doc count, always = 1 for RT posting reader
     * @nCurrentPosting posting pointer, always = 0 for RT posting reader
     * @return last decoded docID
     */
    docid_t decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting);

    /**
     * reset the base position which used in d-gap encoding
     */
    void resetPosition();

    /*
     * get skiplist reader
     */
    SkipListReader* getSkipListReader();

protected:
    int skipInterval_;
    int maxSkipLevel_;

    RTPostingWriter* pPostingWriter_;
    DecodeState* pDS_;			///decoding state
    SkipListReader* pSkipListReader_; ///skiplist reader
    BitVector* pDocFilter_;
    friend class PostingMerger;
};

/**
*RTDiskPostingReader
*/

/// the descriptor of posting
struct PostingDescriptor
{
    int64_t length; ///length of the posting
    count_t df; ///document frequency of this field
    int64_t ctf; ///global  term frequency
    fileoffset_t poffset; ///offset of the position postings in the .pop file
    int64_t plength;
};

/// the descriptor of chunk
struct ChunkDescriptor
{
    int64_t length; ///length of the chunk
    docid_t lastdocid; ///the last doc id of the chunk
};

class RTDiskPostingReader:public PostingReader
{
    struct DecodeState
    {
        docid_t lastDecodedDocID;		///the latest decoded doc id
        int32_t lastDecodedDocTF;		///the latest decoded doc tf
        int32_t decodedDocCount;		///decoded doc count
        loc_t lastDecodedPos; 		///the latest decoded position posting
        int32_t decodedPosCount;		///decoded position posting count
        int32_t skipPosCount_; 		///position count to be skipped
    };
public:
    RTDiskPostingReader(int skipInterval, int maxSkipLevel, InputDescriptor* pInputDescriptor,const TermInfo& termInfo);

    ~RTDiskPostingReader();

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
    int32_t decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength);
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
    bool decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting);

    /**
     * Set file pointer after skipping
     */
    void seekTo(SkipListReader* pSkipListReader);

    /**
     * Decode postings to target docID
     * @param docID target docID 
     * @param pPosting posting buffer that store decoded doc. 
     * @length buffer length for pPosting
     * @decodedCount decoded doc count, always = 1 for RT posting reader
     * @nCurrentPosting posting pointer, always = 0 for RT posting reader
     * @return last decoded docID
     */
    docid_t decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting);

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
        return postingDesc_.df;
    };

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF()const
    {
        return postingDesc_.ctf;
    };

    /*
     * get current tf
    */
    count_t getCurTF() const
    {
        return ds_.lastDecodedDocTF;
    }

    SkipListReader* getSkipListReader()
    {
        return pSkipListReader_;
    }

    docid_t lastDocID()
    {
        return chunkDesc_.lastdocid;
    }

    count_t getDPostingLen()
    {
        return postingDesc_.length;
    }

    count_t getPPostingLen()
    {
        return postingDesc_.plength;
    }

    void setFilter(BitVector* pFilter) 
    { 
        pDocFilter_ = pFilter;
    }

    InputDescriptor* getInputDescriptor()
    {
        return pInputDescriptor_;
    }

protected:
    void skipPositions();	

protected:
    int skipInterval_;
    int maxSkipLevel_;
    PostingDescriptor postingDesc_;
    ChunkDescriptor chunkDesc_;
    fileoffset_t postingOffset_;
    int64_t nPPostingLength_;
    InputDescriptor* pInputDescriptor_;
    RTDiskPostingReader::DecodeState ds_;
    SkipListReader* pSkipListReader_; ///skiplist reader
    BitVector* pDocFilter_;

    friend class PostingMerger;
};

}

NS_IZENELIB_IR_END


#endif

