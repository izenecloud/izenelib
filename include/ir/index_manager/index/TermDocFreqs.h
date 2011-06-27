/**
* @file        TermDocFreqs.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Iterate the index of term document frequency posting (*.dfp)
*/
#ifndef TERMDOCS_H
#define TERMDOCS_H

#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/PostingReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class TermReader;
class Term;
/**
* Iterate the index of term document frequency posting (*.dfp)
* TermDocFreqs iterate the posting of a single barrel
*/
class TermDocFreqs
{
public:
    TermDocFreqs();

    TermDocFreqs(PostingReader * pPosting,const TermInfo& ti,bool ownPosting = true);

    virtual ~TermDocFreqs();
public:
    virtual void reset(PostingReader * pPosting,const TermInfo& ti,bool ownPosting = true);

    virtual freq_t docFreq();

    virtual int64_t getCTF();

    virtual docid_t skipTo(docid_t docId);	

    virtual bool next();

    virtual docid_t doc();

    virtual count_t freq();

    virtual void close();

protected:

    virtual bool decode();

    void createBuffer();

    int32_t bsearch(docid_t docs[],int32_t start,int32_t end,docid_t key,docid_t& keyFound);

protected:
    TermInfo termInfo_;

    PostingReader* pPosting_;

    uint32_t* pPostingBuffer_;  ///buffer for store decoded postings

    size_t nBufferSize_;        ///the buffer size allocated for @p pPostingBuffer_

    size_t nMaxDocCount_;       /// max number of decoded docs @p pPostingBuffer_ could store

    size_t nFreqStart_;         /// the start position of decoded frequency in @p pPostingBuffer_

    int32_t nTotalDecodedCount_;	///size of decoded postings

    int32_t nCurDecodedCount_;

    int32_t nCurrentPosting_;	///position of current posting

    bool ownPosting_;
};

}

NS_IZENELIB_IR_END

#endif
