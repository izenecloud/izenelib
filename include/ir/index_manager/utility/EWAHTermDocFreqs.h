/**
 * \file EWAHTermDocFreqs.h
 * \brief an implementation of TermDocFreqs based on EWAHBoolArray.
 * \date Nov 8, 2011
 * \author Xin Liu
 */

#ifndef EWAH_TERM_DOC_FREQS_H
#define EWAH_TERM_DOC_FREQS_H

#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/utility/system.h>
#include <am/bitmap/Ewah.h>
#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

template <typename word_t>
class EWAHTermDocFreqs : public TermDocFreqs
{
public:
    typedef izenelib::am::EWAHBoolArray<word_t> bitmap_t;
    typedef izenelib::am::EWAHBoolArrayBitIterator<word_t> bitmap_iterator_t;

    EWAHTermDocFreqs(const boost::shared_ptr<bitmap_t>& pBitMap)
        : TermDocFreqs()
        , pBitMap_(pBitMap)
        , bitmapIter_(pBitMap->bit_iterator())
    {
    }

    EWAHTermDocFreqs(const EWAHTermDocFreqs& other)
        : TermDocFreqs(other)
        , bitmapIter_(other.bitmapIter_)
    {
    }

    ~EWAHTermDocFreqs() {}

    void reset(PostingReader * pPosting, const TermInfo& ti, bool ownPosting) {}

    freq_t docFreq() { return bitmapIter_.numberOfOnes(); }

    int64_t getCTF() { return 1; }

    count_t freq() { return 1; }

    docid_t doc() { return bitmapIter_.getCurr(); }

    docid_t skipTo(docid_t target)
    {
        docid_t currDoc;
        do
        {
            if (!bitmapIter_.next())
                return BAD_DOCID;
            currDoc = doc();
        }
        while (target > currDoc);

        return currDoc;
    }

    bool next() { return bitmapIter_.next(); }

private:
    boost::shared_ptr<bitmap_t> pBitMap_;
    bitmap_iterator_t bitmapIter_;
};

}
NS_IZENELIB_IR_END

#endif /* EWAH_TERM_DOC_FREQS_H */
