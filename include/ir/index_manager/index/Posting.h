/**
* @file        Posting.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Process the postings
*/

#ifndef POSTING_H
#define POSTING_H

#include <ir/index_manager/index/VariantDataPool.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class OutputDescriptor;
class InputDescriptor;

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

/**
*virtual base class of InMemoryPosting and OnDiskPosting
*/
class SkipListWriter;
class SkipListReader;

class Posting
{
public:
    Posting(int skipInterval, int maxSkipLevel);
    virtual ~Posting();
public:
    /**
     * Get the posting data
     * @param ppState the state of decode process
     * @param pPosing the address to store posting data
     * @param the length of pPosting,also tell us the length of actually decoded data
     * @return decoded posting count
     */
    virtual int32_t decodeNext(uint32_t* pPosting,int32_t length) = 0;

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param length length of pPosting
     */
    virtual bool decodeNextPositions(uint32_t* pPosting,int32_t length) = 0;

    /**
     * Get the position posting data
     * @param pPosing the address to store posting data
     * @param pFreqs freqs array
     * @param nFreqs size of freqs array
     */
    virtual bool decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs) = 0;

    /**
     * reset the base position which used in d-gap encoding
     */
    virtual void resetPosition() = 0;

    /**
     * Decode postings to target docID
     * @param docID target docID 
     * @return last decoded docID
     */
    virtual docid_t decodeTo(docid_t docID) = 0;

    /**
     * reset the content of Posting list.
     */
    virtual void reset() = 0;

    /**
     * clone the object
     * @return the clone object
     */
    virtual Posting* clone() = 0;

    /**
     * get document frequency
     * @return DF value
     */
    virtual count_t docFreq()const = 0;

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    virtual int64_t getCTF()const = 0;

    /*
     * get current tf
    */
    virtual count_t getCurTF()const = 0;


    /**
     * get buffer size of posting
     * @return buffer size
     */
    virtual size_t getBufferSize()
    {
        return 0;
    }

    virtual SkipListReader* getSkipListReader() = 0;    

    void setFilter(BitVector* pFilter) { pDocFilter_ = pFilter;}

    virtual docid_t lastDocID() = 0;

    virtual count_t getDPostingLen() = 0;

    virtual count_t getPPostingLen() = 0;

public:
    int skipInterval_;
    int maxSkipLevel_;

protected:
    BitVector* pDocFilter_;
};

/**
*InMemoryPosting
*/
class InMemoryPosting:public Posting
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
    InMemoryPosting(int skipInterval, int maxSkipLevel);
    InMemoryPosting(MemCache* pMemCache, int skipInterval, int maxSkipLevel);
    virtual ~InMemoryPosting();
public:
    /**
     * add (docid,position) pair
     * @param docid the identifier of document
     * @param location the location of term
     */
    void add(docid_t docid, loc_t location);

    /**
     * Is there any chunk?
     * @return false: has allocated; true:no memory has allocated
     **/
    bool hasNoChunk()
    {
        return (pDocFreqList_->pTailChunk_==NULL);
    }

    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq() const
    {
        return nDF_;
    };

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t getCTF() const
    {
        return nCTF_;
    };

    /*
     * get current tf
    */
    count_t getCurTF() const
    {
        return nCurTermFreq_;
    }

    /** get last added doc id */
    docid_t lastDocID()
    {
        return nLastDocID_;
    }

    int32_t getSkipLevel();

    count_t getDPostingLen();

    count_t getPPostingLen();

    /**
     * write index data
     * @param pOutputDescriptor output place
     * @param termInfo set term info for voc
     */
    void write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo);

    /**
     * reset the content of Posting list.
     */
    void reset();


    Posting* clone();

public:
    /**
     * Get the posting data
     * @param ppState the state of decode process
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
     * flush last document
     * @param bTruncTail trunc the tail of posting or not
     */
    void flushLastDoc(bool bTruncTail);

    /*
     * get skiplist reader
     */
    SkipListReader* getSkipListReader();

    void setDirty(bool dirty) { dirty_ = dirty; }

    bool isDirty() { return dirty_; }
protected:
    MemCache* pMemCache_;	/// memory cache
    count_t nDF_;			///document frequency of this field
    docid_t nLastDocID_;	///current added doc id
    loc_t nLastLoc_; 		///current added word offset
    count_t nCurTermFreq_; ///current term freq
    int32_t nCTF_; 			///Collection's total term frequency
    DecodeState* pDS_;			///decoding state
    VariantDataPool* pDocFreqList_; /// Doc freq list
    VariantDataPool* pLocList_; 	/// Location list
    SkipListWriter* pSkipListWriter_;   ///skiplist writer
    SkipListReader* pSkipListReader_; ///skiplist reader
    volatile bool dirty_;
    friend class PostingMerger;
};

/**
*OnDiskPosting
*/

class OnDiskPosting:public Posting
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
    OnDiskPosting(int skipInterval, int maxSkipLevel);

    OnDiskPosting(int skipInterval, int maxSkipLevel, InputDescriptor* pInputDescriptor,const TermInfo& termInfo);

    ~OnDiskPosting();

    InputDescriptor* getInputDescriptor()
    {
        return pInputDescriptor_;
    }
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
     * clone the object
     * @return the clone object
     */
    Posting* clone();

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

protected:
    PostingDescriptor postingDesc_;
    ChunkDescriptor chunkDesc_;
    fileoffset_t postingOffset_;
    int64_t nPPostingLength_;
    InputDescriptor* pInputDescriptor_;
    size_t nBufSize_;
    OnDiskPosting::DecodeState ds_;
    SkipListReader* pSkipListReader_; ///skiplist reader

    friend class PostingMerger;
};


}

NS_IZENELIB_IR_END

#endif
