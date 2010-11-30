#include <ir/index_manager/index/GPartitionPolicy.h>

using namespace izenelib::ir::indexmanager;

GPartitionPolicy::GPartitionPolicy()
        :nR_(3)
        ,nP_(0)
        ,curPartitionSize_(1)
{
}

GPartitionPolicy::~GPartitionPolicy(void)
{
    endMerge();
}
void GPartitionPolicy::addBarrel(MergeBarrelEntry* pEntry)
{
    Partition* pPartition = NULL;
    int32_t nPartition = getPartition(curPartitionSize_);
    map<int32_t,Partition*>::iterator iter = partitionMap_.find(nPartition);
    if (iter != partitionMap_.end())
    {
        pPartition = iter->second;
        pPartition->nPartitionSize_ += curPartitionSize_;///update partition size
        pPartition->add(pEntry);
        if ((int32_t)pPartition->pBarrelQueue_->size() >= 2) ///collision,trigger a merge event
        {
            triggerMerge(pPartition,nPartition);
        }
    }
    else
    {
        pPartition = new Partition(nPartition,curPartitionSize_,2*MAX_TRIGGERS);
        pPartition->add(pEntry);
        partitionMap_.insert(make_pair(nPartition,pPartition));
    }
}
void GPartitionPolicy::endMerge()
{
    map<int32_t,Partition*>::iterator iter = partitionMap_.begin();
    while (iter != partitionMap_.end())
    {
        delete iter->second;
        iter++;
    }
    partitionMap_.clear();
}

void GPartitionPolicy::triggerMerge(Partition* pPartition,int32_t p)
{
    Partition* pPartition1 = pPartition;
    int32_t nP = this->getPartition(pPartition->nPartitionSize_);
    int32_t nTriggers = 0;
    for (int32_t i = p + 1;(i <= nP) && (nTriggers <= MAX_TRIGGERS);i++)
    {
        map<int32_t,Partition*>::iterator iter = partitionMap_.find(i);
        if (iter != partitionMap_.end())
        {
            Partition* pPartition2 = iter->second;
            if ((int32_t)pPartition2->pBarrelQueue_->size() + 1 >= 2)	///will trigger a merge event in upper partition
            {
                ///copy elements to upper partition
                while (pPartition1->pBarrelQueue_->size() >0)
                {
                    pPartition2->pBarrelQueue_->put(pPartition1->pBarrelQueue_->pop());
                }
                pPartition2->nPartitionSize_ += pPartition1->nPartitionSize_;
                pPartition1->nPartitionSize_ = 0;
                nP = this->getPartition(pPartition2->nPartitionSize_);
                pPartition1 = pPartition2;
                nTriggers++;
            }
            else break;
        }
        else break;
    }

    if (pPartition1->pBarrelQueue_->size() > 0)
    {
        curPartitionSize_ = pPartition1->nPartitionSize_;
        pPartition1->nPartitionSize_ = 0;
        pIndexMerger_->mergeBarrel(pPartition1->pBarrelQueue_);
        pPartition1->increaseMergeTimes();
        curPartitionSize_ = 1;
    }
}
int32_t GPartitionPolicy::getPartition(int32_t nPartSize)
{
    if (nPartSize <= 0)
        return 0;
    int32_t nPartition = 0;
    ///determine its level
    while (nPartSize > (nR_ - 1)*((int32_t)pow((double)nR_,(double)nPartition)) )
    {
        nPartition++;
    }
    return nPartition;
}


