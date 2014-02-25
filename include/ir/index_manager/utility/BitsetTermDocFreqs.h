/**
 * @file BitsetTermDocFreqs.h
 * @brief an implementation of TermDocFreqs based on Bitset.
 */

#ifndef BITSET_TERM_DOC_FREQS_H
#define BITSET_TERM_DOC_FREQS_H

#include "Bitset.h"
#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/utility/system.h>
#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class BitsetTermDocFreqs : public TermDocFreqs
{
public:
    BitsetTermDocFreqs(const boost::shared_ptr<const Bitset>& pBitset)
        : TermDocFreqs()
        , pBitset_(pBitset)
        , curDoc_(0)
        , setBitNum_(0)
        , hasInitSetBitNum_(false)
    {
    }

    BitsetTermDocFreqs(const BitsetTermDocFreqs& other)
        : TermDocFreqs(other)
        , pBitset_(other.pBitset_)
        , curDoc_(other.curDoc_)
        , setBitNum_(other.setBitNum_)
        , hasInitSetBitNum_(other.hasInitSetBitNum_)
    {
    }

    void reset(PostingReader * pPosting, const TermInfo& ti, bool ownPosting) {}

    freq_t docFreq()
    {
        if (!hasInitSetBitNum_)
        {
            setBitNum_ = pBitset_->count();
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
        if (curDoc_ == BAD_DOCID)
            return false;

        curDoc_ = curDoc_ ?
                pBitset_->find_next(curDoc_) :
                pBitset_->find_first();

        return curDoc_ != BAD_DOCID;
    }

private:
    boost::shared_ptr<const Bitset> pBitset_;

    docid_t curDoc_;

    std::size_t setBitNum_;
    bool hasInitSetBitNum_; // whether member "setBitNum_" has been initialized
};

}
NS_IZENELIB_IR_END

#endif /* BITSET_TERM_DOC_FREQS_H */
