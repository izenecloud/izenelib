/**
 * @brief mock index merger to test index merge policies
 * @author Yingfeng Zhang
 * @date 2010-11-10
 */
#ifndef MOCK_MERGER_H
#define MOCK_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class MockMerger
{
public:
    class BTLayer
    {
    public:
        BTLayer(int l,int nLevelSize,int nMaxSize)
                :nLevel_(l)
                ,nMergeTimes_(0)
                ,nLevelSize_(nLevelSize)
        {
            std::string s = "_mid_";
            s = append(s,l);
            s += "_";
            s = append(s,nMergeTimes_);
            pMergeBarrel_ = new MergeBarrel(s.c_str(),nMaxSize);
        }
        ~BTLayer()
        {
            delete pMergeBarrel_;
            pMergeBarrel_ = NULL;
        }
    public:
        void add(MergeBarrelEntry* pEntry)
        {
            pMergeBarrel_->put(pEntry);
        }

        void	increaseMergeTimes()
        {
            nMergeTimes_++;
            std::string s = "_mid_";
            s = append(s,nLevel_);
            s += "_";
            s = append(s,nMergeTimes_);
            pMergeBarrel_->setIdentifier(s);
        }
    private:
        int nLevel_;				///level of this node

        MergeBarrel* pMergeBarrel_;		///index merge barrel

        int nMergeTimes_;		///merge times

        int nLevelSize_;			///size of level

        friend class MockMerger;
    };

public:

    MockMerger(BarrelsInfo* pBarrelsInfo,Directory* pDirectory);

    virtual ~MockMerger();

public:
    void addToMerge(BarrelInfo* pBarrelInfo);

    void merge();
private:

    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();

    int getLevel(int64_t nLevelSize);

    void triggerMerge(BTLayer* pLevel,int nLevel);

    int getC(int nLevel);

    void mergeBarrel(MergeBarrel* pBarrel);

private:
    BarrelsInfo* pBarrelsInfo_;

    Directory* pDirectory_;

    std::vector<MergeBarrelEntry*>* pMergeBarrels_;

    bool triggerMerge_;

    std::map<int,int> nCMap_; ///collision factor, when there are nC_ barrels in a same level, a merge will be trigged.

    int nCurLevelSize_; ///size of level

    std::map<int,BTLayer*> nodesMap_;

    int num_doc_per_barrel_;
};

}

NS_IZENELIB_IR_END

#endif

