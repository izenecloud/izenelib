#include <ir/index_manager/index/OptimizeMerger.h>

using namespace izenelib::ir::indexmanager;

OptimizeMerger::OptimizeMerger(Directory* pSrDirectory)
        :IndexMerger(pSrDirectory)
        ,nC(5)
        ,nScale(0)
        ,nCurLevelSize(1)
{
}
OptimizeMerger::OptimizeMerger(Directory* pSrDirectory,char* buffer,size_t bufsize)
        :IndexMerger(pSrDirectory,buffer,bufsize)
        ,nC(5)
        ,nScale(0)
        ,nCurLevelSize(1)
{
}

OptimizeMerger::~OptimizeMerger(void)
{
    map<int32_t,OptimizeMergeTreeLevel*>::iterator iter = levelsMap.begin();
    while (iter != levelsMap.end())
    {
        delete iter->second;
        iter++;
    }
    levelsMap.clear();
}
void OptimizeMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    if (nScale > 0)
    {
        nCurLevelSize = (int32_t)( (double)pEntry->numDocs()/(double)nScale + 0.5 );
    }
    int32_t nLevel = getLevel(nCurLevelSize);

    OptimizeMergeTreeLevel* pLevel = NULL;
    map<int32_t,OptimizeMergeTreeLevel*>::iterator iter = levelsMap.find(nLevel);
    if (iter != levelsMap.end())
    {
        pLevel = iter->second;
        pLevel->nLevelSize += nCurLevelSize;
        pLevel->add(pEntry);
        if ((int32_t)pLevel->pMergeBarrel->size() >= nC) ///collision,trigger a merge event
        {
            nCurLevelSize = pLevel->nLevelSize;
            pLevel->nLevelSize = 0;
            mergeBarrel(pLevel->pMergeBarrel);
            pLevel->increaseMergeTimes();
            nCurLevelSize = 1;
        }
    }
    else
    {
        pLevel = new OptimizeMergeTreeLevel(nLevel,nCurLevelSize,nC);
        pLevel->add(pEntry);
        levelsMap.insert(make_pair(nLevel,pLevel));
    }
}
void OptimizeMerger::endMerge()
{
    OptimizeMergeTreeLevel* pCurLevel = NULL;
    OptimizeMergeTreeLevel* pLevel = NULL;
    if (levelsMap.size() == 1)
    {
        pLevel =	levelsMap.begin()->second;
        if (pLevel->pMergeBarrel->size() > 1)
        {
            nCurLevelSize = pLevel->nLevelSize;
            pLevel->nLevelSize = 0;
            mergeBarrel(pLevel->pMergeBarrel);
            pLevel->increaseMergeTimes();
            nCurLevelSize = 1;
        }
    }
    else
    {
        bool bMerged;
        do
        {
            bMerged = false;
            pCurLevel = NULL;
            map<int32_t,OptimizeMergeTreeLevel*>::iterator iter = levelsMap.begin();
            while (iter != levelsMap.end())
            {
                pLevel = iter->second;
                if (pCurLevel)
                {
                    ///copy elements to upper level
                    while (pCurLevel->pMergeBarrel->size() > 0)
                    {
                        pLevel->pMergeBarrel->put(pCurLevel->pMergeBarrel->pop());
                        pLevel->nLevelSize = pCurLevel->nLevelSize;
                        pCurLevel->nLevelSize = 0;
                        if ((int32_t)pLevel->pMergeBarrel->size() >= nC)
                        {
                            nCurLevelSize = pLevel->nLevelSize;
                            pLevel->nLevelSize = 0;
                            mergeBarrel(pLevel->pMergeBarrel);
                            pLevel->increaseMergeTimes();
                            nCurLevelSize = 1;
                            bMerged = true;
                        }
                    }
                }
                iter++;

                pCurLevel = pLevel;
            }
        }
        while (bMerged);

        if (pLevel->pMergeBarrel->size() > 0)
        {
            nCurLevelSize = pLevel->nLevelSize;
            pLevel->nLevelSize = 0;
            mergeBarrel(pLevel->pMergeBarrel);
            pLevel->increaseMergeTimes();
            nCurLevelSize = 1;
        }
    }

    map<int32_t,OptimizeMergeTreeLevel*>::iterator iter = levelsMap.begin();
    while (iter != levelsMap.end())
    {
        delete iter->second;
        iter++;
    }
    levelsMap.clear();
}

int32_t OptimizeMerger::getLevel(int64_t nLevelSize)
{
    if (nLevelSize <= 0)
        return 0;
    int32_t level = 0;
    ///determine its level
    while ( ( level < 31) && (nLevelSize >= (int32_t)pow((double)nC,(double)(level + 1)) ) )
    {
        level++;
    }
    return level;
}

