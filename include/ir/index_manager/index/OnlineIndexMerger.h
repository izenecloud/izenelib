/**
* @file        OnlineIndexMergerr.h
* @version     SF1 v5.0
* @brief DBT index merge algorithm
*/
#ifndef OnlineIndexMerger_H
#define OnlineIndexMerger_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/store/Directory.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* The class has implemented the DBT index merge algorithm
*/
class OnlineIndexMerger :	public IndexMerger
{
public:
    class DBTLayer
    {
    public:
        DBTLayer(int32_t l,int32_t nLevelSize_,int32_t nMaxSize)
                :nLevel(l)
                ,nMergeTimes(0)
                ,nLevelSize(nLevelSize_)
        {
            std::string s = "_mid_";
            s = append(s,l);
            s += "_";
            s = append(s,nMergeTimes);
            pMergeBarrel = new MergeBarrel(s.c_str(),nMaxSize);
        }
        ~DBTLayer()
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
            std::string s = "_mid_";
            s = append(s,nLevel);
            s += "_";
            s = append(s,nMergeTimes);
            pMergeBarrel->setIdentifier(s);
        }
    private:
        int32_t nLevel;				///level of this node

        MergeBarrel* pMergeBarrel;		///index merge barrel

        int32_t nMergeTimes;		///merge times

        int32_t nLevelSize;			///size of level

        friend class OnlineIndexMerger;
    };

public:
    OnlineIndexMerger(Directory* pSrcDirectory);

    OnlineIndexMerger(Directory* pSrcDirectory,char* buffer,size_t bufsize);

    virtual ~OnlineIndexMerger(void);
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

    /**
     * set parameter of merger
     * @param pszParam parameter string, format:name1=value1;param2=value2
     */
    void setParam(const char* pszParam);

private:
    /**
     * determine the level of index barrel.
     * @param nLevelSize size of level
     * @param m M parameter
     */
    int32_t getLevel(int64_t nLevelSize,int32_t m);

    /**
     * trigger a merge
     * @param pLevel the level need to be merged
     * @param nLevel the level number
     */
    void triggerMerge(DBTLayer* pLevel,int32_t nLevel);
private:
    const static int32_t MAX_TRIGGERS = 5;

    int32_t nM;				///
    int32_t nS;				///scale factor
    int32_t nC;				///collision factor, when there are nC barrels in a same level, a merge will be trigged.
    int32_t nCurLevelSize;	///size of level

    std::map<int32_t,DBTLayer*>	nodesMap;
};

}

NS_IZENELIB_IR_END

#endif
