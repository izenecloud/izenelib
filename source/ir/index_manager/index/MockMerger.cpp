#include <ir/index_manager/index/MockMerger.h>

#include <algorithm>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

#define NUMDOC_PER_BARREL 10000
#define MAXLEVEL 30
#define LAZY_MERGE_START_LEVEL 10
#define COLLISION_FACTOR_FOR_LEVEL_1 3

MockMerger::MockMerger(BarrelsInfo* pBarrelsInfo,Directory* pDirectory)
        : pBarrelsInfo_(pBarrelsInfo)
        , pDirectory_(pDirectory)
        , nCurLevelSize_(1)
{
    pMergeBarrels_ = new std::vector<MergeBarrelEntry*>;
}

MockMerger::~MockMerger()
{
    if (pMergeBarrels_)
    {
        pMergeBarrels_->clear();
        delete pMergeBarrels_;
        pMergeBarrels_ = NULL;
    }

    endMerge();
}

int MockMerger::getC(int nLevel)
{
    return nCMap_[nLevel];
}

void MockMerger::addBarrel(MergeBarrelEntry* pEntry)
{
    cout<<"add barrel "<<pEntry->pBarrelInfo_->barrelName<<" num doc "<<pEntry->pBarrelInfo_->nNumDocs<<endl;
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
        pLevel = new BTLayer(nLevel,nCurLevelSize_,nC*2);
        pLevel->add(pEntry);
        nodesMap_.insert(make_pair(nLevel,pLevel));
    }
}

void MockMerger::endMerge()
{
    std::map<int,BTLayer*>::iterator iter = nodesMap_.begin();
    while (iter != nodesMap_.end())
    {
        delete iter->second;
        iter++;
    }
    nodesMap_.clear();
}

void MockMerger::triggerMerge(BTLayer* pLevel,int nLevel)
{
    BTLayer* pLevel1 = pLevel;
    int nL = getLevel(pLevel->nLevelSize_);
    int nTriggers = 0;
    for ( int i = nLevel + 1;i <= nL;i++)
    {
        std::map<int,BTLayer*>::iterator iter2 = nodesMap_.find(i);
        if (iter2 != nodesMap_.end())
        {
            BTLayer* pLevel2 = iter2->second;
            int nC = getC(i);
            if ((int)pLevel2->pMergeBarrel_->size() + 1 >= nC) ///will trigger another merge event in upper level
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
    for(int i = nL + 1; i < MAXLEVEL; i++)
    {
        std::map<int,BTLayer*>::iterator iter2 = nodesMap_.find(i);
        if(iter2 != nodesMap_.end())
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
                nL = getLevel(pLevel2->nLevelSize_);
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
int MockMerger::getLevel(int64_t nLevelSize)
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
//        num_doc_per_barrel_ = nLevelSize * 3/2;
        int nC = COLLISION_FACTOR_FOR_LEVEL_1;
        nCMap_[1] = COLLISION_FACTOR_FOR_LEVEL_1;
        for(int i = 2; i < MAXLEVEL; i++)
        {
//            if(i >= 2)
//                nC = 10;
            nCMap_[i] = nC;
        }
//        return 1;
    }
    return level;
/*
    int64_t numDocs = num_doc_per_barrel_;
    for(int i = 1; i < MAXLEVEL; ++i)
    {
        numDocs *= nCMap_[i];
        if(nLevelSize <= numDocs)
            return i;
    }
    return MAXLEVEL;
*/	
}

void MockMerger::merge()
{
    triggerMerge_ = false;

    if (pBarrelsInfo_->getBarrelCount() <= 1) return;

    BarrelInfo* pBaInfo;
    MergeBarrel mb(pBarrelsInfo_->getBarrelCount());
    ///put all index barrel into mb
    pBarrelsInfo_->startIterator();
    while (pBarrelsInfo_->hasNext())
    {
        pBaInfo = pBarrelsInfo_->next();
        mb.put(new MergeBarrelEntry(NULL,pBaInfo));
    }

    while (mb.size() > 0)
    {
        addBarrel(mb.pop());
    }

    endMerge();

    if(!triggerMerge_) return;

    pBarrelsInfo_->sort(pDirectory_);

    pBarrelsInfo_->startIterator();
    docid_t maxDoc = 0;
    while (pBarrelsInfo_->hasNext())
    {
        pBaInfo = pBarrelsInfo_->next();
        if(pBaInfo->getMaxDocID() > maxDoc)
            maxDoc = pBaInfo->getMaxDocID();
    }

    if(maxDoc < pBarrelsInfo_->maxDocId())
        pBarrelsInfo_->resetMaxDocId(maxDoc);
    pBarrelsInfo_->write(pDirectory_);
}

void MockMerger::addToMerge(BarrelInfo* pBarrelInfo)
{
    if(pBarrelInfo->isRemoved()) return;

    triggerMerge_ = false;

    MergeBarrelEntry* pEntry = new MergeBarrelEntry(NULL,pBarrelInfo);

    pMergeBarrels_->push_back(pEntry);

    addBarrel(pEntry);

    if(!triggerMerge_) return;

    pBarrelsInfo_->sort(pDirectory_);
}

void MockMerger::mergeBarrel(MergeBarrel* pBarrel)
{
    triggerMerge_ = true;
    cout<< "=> IndexMerger::mergeBarrel(), barrel name: " << pBarrel->getIdentifier() << endl;

    std::string newBarrelName = pBarrel->getIdentifier();
    BarrelInfo* pNewBarrelInfo = new BarrelInfo(newBarrelName,0);
    MergeBarrelEntry* pEntry = NULL;
    count_t nNumDocs = 0;
    int nEntryCount = (int)pBarrel->size();
    ///update min doc id of index barrels,let doc id continuous
    std::map<collectionid_t,docid_t> newBaseDocIDMap;
    docid_t maxDocOfNewBarrel = 0;
    for (int nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);

        nNumDocs += pEntry->pBarrelInfo_->getDocCount();

        if(pEntry->pBarrelInfo_->getMaxDocID() > maxDocOfNewBarrel)
            maxDocOfNewBarrel = pEntry->pBarrelInfo_->getMaxDocID();

        for (map<collectionid_t,docid_t>::iterator iter = pEntry->pBarrelInfo_->baseDocIDMap.begin();
                iter != pEntry->pBarrelInfo_->baseDocIDMap.end(); ++iter)
        {
            if (newBaseDocIDMap.find(iter->first) == newBaseDocIDMap.end())
            {
                newBaseDocIDMap.insert(make_pair(iter->first,iter->second));
            }
            else
            {
                if (newBaseDocIDMap[iter->first] > iter->second)
                {
                    newBaseDocIDMap[iter->first] = iter->second;
                }
            }
        }
    }


    for (int nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);
        pBarrelsInfo_->removeBarrel(pDirectory_,pEntry->pBarrelInfo_->getName());///delete merged barrels
    }
	
    if (pMergeBarrels_)
    {
        std::vector<MergeBarrelEntry*>::iterator iter = pMergeBarrels_->begin();
        bool bRemoved = false;
        uint32_t nRemoved = 0;
        while (iter != pMergeBarrels_->end())
        {
            bRemoved = false;
            for (size_t nEntry = 0;nEntry < pBarrel->size();nEntry++)
            {
                if ((*iter) == pBarrel->getAt(nEntry))
                {
                    iter = pMergeBarrels_->erase(iter);
                    bRemoved = true;
                    nRemoved++;
                }
            }
            if (nRemoved == pBarrel->size())
                break;
            if (!bRemoved)
                iter++;
        }
			
    }
    pBarrel->clear();
	
	
    pNewBarrelInfo->setDocCount(nNumDocs);
    pNewBarrelInfo->setBaseDocID(newBaseDocIDMap);
    pNewBarrelInfo->updateMaxDoc(maxDocOfNewBarrel);
    
    pNewBarrelInfo->setSearchable(true);
    pBarrelsInfo_->addBarrel(pNewBarrelInfo,false);

    cout<< "<= IndexMerger::mergeBarrel(), barrel name: " << pBarrel->getIdentifier() << ", doc count: " << pNewBarrelInfo->getDocCount() <<endl;;

    MergeBarrelEntry* pNewEntry = new MergeBarrelEntry(NULL,pNewBarrelInfo);
    addBarrel(pNewEntry);

}

}
NS_IZENELIB_IR_END


