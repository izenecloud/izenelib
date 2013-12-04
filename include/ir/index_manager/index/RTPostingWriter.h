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
#include <ir/index_manager/index/PostingWriter.h>
#include <ir/index_manager/index/RTPostingReader.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class SkipListWriter;
class RTPostingWriter:public PostingWriter
{
public:
    explicit RTPostingWriter(
        boost::shared_ptr<MemCache> pMemCache,
        int skipInterval,
        int maxSkipLevel,
        IndexLevel indexLevel);

    ~RTPostingWriter();
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

    /** get last added doc id */
    docid_t lastDocID()
    {
        return nLastDocID_;
    }

    int32_t getSkipLevel();

    /*
     * for realtime search.
     */
    void getSnapShot(TermInfo& snapshot);

private:
    RTPostingWriter(const RTPostingWriter&);
    void operator=(const RTPostingWriter&);

    void doAdd(uint32_t docId, uint32_t pos);

    boost::shared_ptr<MemCache> pMemCache_;	/// memory cache
    int skipInterval_;              ///skip interval
    int maxSkipLevel_;           /// max skip level
    count_t nDF_;			///document frequency of this field
    docid_t nLastDocID_;	///current added doc id
    loc_t nLastLoc_;		///current added word offset
    count_t nCurTermFreq_; ///current term freq
    int32_t nCTF_;			///Collection's total term frequency
    count_t nMaxTermFreq_;
    boost::shared_ptr<VariantDataPool> pDocFreqList_; /// Doc freq list
    boost::shared_ptr<VariantDataPool> pLocList_; 	/// Location list
    SkipListWriter* pSkipListWriter_;	///skiplist writer
    IndexLevel indexLevel_;
    boost::shared_mutex mutex_;

    friend class MemPostingReader;
    friend class PostingMerger;
    friend class MemTermReader;
};


}

NS_IZENELIB_IR_END


#endif
