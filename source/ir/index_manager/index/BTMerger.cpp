#include <ir/index_manager/index/BTMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

#define NUMDOC_PER_BARREL 10000
#define MAXLEVEL 30
#define COLLISION_FACTOR_FOR_LEVEL_1 3

BTMerger::BTMerger(Indexer* pIndexer)
        :IndexMerger(pIndexer)
        ,nCurLevelSize_(1)
{
}

BTMerger::~BTMerger()
{
    endMerge();
}

int BTMerger::getC(int nLevel)
{
    return nCMap_[nLevel];
}

void BTMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    nCurLevelSize_ = pEntry->numDocs();
    int nLevel = getLevel(nCurLevelSize_);
    int nC = getC(nLevel);
    BTLayer* pLevel = NULL;
    std::map<int,BTLayer*>::iterator iter = nodesMap_.find(nLevel);
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
        pLevel = new BTLayer(nLevel,nCurLevelSize_,MAX_LAYER_SIZE);
        pLevel->add(pEntry);
        nodesMap_.insert(make_pair(nLevel,pLevel));
    }
}

void BTMerger::endMerge()
{
    std::map<int,BTLayer*>::iterator iter = nodesMap_.begin();
    while (iter != nodesMap_.end())
    {
        delete iter->second;
        iter++;
    }
    nodesMap_.clear();
}

void BTMerger::triggerMerge(BTLayer* pLevel,int nLevel)
{
    BTLayer* pLevel1 = pLevel;
    int nL = getLevel(pLevel->nLevelSize_);
    int nTriggers = 0;
    int i ;
    for ( i = nLevel + 1;(i <= nL);i++)
    {
        std::map<int,BTLayer*>::iterator iter2 = nodesMap_.find(i);
        if (iter2 != nodesMap_.end())
        {
            BTLayer* pLevel2 = iter2->second;
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

    ///whether is it possible to merge barrels from upper layer to avoid of error
    for(int i = 1; i < MAXLEVEL; i++)
    {
        std::map<int,BTLayer*>::iterator iter2 = nodesMap_.find(i);
        if(iter2 != nodesMap_.end() && iter2->second != pLevel1) // ignore current level
        {
            ///find the base doc id range for merged barrels
            docid_t startDoc = -1;
            docid_t endDoc = 0;
            int nEntryCount = (int)pLevel1->pMergeBarrel_->size();
            for (int nEntry = 0;nEntry < nEntryCount;nEntry++)
            {
                docid_t baseDoc = pLevel1->pMergeBarrel_->getAt(nEntry)->baseDocID();
                startDoc = baseDoc < startDoc ? baseDoc : startDoc; 	
                endDoc = baseDoc > endDoc ? baseDoc : endDoc;
            }

            ///juge the base doc id range of upper layer
            BTLayer* pLevel2 = iter2->second;
            nEntryCount = (int)pLevel2->pMergeBarrel_->size();
            bool needToMergeUpperLayer = false;
            for (int nEntry = 0;nEntry < nEntryCount;nEntry++)
            {
                docid_t baseDoc = pLevel2->pMergeBarrel_->getAt(nEntry)->baseDocID();
                if((baseDoc > startDoc) && (baseDoc < endDoc))
                {
                    needToMergeUpperLayer = true;
                    break;
                }
            }
            if(needToMergeUpperLayer)
            {
                ///copy elements to upper level
                while (pLevel1->pMergeBarrel_->size() >0)
                {
                    pLevel2->pMergeBarrel_->put(pLevel1->pMergeBarrel_->pop());
                }
                pLevel2->nLevelSize_ += pLevel1->nLevelSize_;
                pLevel1->nLevelSize_ = 0;
                pLevel1 = pLevel2;
                nTriggers++;
            }
        }
    }

    ///merge
    if (pLevel1->pMergeBarrel_->size() > 0)
    {
        nCurLevelSize_ = pLevel1->nLevelSize_;
        pLevel1->nLevelSize_ = 0;
        mergeBarrel(pLevel1->pMergeBarrel_);
        pLevel1->increaseMergeTimes();
        nCurLevelSize_ = 1;
    }
}
int BTMerger::getLevel(int64_t nLevelSize)
{
    if (nLevelSize <= 0)
        return 0;

    int level = 0;
    while( ( level < MAXLEVEL) && (nLevelSize >= (int)pow(3.0,(double)(level + 1)) ) )
    {
        level++;
    }

    if(nCMap_.empty())
    {
        int nC = COLLISION_FACTOR_FOR_LEVEL_1;
        for(int i = 1; i < MAXLEVEL; i++)
        {
            nCMap_[i] = nC;
        }
    }
    return level;
}

}
NS_IZENELIB_IR_END

