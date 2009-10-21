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

    virtual freq_t docLength();

    virtual void close();

protected:

    virtual bool decode();

    /** create buffer for decoding */
    void createBuffer();

protected:
    TermInfo termInfo;

    Posting* pPosting;

    uint32_t* pPostingBuffer;	///buffer for store decoded postings

    size_t nBufferSize;		///the size of buffer.

    size_t nFreqStart;

    size_t nDocLenStart;

    int32_t nTotalDecodedCount;	///size of decoded postings

    int32_t nCurDecodedCount;

    int32_t nCurrentPosting;	///position of current posting

    TermReader* pTermReader;

    InputDescriptor* pInputDescriptor;

    static const size_t DEFAULT_BUFFERSIZE = 49152;
};

}

NS_IZENELIB_IR_END

#endif
