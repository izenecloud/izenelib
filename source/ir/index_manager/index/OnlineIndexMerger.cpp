#include <ir/index_manager/index/OnlineIndexMerger.h>
#include <ir/index_manager/utility/ParamParser.h>


using namespace izenelib::ir::indexmanager;

OnlineIndexMerger::OnlineIndexMerger(Directory* pSrDirectory)
        :IndexMerger(pSrDirectory)
        ,nM(3)
        ,nS(1)
        ,nC(3)
        ,nCurLevelSize(1)
{
}

OnlineIndexMerger::OnlineIndexMerger(Directory* pSrDirectory,char* buffer,size_t bufsize)
        :IndexMerger(pSrDirectory,buffer,bufsize)
        ,nM(3)
        ,nS(1)
        ,nC(3)
        ,nCurLevelSize(1)
{
}

OnlineIndexMerger::~OnlineIndexMerger()
{
    endMerge();
}

void OnlineIndexMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    if (nS > 0)
    {
        nCurLevelSize = (int32_t)( (double)pEntry->numDocs()/(double)nS + 0.5 );
    }
    int32_t nLevel = getLevel(nCurLevelSize,nM);
    DBTLayer* pLevel = NULL;
    map<int32_t,DBTLayer*>::iterator iter = nodesMap.find(nLevel);
    if (iter != nodesMap.end())
    {
        pLevel = iter->second;
        pLevel->nLevelSize += nCurLevelSize;
        pLevel->add(pEntry);
        if ((int32_t)pLevel->pMergeBarrel->size() >= nC) ///collision,trigger a merge event
        {
            triggerMerge(pLevel,nLevel);
        }
    }
    else
    {
        pLevel = new DBTLayer(nLevel,nCurLevelSize,nC*MAX_TRIGGERS);
        pLevel->add(pEntry);
        nodesMap.insert(make_pair(nLevel,pLevel));
    }
}

void OnlineIndexMerger::endMerge()
{
    map<int32_t,DBTLayer*>::iterator iter = nodesMap.begin();
    while (iter != nodesMap.end())
    {
        delete iter->second;
        iter++;
    }
    nodesMap.clear();
}
void OnlineIndexMerger::setParam(const char* pszParam)
{
    ParamParser parser;
    if (parser.parse(pszParam,";"))
    {
        parser.getParam("m",nM);
        parser.getParam("s",nS);
        parser.getParam("c",nC);
    }
}
void OnlineIndexMerger::triggerMerge(DBTLayer* pLevel,int32_t nLevel)
{
    DBTLayer* pLevel1 = pLevel;
    int32_t nL = this->getLevel(pLevel->nLevelSize,nM);
    int32_t nTriggers = 0;

    int32_t i ;
    for ( i = nLevel + 1;(i <= nL) && (nTriggers <= MAX_TRIGGERS);i++)
    {
        map<int32_t,DBTLayer*>::iterator iter2 = nodesMap.find(i);
        if (iter2 != nodesMap.end())
        {
            DBTLayer* pLevel2 = iter2->second;
            if ((int32_t)pLevel2->pMergeBarrel->size() + 1 >= nC)	///will trigger another merge event in upper level
            {
                ///copy elements to upper level
                while (pLevel1->pMergeBarrel->size() >0)
                {
                    pLevel2->pMergeBarrel->put(pLevel1->pMergeBarrel->pop());
                }
                pLevel2->nLevelSize += pLevel1->nLevelSize;
                pLevel1->nLevelSize = 0;
                nL = this->getLevel(pLevel2->nLevelSize,nM);
                pLevel1 = pLevel2;
                nTriggers++;
            }
            else break;
        }
        else break;
    }

    if (pLevel1->pMergeBarrel->size() > 0)
    {
        nCurLevelSize = pLevel1->nLevelSize;
        pLevel1->nLevelSize = 0;
        mergeBarrel(pLevel1->pMergeBarrel);
        pLevel1->increaseMergeTimes();
        nCurLevelSize = 1;
    }
}
int32_t OnlineIndexMerger::getLevel(int64_t nLevelSize,int32_t m)
{
    if (nLevelSize <= 0)
        return 0;
    int32_t level = 0;
    ///determine its level
    while ( ( level < 31) && (nLevelSize >= (int32_t)pow((double)m,(double)(level + 1)) ) )
    {
        level++;
    }
    return level;
}

