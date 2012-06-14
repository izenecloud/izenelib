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

    TermDocFreqs(
        PostingReader * pPosting,
        const TermInfo& ti,
        bool ownPosting = true);

    virtual ~TermDocFreqs();
public:
    virtual void reset(
        PostingReader * pPosting,
        const TermInfo& ti,
        bool ownPosting = true);

    virtual freq_t docFreq();

    virtual int64_t getCTF();

    virtual int32_t getMaxTF();

    virtual docid_t skipTo(docid_t docId);

    virtual bool next();

    virtual docid_t doc();

    virtual count_t freq();

    virtual void close();

protected:

    virtual bool decode();

    void createBuffer();

    int32_t bsearch(
        docid_t docs[],
        int32_t start,
        int32_t end,
        docid_t key,
        docid_t& keyFound);

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

inline freq_t TermDocFreqs::docFreq()
{
    return termInfo_.docFreq_;
}

inline int64_t TermDocFreqs::getCTF()
{
    return termInfo_.ctf_;
}

inline int32_t TermDocFreqs::getMaxTF(){
    return termInfo_.maxTF_;
}

inline docid_t TermDocFreqs::doc()
{
    return pPostingBuffer_[nCurrentPosting_];
}

inline count_t TermDocFreqs::freq()
{
    return pPostingBuffer_[nFreqStart_ + nCurrentPosting_];
}

inline bool TermDocFreqs::next()
{
    nCurrentPosting_ ++;
    if (nCurrentPosting_ >= nCurDecodedCount_)
    {
        if (!decode())
            return false;
        return true;
    }
    return true;
}

inline bool TermDocFreqs::decode()
{
    count_t df = termInfo_.docFreq();
    if ((count_t)nTotalDecodedCount_ == df)
    {
        return false;
    }

    if (!pPostingBuffer_)
        createBuffer();

    nCurrentPosting_ = 0;
    nCurDecodedCount_ = pPosting_->DecodeNext(pPostingBuffer_,nBufferSize_, nMaxDocCount_);
    if (nCurDecodedCount_ <= 0)
        return false;
    nTotalDecodedCount_ += nCurDecodedCount_;

    return true;
}

}

NS_IZENELIB_IR_END

#endif
