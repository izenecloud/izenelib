#include <ir/index_manager/index/DBTMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

DBTMerger::DBTMerger(Indexer* pIndexer)
        :IndexMerger(pIndexer)
        ,nC_(3)
        ,nCurLevelSize_(1)
{
}

DBTMerger::~DBTMerger()
{
    endMerge();
}

void DBTMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    nCurLevelSize_ = pEntry->numDocs();
    int nLevel = getLevel(nCurLevelSize_);
    DBTLayer* pLevel = NULL;
    std::map<int,DBTLayer*>::iterator iter = nodesMap_.find(nLevel);
    if (iter != nodesMap_.end())
    {
        pLevel = iter->second;
        pLevel->nLevelSize_ += nCurLevelSize_;
        pLevel->add(pEntry);
        if ((int)pLevel->pMergeBarrel_->size() >= nC_) ///collision,trigger a merge event
        {
            triggerMerge(pLevel,nLevel);
        }
    }
    else
    {
        pLevel = new DBTLayer(nLevel,nCurLevelSize_,nC_*MAX_TRIGGERS);
        pLevel->add(pEntry);
        nodesMap_.insert(make_pair(nLevel,pLevel));
    }
}

void DBTMerger::endMerge()
{
    std::map<int,DBTLayer*>::iterator iter = nodesMap_.begin();
    while (iter != nodesMap_.end())
    {
        delete iter->second;
        iter++;
    }
    nodesMap_.clear();
}

void DBTMerger::triggerMerge(DBTLayer* pLevel,int nLevel)
{
    DBTLayer* pLevel1 = pLevel;
    int nL = getLevel(pLevel->nLevelSize_);
    int nTriggers = 0;

    int i ;
    for ( i = nLevel + 1;(i <= nL)/* && (nTriggers <= MAX_TRIGGERS)*/;i++)
    {
        std::map<int,DBTLayer*>::iterator iter2 = nodesMap_.find(i);
        if (iter2 != nodesMap_.end())
        {
            DBTLayer* pLevel2 = iter2->second;
            if ((int)pLevel2->pMergeBarrel_->size() + 1 >= nC_)	///will trigger another merge event in upper level
            {
                ///copy elements to upper level
                while (pLevel1->pMergeBarrel_->size() >0)
                {
                    pLevel2->pMergeBarrel_->put(pLevel1->pMergeBarrel_->pop());
                }
                pLevel2->nLevelSize_ += pLevel1->nLevelSize_;
                pLevel1->nLevelSize_ = 0;
                nL = getLevel(pLevel2->nLevelSize_);
                pLevel1 = pLevel2;
                nTriggers++;
            }
            else break;
        }
        else break;
    }

    if (pLevel1->pMergeBarrel_->size() > 0)
    {
        nCurLevelSize_ = pLevel1->nLevelSize_;
        pLevel1->nLevelSize_ = 0;
        mergeBarrel(pLevel1->pMergeBarrel_);
        pLevel1->increaseMergeTimes();
        nCurLevelSize_ = 1;
    }
}
int DBTMerger::getLevel(int64_t nLevelSize)
{
    if (nLevelSize <= 0)
        return 0;
/*
    int level = 0;
    ///determine its level
    while ( ( level < 31) && (nLevelSize >= (int)pow(3,(double)(level + 1)) ) )
    {
        level++;
    }
    return level;
*/
    return nLevelSize/10000+1;
}

}
NS_IZENELIB_IR_END

