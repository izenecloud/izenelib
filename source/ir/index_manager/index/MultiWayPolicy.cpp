#include <ir/index_manager/index/MultiWayPolicy.h>

using namespace izenelib::ir::indexmanager;

MultiWayPolicy::MultiWayPolicy()
        :curGeneration_(0)
{
}

MultiWayPolicy::~MultiWayPolicy(void)
{
    endMerge();
}
void MultiWayPolicy::addBarrel(MergeBarrelEntry* pEntry)
{
    MultiWayPolicy::Generation* pGen = NULL;
    map<int,MultiWayPolicy::Generation*>::iterator iter = generationMap_.find(curGeneration_);
    if (iter != generationMap_.end())
    {
        pGen = iter->second;
        pGen->add(pEntry);
        if ((int)pGen->pBarrelQueue_->size() >= 2) ///collision,trigger a merge event
        {
            triggerMerge(pGen,curGeneration_);
        }
    }
    else
    {
        pGen = new MultiWayPolicy::Generation(curGeneration_,2*MAX_TRIGGERS);
        pGen->add(pEntry);
        generationMap_.insert(make_pair(curGeneration_,pGen));
    }
}
void MultiWayPolicy::endMerge()
{
    map<int,MultiWayPolicy::Generation*>::iterator iter = generationMap_.begin();
    while (iter != generationMap_.end())
    {
        delete iter->second;
        iter++;
    }
    generationMap_.clear();
}

void MultiWayPolicy::triggerMerge(MultiWayPolicy::Generation* pGen,int nGen)
{
    MultiWayPolicy::Generation* pGen1 = pGen;
    int nCurGen = nGen;
    for (int i = nGen + 1;i <= (nGen + MAX_TRIGGERS);i++)
    {
        map<int,MultiWayPolicy::Generation*>::iterator iter2 = generationMap_.find(i);
        if (iter2 != generationMap_.end())
        {
            MultiWayPolicy::Generation* pGen2 = iter2->second;
            if ((int)pGen2->pBarrelQueue_->size() + 1 >= 2)	///will trigger a merge event in upper generation
            {
                ///copy elements to upper generation
                while (pGen1->pBarrelQueue_->size() >0)
                {
                    pGen2->pBarrelQueue_->put(pGen1->pBarrelQueue_->pop());
                }
                pGen1 = pGen2;
                nCurGen = i;
            }
            else break;
        }
        else break;
    }

    if (pGen1->pBarrelQueue_->size() > 0)
    {
        curGeneration_ = nCurGen + 1;
        pIndexMerger_->mergeBarrel(pGen1->pBarrelQueue_);
        pGen1->increaseMergeTimes();
        curGeneration_ = 0;
    }
}

