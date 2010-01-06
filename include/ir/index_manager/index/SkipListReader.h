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

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/**
 * @brief SkipListReader
 */
class SkipListReader
{
public:
    SkipListReader(IndexInput* pSkipInput, int skipInterval, int numSkipLevels);

    virtual ~SkipListReader();

public:
    docid_t skipTo(docid_t docID);

    docid_t getDoc() { return curDoc_;}

    fileoffset_t getOffset() { return lastOffset_; }
	
    fileoffset_t getPOffset() { return lastPOffset_; }

    int getNumLevels(){ return numSkipLevels_; }

    IndexInput* getLevelInput(int level)
    {
	assert(level >= 0&& level < numSkipLevels_);
	return skipStream_[level];
    }

    int getLevelSkipInterval(int level)
    {
        int skipInterval = defaultSkipInterval_;
        for(int i = 0;i < level;i++)
            skipInterval = skipInterval * skipInterval;
        return skipInterval;
    }

    int getCurSkipInterval()	{ return curSkipInterval_; }

    void reset();

private:
    void seekChild(int level);

    void loadSkipLevels();

    bool loadNextSkip(int level);

    docid_t readSkipPoint(int level,IndexInput* pLevelInput);

private:
    bool loaded_;
    int defaultSkipInterval_;
    int numSkipLevels_;	 ///number of skip levels

    IndexInput** skipStream_; /// skipStream for each level

    fileoffset_t* skipPointer_; /// the start pointer of each skip level
    int* skipInterval_; ///skip interval in each level
    int* numSkipped_; ///number of skipped document per level 
    int nNumSkipped_; ///number of skipped document per level 
    int curSkipInterval_; ///skip interval of current skip point

    docid_t* skipDoc_; ///doc id of current skip entry per level
    docid_t curDoc_; ///document of current skip point
    fileoffset_t* childPointer_; ///current child pointer of each level (except level 0) 
    fileoffset_t lastChildPointer_; ///child pointer of current skip point
    fileoffset_t* offsets_; ///offset of this point in posting (relative  to the begin of the posting)	
    fileoffset_t lastOffset_;
    fileoffset_t* pOffsets_; ///offset of the first doc's position in position posting.		
    fileoffset_t lastPOffset_;

};

}
NS_IZENELIB_IR_END

#endif

