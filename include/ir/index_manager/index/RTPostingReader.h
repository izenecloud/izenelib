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
#include <ir/index_manager/index/InputDescriptor.h>

#include <boost/scoped_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Bitset;
class SkipListReader;
class OutputDescriptor;

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
        int32_t decodedPosCount;
    };
public:
    explicit MemPostingReader(
        boost::shared_ptr<RTPostingWriter> pPostingWriter,
        const TermInfo& termInfo,
        IndexLevel type=WORDLEVEL);

    virtual ~MemPostingReader();
public:
    ///Get document frequency
    count_t docFreq() const;
    /// Get collection's total term frequency
    int64_t getCTF() const;

    int32_t getMaxDocFreq()const;
    /// Get last added doc id
    docid_t lastDocID();

    int32_t getSkipLevel();

    count_t getDPostingLen();

    count_t getPPostingLen();

    void setFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs);

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
     * @param the length of pPosting
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
     * @post as RT posting reader, @p decodedCount is always 1, @p nCurrentPosting is always 0
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
    void ResetPosition();

    /**
     * get skiplist reader
     */
    SkipListReader* getSkipListReader();
private:
    MemPostingReader(const MemPostingReader&);
    void operator=(const MemPostingReader&);

protected:
    int skipInterval_;
    int maxSkipLevel_;

    boost::shared_ptr<RTPostingWriter> pPostingWriter_;
    boost::shared_ptr<VariantDataPool> pDocFreqList_; /// Doc freq list
    boost::shared_ptr<VariantDataPool> pLocList_; 	/// Location list
    TermInfo termInfo_;
    DecodeState* pDS_;			///decoding state
    SkipListReader* pSkipListReader_; ///skiplist reader
    Bitset* pDocFilter_;
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
    int32_t maxTF;
    int32_t plength;
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
    RTDiskPostingReader(
        int skipInterval,
        int maxSkipLevel,
        InputDescriptor* pInputDescriptor,
        const TermInfo& termInfo);

    ~RTDiskPostingReader();

    int32_t DecodeNext(
        uint32_t* pPosting,
        int32_t length,
        int32_t nMaxDocs);

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
     * @param the length of pPosting
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
     * Set file pointer after skipping
     */
    void SeekTo(SkipListReader* pSkipListReader);

    /**
     * @post as RT posting reader, @p decodedCount is always 1, @p nCurrentPosting is always 0
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
    void ResetPosition();

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

    SkipListReader* getSkipListReader()
    {
        return skipListReaderPtr_.get();
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

    void setFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

    InputDescriptor* getInputDescriptor()
    {
        return inputDescriptorPtr_.get();
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
    boost::scoped_ptr<InputDescriptor> inputDescriptorPtr_;
    RTDiskPostingReader::DecodeState ds_;
    boost::scoped_ptr<SkipListReader> skipListReaderPtr_; ///skiplist reader
    Bitset* pDocFilter_;

    friend class PostingMerger;
};

}

NS_IZENELIB_IR_END


#endif
