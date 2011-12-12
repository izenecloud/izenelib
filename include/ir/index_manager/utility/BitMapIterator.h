/**
 * \file BitMapIterator.h
 * \brief
 * \date Nov 8, 2011
 * \author Xin Liu
 */

#ifndef BITMAPITERATOR_H_
#define BITMAPITERATOR_H_

#include <ir/index_manager/index/TermDocFreqs.h>
#include <am/bitmap/Ewah.h>

#define MAX_DOC_ID      0xFFFFFFFF

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class BitMapIterator : public TermDocFreqs
{
public:

    BitMapIterator(const boost::shared_ptr<EWAHBoolArray<uint32_t> >& pDocIdSet)
            : TermDocFreqs()
            , pDocIdSet_(pDocIdSet)
            , bitmapIter_(pDocIdSet->bit_iterator())
    {
    }

    BitMapIterator(const BitMapIterator& other)
            : TermDocFreqs(other)
            , bitmapIter_(other.bitmapIter_)
    {
    }

    ~BitMapIterator()
    {
    }
public:
    void reset(PostingReader * pPosting,const TermInfo& ti,bool ownPosting = true) {}

    freq_t docFreq()
    {
        return bitmapIter_.numberOfOnes();
    }

    int64_t getCTF()
    {
        return 1;
    }

    count_t freq()
    {
        return 1;
    }

    docid_t doc()
    {
        return bitmapIter_.getCurr();
    }

    docid_t skipTo(docid_t target)
    {
        docid_t currDoc;
        do
        {
            if (!bitmapIter_.next())
                return MAX_DOC_ID;
            currDoc = doc();
        }
        while (target > currDoc);

        return currDoc;
    }

    bool next()
    {
        return bitmapIter_.next();
    }

private:
    boost::shared_ptr<EWAHBoolArray<uint32_t> > pDocIdSet_;
    izenelib::am::EWAHBoolArrayBitIterator<uint32_t> bitmapIter_;
};

}
NS_IZENELIB_IR_END

#endif /* BITMAPITERATOR_H_ */
