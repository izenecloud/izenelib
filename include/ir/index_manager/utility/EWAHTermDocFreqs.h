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
#include <am/bitmap/ewah.h>
#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

template <typename word_t>
class EWAHTermDocFreqs : public TermDocFreqs
{
public:
    typedef izenelib::am::EWAHBoolArray<word_t> bitmap_t;
    typedef typename bitmap_t::const_iterator bitmap_iterator_t;

    EWAHTermDocFreqs(const boost::shared_ptr<const bitmap_t>& pBitMap)
        : TermDocFreqs()
        , pBitMap_(pBitMap)
        , curDoc_(BAD_DOCID)
        , iter_(pBitMap->begin())
        , iterEnd_(pBitMap->end())
        , setBitNum_(0)
        , hasInitSetBitNum_(false)
    {
    }

    EWAHTermDocFreqs(const EWAHTermDocFreqs& other)
        : TermDocFreqs(other)
        , pBitMap_(other.pBitMap_)
        , curDoc_(other.curDoc_)
        , iter_(other.iter_)
        , iterEnd_(other.iterEnd_)
        , setBitNum_(other.setBitNum_)
        , hasInitSetBitNum_(other.hasInitSetBitNum_)
    {
    }

    ~EWAHTermDocFreqs() {}

    void reset(PostingReader * pPosting, const TermInfo& ti, bool ownPosting) {}

    freq_t docFreq()
    {
        if (!hasInitSetBitNum_)
        {
            setBitNum_ = pBitMap_->numberOfOnes();
            hasInitSetBitNum_ = true;
        }

        return setBitNum_;
    }

    int64_t getCTF() { return 1; }

    count_t freq() { return 1; }

    docid_t doc() { return curDoc_; }

    docid_t skipTo(docid_t target)
    {
        while (next())
        {
            if (curDoc_ >= target)
                break;
        }

        return curDoc_;
    }

    bool next()
    {
        if (iter_ != iterEnd_)
        {
            curDoc_ = *iter_;
            ++iter_;
            return true;
        }
        else
        {
            curDoc_ = BAD_DOCID;
            return false;
        }
    }

private:
    boost::shared_ptr<const bitmap_t> pBitMap_;

    docid_t curDoc_;

    bitmap_iterator_t iter_;
    const bitmap_iterator_t iterEnd_;

    std::size_t setBitNum_;
    bool hasInitSetBitNum_; // whether member "setBitNum_" has been initialized
};

}
NS_IZENELIB_IR_END

#endif /* EWAH_TERM_DOC_FREQS_H */
