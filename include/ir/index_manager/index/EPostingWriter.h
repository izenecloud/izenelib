/**
* @file        EPostingWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief E posting builder
*/
#ifndef E_POSTING_WRITER_H
#define E_POSTING_WRITER_H

#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/PostingWriter.h>
#include <ir/index_manager/index/BlockDataPool.h>
#include <ir/index_manager/index/FixedBlockSkipListWriter.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/*******************************************************************
* @BlockPostingWriter
* Block based posting builder:
*      [skip data][block][block]
********************************************************************/
class BlockPostingWriter:public PostingWriter
{
public:
    BlockPostingWriter(
        boost::shared_ptr<MemCache> pMemCache,
        IndexLevel indexLevel);

    ~BlockPostingWriter();
    /**
     * add data to posting
     */
    void add(uint32_t docId, uint32_t pos, bool realTimeFlag=false);
    /**
    * whether current posting contains valid data
    */
    bool isEmpty();
    /**
     * write index data
     * @param pOutputDescriptor output place
     * @param termInfo set term info for voc
     */
    void write(
        OutputDescriptor* pOutputDescriptor,
        TermInfo& termInfo);
    /**
     * reset the content of Posting list.
     */
    void reset();
    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq() const
    {
        return nDF_;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    count_t getCTF() const
    {
        return nCTF_;
    }


    /** get last added doc id */
    docid_t lastDocID()
    {
        return nLastDocID_;
    }

protected:
    void flush();

protected:
    ChunkEncoder chunk_;
    boost::shared_ptr<MemCache> pMemCache_;	/// memory cache
    BlockDataPool* pBlockDataPool_;
    ChunkDataPool* pPosDataPool_;
    FixedBlockSkipListWriter* pSkipListWriter_;
    uint32_t doc_ids_[ChunkEncoder::kChunkSize];
    uint32_t frequencies_[ChunkEncoder::kChunkSize];
    int current_nocomp_block_pointer_;

    uint32_t* positions_;
    uint32_t position_buffer_pointer_;
    uint32_t curr_position_buffer_size_;
    count_t nDF_;			///document frequency of this field
    count_t nCurTermFreq_;
    count_t nmaxTF_;
    count_t nCTF_;			///Collection's total term frequency
    docid_t nLastDocID_;	///current added doc id

    uint32_t current_block_id_;

    IndexLevel indexLevel_;
};



/*******************************************************************
* @ChunkPostingWriter
* Chunk based posting builder:
*      [skip data][chunk][chunk]
********************************************************************/
class SkipListWriter;
class ChunkPostingWriter:public PostingWriter
{
public:
    ChunkPostingWriter(
        boost::shared_ptr<MemCache> pMemCache,
        int skipInterval,
        int maxSkipLevel,
        IndexLevel indexLevel);

    ~ChunkPostingWriter();
    /**
     * add data to posting
     */
    void add(uint32_t docId, uint32_t pos, bool realTimeFlag=false);
    /**
    * whether current posting contains valid data
    */
    bool isEmpty();
    /**
     * write index data
     * @param pOutputDescriptor output place
     * @param termInfo set term info for voc
     */
    void write(
        OutputDescriptor* pOutputDescriptor,
        TermInfo& termInfo);
    /**
     * reset the content of Posting list.
     */
    void reset();
    /**
     * get document frequency
     * @return DF value
     */
    count_t docFreq() const
    {
        return nDF_;
    }

    /**
     * get collection's total term frequency
     * @return CTF value
     */
    count_t getCTF() const
    {
        return nCTF_;
    }

    /** get last added doc id */
    docid_t lastDocID()
    {
        return nLastDocID_;
    }

protected:
    void flush();

protected:
    ChunkEncoder chunk_;
    boost::shared_ptr<MemCache> pMemCache_;	/// memory cache

    ChunkDataPool* pDocFreqDataPool_;
    ChunkDataPool* pPosDataPool_;

    SkipListWriter* pSkipListWriter_;
    int skipInterval_;              ///skip interval
    int maxSkipLevel_;           /// max skip level

    uint32_t doc_ids_[ChunkEncoder::kChunkSize];
    uint32_t frequencies_[ChunkEncoder::kChunkSize];
    int current_nocomp_block_pointer_;

    uint32_t* positions_;
    uint32_t position_buffer_pointer_;
    uint32_t curr_position_buffer_size_;
    count_t nDF_;			///document frequency of this field
    count_t nCurTermFreq_;
    count_t nmaxTF_;
    count_t nCTF_;			///Collection's total term frequency
    docid_t nLastDocID_;	///current added doc id

    IndexLevel indexLevel_;
};


}

NS_IZENELIB_IR_END


#endif
