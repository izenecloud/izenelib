/**
 * \file BitMapIterator.h
 * \brief 
 * \date Aug 8, 2011
 * \author Xin Liu
 */

#ifndef BITMAPITERATOR_H_
#define BITMAPITERATOR_H_

#include <ir/index_manager/index/TermDocFreqs.h>

#define MAX_DOC_ID      0xFFFFFFFF

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BitMapIterator : public TermDocFreqs {
public:

        BitMapIterator(const vector<docid_t> & idList):
            TermDocFreqs(),
            docIdList_(idList),
            curPos_(-1)
        {
        }

        BitMapIterator(const BitMapIterator& other):TermDocFreqs(other),
             docIdList_(other.docIdList_),
             curPos_(other.curPos_)
        {
        }

         ~BitMapIterator()
         {
             curPos_ = -1;
             docIdList_.clear();
         }
public:
        void reset(PostingReader * pPosting,const TermInfo& ti,bool ownPosting = true){}

        freq_t docFreq() { return docIdList_.size(); }

        int64_t getCTF() { return 1; }

        count_t freq() { return 1; }

        docid_t doc() { return docIdList_[curPos_]; }

        docid_t skipTo(docid_t target){
            docid_t currDoc;
            do
            {
                if(!next())
                    return MAX_DOC_ID;
                currDoc = doc();
            } while(target > currDoc);

            return currDoc;
        }

        bool next()
        {
            curPos_++;
            if (curPos_ < (int)docIdList_.size())
            {
                return true;
            }
            return false;
        }

private:
        std::vector<docid_t> docIdList_;
        int curPos_;
};

}

NS_IZENELIB_IR_END

#endif /* BITMAPITERATOR_H_ */
