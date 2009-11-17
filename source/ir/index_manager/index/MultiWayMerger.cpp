#include <ir/index_manager/index/MultiWayMerger.h>

using namespace izenelib::ir::indexmanager;

MultiWayMerger::MultiWayMerger(Indexer* pIndexer)
        :IndexMerger(pIndexer)
        ,curGeneration_(0)
{
}

MultiWayMerger::~MultiWayMerger(void)
{
    endMerge();
}
void MultiWayMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    MultiWayMerger::Generation* pGen = NULL;
    map<int,MultiWayMerger::Generation*>::iterator iter = generationMap_.find(curGeneration_);
    if (iter != generationMap_.end())
    {
        pGen = iter->second;
        pGen->add(pEntry);
        if ((int)pGen->pMergeBarrel_->size() >= 2) ///collision,trigger a merge event
        {
            triggerMerge(pGen,curGeneration_);
        }
    }
    else
    {
        pGen = new MultiWayMerger::Generation(curGeneration_,2*MAX_TRIGGERS);
        pGen->add(pEntry);
        generationMap_.insert(make_pair(curGeneration_,pGen));
    }
}
void MultiWayMerger::endMerge()
{
    map<int,MultiWayMerger::Generation*>::iterator iter = generationMap_.begin();
    while (iter != generationMap_.end())
    {
        delete iter->second;
        iter++;
    }
    generationMap_.clear();
}

void MultiWayMerger::triggerMerge(MultiWayMerger::Generation* pGen,int nGen)
{
    MultiWayMerger::Generation* pGen1 = pGen;
    int nCurGen = nGen;
    for (int i = nGen + 1;i <= (nGen + MAX_TRIGGERS);i++)
    {
        map<int,MultiWayMerger::Generation*>::iterator iter2 = generationMap_.find(i);
        if (iter2 != generationMap_.end())
        {
            MultiWayMerger::Generation* pGen2 = iter2->second;
            if ((int)pGen2->pMergeBarrel_->size() + 1 >= 2)	///will trigger a merge event in upper generation
            {
                ///copy elements to upper generation
                while (pGen1->pMergeBarrel_->size() >0)
                {
                    pGen2->pMergeBarrel_->put(pGen1->pMergeBarrel_->pop());
                }
                pGen1 = pGen2;
                nCurGen = i;
            }
            else break;
        }
        else break;
    }

    if (pGen1->pMergeBarrel_->size() > 0)
    {
        curGeneration_ = nCurGen + 1;
        mergeBarrel(pGen1->pMergeBarrel_);
        pGen1->increaseMergeTimes();
        curGeneration_ = 0;
    }
}

