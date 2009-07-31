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
    /**
     * get document frequency
     * @return frequency
     */
    freq_t	docFreq();
    /**
     * get collection's total term frequency
     * @return CTF value
     */
    int64_t	getCTF();
    /**
     * move to the next documents block
     * @param docs documents block
     * @param freqs term frequency block
     * @return number of documents in the block
     */
    count_t next(docid_t*& docs, count_t*& freqs);
    /**
     * move to the next document
     */
    bool next();
    /**
     * get document id
     * @return document id
     */
    docid_t doc();
    /**
     * get term frequency in this document
     */
    count_t freq();

    void close();

protected:

    virtual bool decode();

    int32_t bsearch(docid_t docs[],int32_t start,int32_t end,docid_t key,docid_t& keyFound);

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
////////////////////////////////////////////////////////////////////////////////
//
inline int32_t TermDocFreqs::bsearch(docid_t docs[],int32_t start,int32_t end,docid_t key,docid_t& keyFound)
{
    int32_t k;
    int32_t nk = end;
    keyFound = docs[end];
    while (start<=end)
    {
        k = (start + end)/2;
        if (key == docs[k])
        {
            keyFound = key;
            return k;
        }
        if (key < docs[k])
        {
            end = k - 1;
            if (k >= start)
            {
                keyFound = docs[k];
                nk =k;
            }
        }
        else
        {
            start = k + 1;
            if (start <= end)
            {
                if (docs[start] > key)
                {
                    keyFound = docs[start];
                    nk = start;
                }
            }
        }
    }
    return nk;
}

}

NS_IZENELIB_IR_END

#endif
