#include <ir/index_manager/index/DefaultMerger.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

#define MAXLEVEL 2
#define MERGE_FACTOR 10

DefaultMerger::DefaultMerger(Indexer* pIndexer)
        :IndexMerger(pIndexer)
        ,nCurLevelSize_(1)
{
}

DefaultMerger::~DefaultMerger()
{
}

void DefaultMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    if(pEntry->pBarrelInfo_->isSearchable())
        return;
    nCurLevelSize_ = pEntry->numDocs();
    int nLevel = getLevel(nCurLevelSize_);
    int nC = nCMap_[nLevel];
    Layer* pLevel = NULL;
    std::map<int,Layer*>::iterator iter = nodesMap_.find(nLevel);
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
        pLevel = new Layer(nLevel,nCurLevelSize_,nC*5);
        pLevel->add(pEntry);
        nodesMap_.insert(make_pair(nLevel,pLevel));
    }
}

void DefaultMerger::endMerge()
{
    Layer* pLevel3 = new Layer(3,1,100);
    Layer* pLevel2 = nodesMap_[2];
    while (pLevel2->pMergeBarrel_->size() >0)
    {
        pLevel3->pMergeBarrel_->put(pLevel2->pMergeBarrel_->pop());
    }
    Layer* pLevel1 = nodesMap_[1];
    while (pLevel1->pMergeBarrel_->size() >0)
    {
        pLevel3->pMergeBarrel_->put(pLevel1->pMergeBarrel_->pop());
    }
    mergeBarrel(pLevel3->pMergeBarrel_);

    std::map<int,Layer*>::iterator iter = nodesMap_.begin();
    for(; iter != nodesMap_.end(); ++iter)
    {
        delete iter->second;
    }
    nodesMap_.clear();
    delete pLevel3;
}

void DefaultMerger::triggerMerge(Layer* pLevel,int nLevel)
{
    Layer* pLevel1 = pLevel;
    int nL = getLevel(pLevel->nLevelSize_);
    int nTriggers = 0;
    int i;
    for ( i = nLevel + 1; i <= nL; i++)
    {
        std::map<int,Layer*>::iterator iter2 = nodesMap_.find(i);
        if (iter2 != nodesMap_.end())
        {
            Layer* pLevel2 = iter2->second;
            int nC = nCMap_[i];
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
int DefaultMerger::getLevel(int64_t nLevelSize)
{
    if (nLevelSize <= 0)
        return 0;

    if(nCMap_.empty())
    {
        num_doc_per_barrel_ = (nLevelSize/1000)*1000;
        nCMap_[1] = MERGE_FACTOR;
        nCMap_[2] = 10000;///unlimited

        return 1;
    }

    int64_t numDocs = num_doc_per_barrel_;
    for(int i = 1; i <= MAXLEVEL; ++i)
    {
        numDocs *= nCMap_[i];
        if(nLevelSize <= numDocs)
            return i;
    }
    return MAXLEVEL;
}

}
NS_IZENELIB_IR_END

