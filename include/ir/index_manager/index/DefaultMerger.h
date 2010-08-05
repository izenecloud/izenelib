/**
* @file        DefaultMerger.h
* @version     SF1 v5.0
* @brief Default index merge algorithm
*/
#ifndef DEFAULT_MERGER_H
#define DEFAULT_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

class DefaultMerger : public IndexMerger
{
public:
    class Layer
    {
    public:
        Layer(int l,int nLevelSize,int nMaxSize)
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
        ~Layer()
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

        friend class DefaultMerger;
    };

public:

    DefaultMerger(Indexer* pIndexer);

    virtual ~DefaultMerger();
public:

    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();

private:

    int getLevel(int64_t nLevelSize);

    void triggerMerge(Layer* pLevel,int nLevel);

private:
    std::map<int,int> nCMap_; ///collision factor, when there are nC_ barrels in a same level, a merge will be trigged.

    int nCurLevelSize_; ///size of level

    std::map<int,Layer*> nodesMap_;

    int num_doc_per_barrel_;
};

}

NS_IZENELIB_IR_END


#endif

