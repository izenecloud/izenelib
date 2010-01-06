/**
* @file        SkipListReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Reading skiplist data
*/

#ifndef SKIPLIST_MERGER_H
#define SKIPLIST_MERGER_H

#include <ir/index_manager/index/SkipListWriter.h>
#include <ir/index_manager/index/SkipListReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/**
 * @brief SkipListMerger
 */
class SkipListMerger : public SkipListWriter
{
public:
    SkipListMerger(int skipInterval, int maxLevel, MemCache* pMemCache);

    virtual ~SkipListMerger();

public:
    bool addToMerge(SkipListReader* pSkipReader,docid_t lastDoc,int nAppendedSkipInterval);

    void writeSkipData(int level,IndexOutput* pSkipLevelOutput);

    void reset();
private:
    bool setSkipPoint(SkipListReader* pSkipReader,int nAppendedSkipInterval);

private:
    docid_t baseDocID_;
    fileoffset_t baseOffset_;
    fileoffset_t basePOffset_;		

    int* pSkipInterval_; ///current skip interval of each skip level

};

}
NS_IZENELIB_IR_END

#endif


