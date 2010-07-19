#include <ir/index_manager/index/DBTMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

#define NUMDOC_PER_BARREL 3000
#define MAXLEVEL 30
#define COLLISION_FACTOR_FOR_LEVEL_1 3

DBTMerger::DBTMerger(Indexer* pIndexer)
        :IndexMerger(pIndexer)
        ,nCurLevelSize_(1)
{
    int nC = COLLISION_FACTOR_FOR_LEVEL_1;
    nCMap_[1] = COLLISION_FACTOR_FOR_LEVEL_1;
    for(int i = 2; i < MAXLEVEL; i++)
    {
        if(i >= 2)
            nC = 10;
        nCMap_[i] = nC;
    }
}

DBTMerger::~DBTMerger()
{
    endMerge();
}

int DBTMerger::getC(int nLevel)
{
    return nCMap_[nLevel];
}

void DBTMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    nCurLevelSize_ = pEntry->numDocs();
    int nLevel = getLevel(nCurLevelSize_);
    int nC = getC(nLevel);
    DBTLayer* pLevel = NULL;
    std::map<int,DBTLayer*>::iterator iter = nodesMap_.find(nLevel);
    if (iter != nodesMap_.end())
    {
        pLevel = iter->second;
        pLevel->nLevelSize_ += nCurLevelSize_;
        pLevel->add(pEntry);
        if ((int)pLevel->pMergeBarrel_->size() >= nC) ///collision,trigger a merge event
        {
            triggerMerge(pLevel,nLevel);
        }
    }
    else
    {
        pLevel = new DBTLayer(nLevel,nCurLevelSize_,nC*MAX_TRIGGERS);
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
            int nC = getC(i);
            if ((int)pLevel2->pMergeBarrel_->size() + 1 >= nC)	///will trigger another merge event in upper level
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
    int64_t numDocs = NUMDOC_PER_BARREL;
    for(int i = 1; i < MAXLEVEL; ++i)
    {
        numDocs *= nCMap_[i];
        if(nLevelSize <= numDocs)
            return i;
    }
    return MAXLEVEL;
}

}
NS_IZENELIB_IR_END

