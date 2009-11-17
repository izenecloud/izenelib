#ifndef GP_MERGER_H
#define GP_MERGER_H

#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/utility/StringUtils.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class GPartitionMerger : public IndexMerger
{
public:
    class Partition
    {
    public:
        Partition(int32_t p,int32_t nPSize,int32_t nMaxSize)
                :partition_(p)
                ,mergeTimes_(0)
                ,nPartitionSize_(nPSize)
        {
            std::string s = "_mid_";
            s = append(s,p);
            s += "_";
            s = append(s,mergeTimes_);
            pMergeBarrel_ = new MergeBarrel(s.c_str(),nMaxSize);
        }
        ~Partition()
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
            mergeTimes_++;
            std::string s = "_mid_";
            s = append(s,partition_);
            s += "_";
            s = append(s,mergeTimes_);
            pMergeBarrel_->setIdentifier(s);
        }
    protected:
        int32_t partition_;		///partition of this sub-index
        MergeBarrel* pMergeBarrel_;		///index merge barrel
        int32_t mergeTimes_;		///merge times
        int32_t nPartitionSize_;	///size of partition

        friend class GPartitionMerger;
    };
public:
    GPartitionMerger(Indexer* pIndexer);

    virtual ~GPartitionMerger(void);
public:
    void addBarrel(MergeBarrelEntry* pEntry);

    void endMerge();

protected:
    int32_t getPartition(int32_t nPartSize);

    void triggerMerge(Partition* pPartition,int32_t p);
protected:
    const static int32_t MAX_TRIGGERS = 5;
    int32_t nR_; ///parameter r, used it to define the capacity of the partitions
    int32_t nP_; ///parameter p, number of partitions
    int32_t curPartitionSize_;
    map<int32_t,Partition*>	partitionMap_;
};

}

NS_IZENELIB_IR_END

#endif

