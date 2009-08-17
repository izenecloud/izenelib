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

    virtual ~TermDocFreqs();
public:
    freq_t	docFreq();

    int64_t	getCTF();

    count_t next(docid_t*& docs, count_t*& freqs);

    bool next();

    docid_t doc();

    count_t freq();

    freq_t docLength();

    void close();

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

    int32_t nTotalDecodedCount;	///size of decoded postings

    int32_t nCurDecodedCount;

    int32_t nCurrentPosting;	///position of current posting

    TermReader* pTermReader;

    InputDescriptor* pInputDescriptor;

    static const size_t DEFAULT_BUFFERSIZE = 32768;
};

}

NS_IZENELIB_IR_END

#endif
