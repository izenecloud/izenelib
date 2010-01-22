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

    docid_t getDoc() { return curDoc_;}

    fileoffset_t getOffset() { return lastOffset_; }
	
    fileoffset_t getPOffset() { return lastPOffset_; }

    int getNumLevels(){ return numSkipLevels_; }

    IndexInput* getLevelInput(int level)
    {
	assert(level >= 0&& level < numSkipLevels_);
	return skipStream_[level];
    }

    ///get skip interval of a certain levels
    int getLevelSkipInterval(int level)
    {
        int skipInterval = defaultSkipInterval_;
        for(int i = 0;i < level;i++)
            skipInterval = skipInterval * skipInterval;
        return skipInterval;
    }

    int getCurSkipInterval()	{ return curSkipInterval_; }

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
    int curSkipInterval_; ///skip interval of current skip point

    std::vector<docid_t> skipDoc_; ///doc id of current skip entry per level
    docid_t curDoc_; ///document of current skip point
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
}
}
NS_IZENELIB_IR_END

#endif

