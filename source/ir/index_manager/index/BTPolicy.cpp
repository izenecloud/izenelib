#include <ir/index_manager/index/BTPolicy.h>

#include <algorithm>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

#define MAXLEVEL 30
#define COLLISION_FACTOR_FOR_LEVEL_1 3

BTPolicy::BTPolicy()
{
    for(int i = 0; i < MAXLEVEL; i++)
        nCMap_[i] = COLLISION_FACTOR_FOR_LEVEL_1;
}

BTPolicy::~BTPolicy()
{
    endMerge();
}

int BTPolicy::getC(int nLevel)
{
    assert(nLevel < MAXLEVEL);
    return nCMap_[nLevel];
}

void BTPolicy::addBarrel(MergeBarrelEntry* pEntry)
{
    int nLevelSize = pEntry->numDocs();
    int nLevel = getLevel(nLevelSize);
    int nC = getC(nLevel);
    BTLayer* pLevel = NULL;
    std::map<int,BTLayer*>::iterator iter = nodesMap_.find(nLevel);
    if (iter != nodesMap_.end())
    {
        pLevel = iter->second;
        pLevel->nLevelSize_ += nLevelSize;
        pLevel->add(pEntry);
        if ((int)pLevel->pBarrelQueue_->size() >= nC) ///collision,trigger a merge event
            triggerMerge(pLevel);
    }
    else
    {
        pLevel = new BTLayer(nLevel,nLevelSize,MAX_LAYER_SIZE);
        pLevel->add(pEntry);
        nodesMap_.insert(make_pair(nLevel,pLevel));
    }
}

void BTPolicy::endMerge()
{
    std::map<int,BTLayer*>::iterator iter = nodesMap_.begin();
    while (iter != nodesMap_.end())
    {
        delete iter->second;
        iter++;
    }
    nodesMap_.clear();
}

void BTPolicy::triggerMerge(BTLayer* pLevel)
{
    LOG(INFO) << "=> BTPolicy::triggerMerge(), pLevel: " << pLevel->nLevel_;

    BTLayer* pLevel1 = pLevel;
    std::map<int,BTLayer*>::iterator iter2;

    // if it would trigger another merge event in upper level,
    // combine them directly
    int newLevel = getLevel(pLevel1->nLevelSize_);
    while(newLevel > pLevel1->nLevel_
            && (iter2 = nodesMap_.find(newLevel)) != nodesMap_.end())
    {
        BTLayer* pLevel2 = iter2->second;
        if ((int)pLevel2->pBarrelQueue_->size() + 1 < getC(newLevel))
            break;

        pLevel1 = combineLayer(pLevel1, pLevel2);
        newLevel = getLevel(pLevel1->nLevelSize_);
    }

    // iterate barrels in other layer, merge them if within the base doc id range
    pair<docid_t, docid_t> range1 = pLevel1->getBaseDocIDRange();
    for(iter2 = nodesMap_.begin(); iter2 != nodesMap_.end(); ++iter2)
    {
        // ignore current level
        if(iter2->second == pLevel1)
            continue;

        // check the base doc id of other layer
        BTLayer* pLevel2 = iter2->second;
        const size_t count = (int)pLevel2->pBarrelQueue_->size();
        bool isCombineLayer = false;
        for (size_t i=0; i<count; ++i)
        {
            docid_t baseDoc = pLevel2->pBarrelQueue_->getAt(i)->baseDocID();
            // equal is used in case of some barrel is empty
            if((baseDoc >= range1.first) && (baseDoc <= range1.second))
            {
                isCombineLayer = true;
                break;
            }
        }
        if(isCombineLayer)
        {
            pLevel1 = combineLayer(pLevel1, pLevel2);
            range1 = pLevel1->getBaseDocIDRange();
        }
    }

    // merge and clear the level
    if (pLevel1->pBarrelQueue_->size() > 0)
    {
        pLevel1->nLevelSize_ = 0;
        pIndexMerger_->mergeBarrel(pLevel1->pBarrelQueue_);
        pLevel1->increaseMergeTimes();
    }

    LOG(INFO) << "<= BTPolicy::triggerMerge()";
}

int BTPolicy::getLevel(int64_t nLevelSize)
{
    if (nLevelSize <= 0)
        return 0;

    int level = 0;
    int maxSize = 1;
    for(; level < MAXLEVEL; ++level)
    {
        maxSize *= 3;
        if(nLevelSize < maxSize)
            break;
    }

    return level;
}

BTPolicy::BTLayer* BTPolicy::combineLayer(BTLayer* pLayer1, BTLayer* pLayer2) const
{
    assert(pLayer1 != pLayer2
        && pLayer1->nLevel_ != pLayer2->nLevel_);

    BTLayer* pLower = pLayer1;
    BTLayer* pHigher = pLayer2;
    if(pLower->nLevel_ > pHigher->nLevel_)
        swap(pLower, pHigher);

    DVLOG(2) << "BTPolicy::combineLayer() => layer " << pLower->nLevel_
             << " combined to layer " << pHigher->nLevel_;

    // copy elements to upper level
    while (pLower->pBarrelQueue_->size() >0)
        pHigher->pBarrelQueue_->put(pLower->pBarrelQueue_->pop());

    pHigher->nLevelSize_ += pLower->nLevelSize_;
    pLower->nLevelSize_ = 0;

    return pHigher;
}

}
NS_IZENELIB_IR_END

