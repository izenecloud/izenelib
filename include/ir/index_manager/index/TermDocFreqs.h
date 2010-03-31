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
#include <ir/index_manager/index/Posting.h>

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

    TermDocFreqs(TermReader * pReader,InputDescriptor * pInputDescriptor,TermInfo& ti);

    TermDocFreqs(TermReader * pReader,Posting * pPosting,TermInfo& ti);

    TermDocFreqs(Posting* pPosting);

    virtual ~TermDocFreqs();
public:
    virtual freq_t docFreq();

    virtual int64_t getCTF();

    virtual count_t next(docid_t*& docs, count_t*& freqs);

    virtual bool next();

    virtual docid_t doc();

    virtual count_t freq();

    virtual void close();

protected:

    virtual bool decode();

    void createBuffer();

protected:
    TermInfo termInfo_;

    Posting* pPosting_;

    uint32_t* pPostingBuffer_;	///buffer for store decoded postings

    size_t nBufferSize_;		///the size of buffer.

    size_t nFreqStart_;

    int32_t nTotalDecodedCount_;	///size of decoded postings

    int32_t nCurDecodedCount_;

    int32_t nCurrentPosting_;	///position of current posting

    TermReader* pTermReader_;

    InputDescriptor* pInputDescriptor_;

    bool ownPosting_;

    static const size_t DEFAULT_BUFFERSIZE = 131072;
};

}

NS_IZENELIB_IR_END

#endif
