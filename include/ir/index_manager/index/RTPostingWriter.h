/**
* @file        RTPostingWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief real time posting builder
*/

#ifndef RT_POSTING_WRITER_H
#define RT_POSTING_WRITER_H

#include <ir/index_manager/index/VariantDataPool.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/index/PostingWriter.h>
#include <ir/index_manager/index/RTPostingReader.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class SkipListWriter;
class RTPostingWriter:public PostingWriter
{
public:
    RTPostingWriter(MemCache* pMemCache, int skipInterval, int maxSkipLevel, IndexLevel indexLevel);

    ~RTPostingWriter();
    /**
     * add data to posting
     */
    void add(uint32_t docId, uint32_t pos);
    /**
    * whether current posting contains valid data
    */
    bool isEmpty();
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
    /**
    * flush last document
    */
    void flushLastDoc(bool bTruncTail);
    /**
    * get posting reader, it is required by real-time query
    */
    PostingReader* createPostingReader();
    /*
    * set in memory posting as dirty, as a result, it can not be searched any more
    */
    void setDirty(bool dirty) { dirty_ = dirty; }
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

private:
    MemCache* pMemCache_;	/// memory cache
    int skipInterval_;              ///skip interval
    int maxSkipLevel_;           /// max skip level
    count_t nDF_;			///document frequency of this field
    docid_t nLastDocID_;	///current added doc id
    loc_t nLastLoc_;		///current added word offset
    count_t nCurTermFreq_; ///current term freq
    int32_t nCTF_;			///Collection's total term frequency
    VariantDataPool* pDocFreqList_; /// Doc freq list
    VariantDataPool* pLocList_; 	/// Location list
    SkipListWriter* pSkipListWriter_;	///skiplist writer
    volatile bool dirty_;
    IndexLevel indexLevel_;
    friend class MemPostingReader;
    friend class PostingMerger;
};


}

NS_IZENELIB_IR_END


#endif

