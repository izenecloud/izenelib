/**
* @file        OptimizeMergerMergerr.h
* @version     SF1 v5.0
* @brief OPT index merge algorithm
*/
#ifndef OPTIMIZEINDEXMERGER_H
#define OPTIMIZEINDEXMERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* This class has implemented the optimize index merge algorithm that will merge all the barrels into a single index barrel
*/
class OptimizeMerger : public IndexMerger
{
public:
    class OptimizeMergeTreeLevel
    {
    public:
        OptimizeMergeTreeLevel(int32_t l,int32_t nLevelSize_,int32_t nMaxSize)
                :nLevel(l)
                ,nMergeTimes(0)
                ,nLevelSize(nLevelSize_)
        {
            string s = "_mid_";
            s = append(s,l);
            s += "_";
            s = append(s,nMergeTimes);
            pMergeBarrel = new MergeBarrel(s.c_str(),nMaxSize);
        }
        ~OptimizeMergeTreeLevel()
        {
            delete pMergeBarrel;
            pMergeBarrel = NULL;
        }
    public:
        void add(MergeBarrelEntry* pEntry)
        {
            pMergeBarrel->put(pEntry);
        }
        void	increaseMergeTimes()
        {
            nMergeTimes++;
            string s = "_mid_";
            s = append(s,nLevel);
            s += "_";
            s = append(s,nMergeTimes);
            pMergeBarrel->setIdentifier(s);
        }
    protected:
        int32_t nLevel;			///level of this node
        MergeBarrel* pMergeBarrel;		///index merge barrel
        int32_t nMergeTimes;		///merge times
        int32_t nLevelSize;		///size of level
        friend class OptimizeMerger;
    };
public:
    OptimizeMerger(Directory* pSrcDirectory);
    OptimizeMerger(Directory* pSrcDirectory,char* buffer,size_t bufsize);
    virtual ~OptimizeMerger(void);
public:
    /**
     * add new index barrel to merge,derived classes implement it,and could apply some merge strategies.
     * @param pEntry the index barrel,derived classes are responsible for deleting
     */
    void addBarrel(MergeBarrelEntry* pEntry);

    /**
     * the merge is over,derived classes implement it,and could clear some resources for merging.
     */
    void endMerge();
protected:
    /**
     * determine the level number.
     * @param nLevelSize size of level
     */
    int32_t getLevel(int64_t nLevelSize);
protected:
    int32_t nC;		///collision factor, when there are C barrels , a merge will be trigged.
    int32_t nScale;
    int32_t nCurLevelSize;	///size of level
    map<int32_t,OptimizeMergeTreeLevel*> levelsMap;
};

}

NS_IZENELIB_IR_END

#endif

