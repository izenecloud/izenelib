/**
* @file        SkipListReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Reading skiplist data
*/

#ifndef SKIPLIST_READER_H
#define SKIPLIST_READER_H

#include <ir/index_manager/index/VariantDataPool.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/store/IndexInput.h>

#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/**
 * @brief SkipListReader
 */
class SkipListReader
{
public:
    SkipListReader(IndexInput* pSkipInput, int skipInterval, int numSkipLevels);

    SkipListReader(VariantDataPool** pSkipLevels, int skipInterval, int numSkipLevels);

    virtual ~SkipListReader();

public:
    docid_t skipTo(docid_t docID);

    ///iterating skip points of lowest level
    bool nextSkip(docid_t docID);

    docid_t getDoc() { return lastDoc_;}

    fileoffset_t getOffset() { return lastOffset_; }
	
    fileoffset_t getPOffset() { return lastPOffset_; }

    int getNumLevels(){ return numSkipLevels_; }

    ///get skip interval of a certain levels
    int getLevelSkipInterval(int level)
    {
        int skipInterval = defaultSkipInterval_;
        for(int i = 0;i < level;i++)
            skipInterval = skipInterval * defaultSkipInterval_;
        return skipInterval;
    }

    int getSkipInterval()
    {
        return lastSkipInterval_;
    }

    int getNumSkipped() { return totalSkipped_; }

private:
    inline void init();

    void seekChild(int level);

    void loadSkipLevels();

    bool loadNextSkip(int level);

    void readSkipPoint(int level,IndexInput* pLevelInput);

private:
    bool loaded_;
    int defaultSkipInterval_;
    int numSkipLevels_;	 ///number of skip levels

    std::vector<IndexInput*> skipStream_; /// skipStream for each level

    std::vector<fileoffset_t> skipPointer_; /// the start pointer of each skip level
    std::vector<int> skipInterval_; ///skip interval in each level
    std::vector<int> numSkipped_; ///number of skipped document per level 
    int totalSkipped_; ///total skipped document 

    std::vector<docid_t> skipDoc_; ///doc id of current skip entry per level
    docid_t lastDoc_; ///document of current skip point
    int lastSkipInterval_;///last skip interval
    std::vector<fileoffset_t> childPointer_; ///current child pointer of each level (except level 0) 
    fileoffset_t lastChildPointer_; ///child pointer of current skip point
    std::vector<fileoffset_t> offsets_; ///offset of this point in posting (relative  to the begin of the posting)	
    fileoffset_t lastOffset_;
    std::vector<fileoffset_t> pOffsets_; ///offset of the first doc's position in position posting.		
    fileoffset_t lastPOffset_;
};


inline void SkipListReader::init()
{
    skipDoc_.assign(numSkipLevels_, 0);
    skipInterval_.assign(numSkipLevels_, 0);
    numSkipped_.assign(numSkipLevels_, 0);
    childPointer_.assign(numSkipLevels_, 0);
    skipPointer_.assign(numSkipLevels_, 0);
    offsets_.assign(numSkipLevels_, 0);
    pOffsets_.assign(numSkipLevels_, 0);
    skipStream_.resize(numSkipLevels_,NULL);
    skipInterval_.assign(numSkipLevels_, 0);
//    for(int i = 0; i < numSkipLevels_; i++)
//        skipInterval_[i] = getLevelSkipInterval(i);
}
}
NS_IZENELIB_IR_END

#endif

