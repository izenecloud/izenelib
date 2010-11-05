#include <boost/date_time/posix_time/posix_time.hpp>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/FieldMerger.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/utility/StringUtils.h>

#include <util/izene_log.h>
#include <util/ThreadModel.h>

#include <boost/thread.hpp>

#include <sstream>
#include <memory>
#include <algorithm>


using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///MergeBarrelEntry
MergeBarrelEntry::MergeBarrelEntry(Directory* pDirectory,BarrelInfo* pBarrelInfo)
        :pDirectory_(pDirectory)
        ,pBarrelInfo_(pBarrelInfo)
        ,pCollectionsInfo_(NULL)
        ,currColID_(-1)
{
}
MergeBarrelEntry::~MergeBarrelEntry()
{
    if (pCollectionsInfo_)
    {
        delete pCollectionsInfo_;
        pCollectionsInfo_ = NULL;
    }

    pBarrelInfo_ = NULL;
}

void MergeBarrelEntry::load()
{
    IndexBarrelWriter* pWriter = pBarrelInfo_->getWriter();
    if (!pCollectionsInfo_)
    {
        if (pWriter)
        {
            pCollectionsInfo_ = new CollectionsInfo(*(pWriter->getCollectionsInfo()));
        }
        else
            pCollectionsInfo_ = new CollectionsInfo();
    }

    if (!pWriter)
    {
        IndexInput* fdiStream = pDirectory_->openInput(pBarrelInfo_->getName() + ".fdi");

        pCollectionsInfo_->read(fdiStream);///read collection info
        delete fdiStream;
    }
}

IndexMerger::IndexMerger(Indexer* pIndexer)
        :pIndexer_(pIndexer)
        ,pDirectory_(pIndexer->getDirectory())
        ,pMergeBarrels_(NULL)
	,pDocFilter_(NULL)
	,triggerMerge_(false)
	,optimize_(false)
{
}

IndexMerger::~IndexMerger()
{
    pIndexer_ = 0;

    if (pMergeBarrels_)
    {
        pMergeBarrels_->clear();
        delete pMergeBarrels_;
        pMergeBarrels_ = NULL;
    }
    pDocFilter_ = 0;
}

void IndexMerger::merge(BarrelsInfo* pBarrels)
{
    triggerMerge_ = false;

    if (!pBarrels || ((pBarrels->getBarrelCount() <= 1)&&!pDocFilter_))
    {
        //updateBarrels(pBarrels);
        return ;
    }

    pBarrelsInfo_ = pBarrels;

    BarrelInfo* pBaInfo;
    boost::mutex::scoped_lock lock(pBarrels->getMutex());
    MergeBarrel mb(pBarrels->getBarrelCount());
    ///put all index barrel into mb
    pBarrels->startIterator();
    while (pBarrels->hasNext())
    {
        pBaInfo = pBarrels->next();
        if(pBaInfo->getWriter())
            continue;
        mb.put(new MergeBarrelEntry(pDirectory_,pBaInfo));
    }
    lock.unlock();

    while (mb.size() > 0)
    {
        addBarrel(mb.pop());
    }

    endMerge();

    if(!triggerMerge_)
        return;

    updateBarrels(pBarrels); 
    pBarrelsInfo_ = NULL;

    lock.lock();
    pBarrels->startIterator();
    docid_t maxDoc = 0;
    while (pBarrels->hasNext())
    {
        pBaInfo = pBarrels->next();
        if(pBaInfo->getMaxDocID() > maxDoc)
            maxDoc = pBaInfo->getMaxDocID();
    }
    lock.unlock();

    if(maxDoc < pBarrels->maxDocId())
        pBarrels->resetMaxDocId(maxDoc);
    pBarrels->write(pDirectory_);
}

void IndexMerger::addToMerge(BarrelsInfo* pBarrelsInfo,BarrelInfo* pBarrelInfo)
{
    DVLOG(2) << "=> IndexMerger::addToMerge(), barrel name: " << pBarrelInfo->barrelName << " ...";

    if(pBarrelInfo->isRemoved())
    {
        DVLOG(2) << "<= IndexMerger::addToMerge(), barrel already removed";
        return;
    }

    triggerMerge_ = false;
    if (!pMergeBarrels_)
        pMergeBarrels_ = new vector<MergeBarrelEntry*>();

    MergeBarrelEntry* pEntry = NULL;
    pBarrelsInfo_ = pBarrelsInfo;
    pEntry = new MergeBarrelEntry(pDirectory_,pBarrelInfo);
    pMergeBarrels_->push_back(pEntry);

    addBarrel(pEntry);
 
    if(!triggerMerge_)
    {
        DVLOG(2) << "<= IndexMerger::addToMerge(), no merge triggered";
        return;
    }

    updateBarrels(pBarrelsInfo);

    pBarrelsInfo->write(pDirectory_);
    DVLOG(2) << "<= IndexMerger::addToMerge()";
}

void IndexMerger::updateBarrels(BarrelsInfo* pBarrelsInfo)
{
    {
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(pIndexer_->mutex_);
    ///sort barrels
    pBarrelsInfo->sort(pDirectory_);
    }
    pIndexer_->setDirty();
}

void IndexMerger::mergeBarrel(MergeBarrel* pBarrel)
{
    DVLOG(2)<< "=> IndexMerger::mergeBarrel(), barrel name: " << pBarrel->getIdentifier() << " ...";
    triggerMerge_ = true;
    pBarrel->load();
    string newBarrelName = pBarrel->getIdentifier();
    BarrelInfo* pNewBarrelInfo = new BarrelInfo(newBarrelName,0,pIndexer_->getIndexCompressType());

    string name = newBarrelName + ".voc";///the file name of new index barrel
    IndexOutput* pVocStream = pDirectory_->createOutput(name);
    name = newBarrelName + ".dfp";
    IndexOutput* pDStream = pDirectory_->createOutput(name);
    name = newBarrelName + ".pop";
    IndexOutput* pPStream = pDirectory_->createOutput(name);

    OutputDescriptor* pOutputDesc = new OutputDescriptor(pVocStream,pDStream,pPStream,true);
    pOutputDesc->setBarrelName(newBarrelName);
    pOutputDesc->setDirectory(pDirectory_);

    int32_t nEntry;
    MergeBarrelEntry* pEntry = NULL;
    count_t nNumDocs = 0;
    int32_t nEntryCount = (int32_t)pBarrel->size();
    ///update min doc id of index barrels,let doc id continuous
    map<collectionid_t,docid_t> newBaseDocIDMap;
    docid_t maxDocOfNewBarrel = 0;
    bool isNewBarrelUpdateBarrel = true;
    bool hasUpdateBarrel = false;
    for (nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);

        nNumDocs += pEntry->pBarrelInfo_->getDocCount();

        isNewBarrelUpdateBarrel &= pEntry->pBarrelInfo_->isUpdate;
        hasUpdateBarrel |= pEntry->pBarrelInfo_->isUpdate;

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
    bool needSortingMerge = hasUpdateBarrel&&(!isNewBarrelUpdateBarrel);
	
    pNewBarrelInfo->setDocCount(nNumDocs);
    pNewBarrelInfo->setBaseDocID(newBaseDocIDMap);
    pNewBarrelInfo->updateMaxDoc(maxDocOfNewBarrel);
    pNewBarrelInfo->isUpdate = isNewBarrelUpdateBarrel;
    pNewBarrelInfo->setSearchable(false);


    FieldsInfo* pFieldsInfo = NULL;
    CollectionsInfo collectionsInfo;
    CollectionInfo* pColInfo = NULL;
    FieldInfo* pFieldInfo = NULL;
    FieldMerger* pFieldMerger = NULL;
    fieldid_t fieldid = 0;
    bool bFinish = false;

    fileoffset_t vocOff1,vocOff2,dfiOff1,dfiOff2,ptiOff1 = 0,ptiOff2 = 0;
    fileoffset_t voffset = 0;

    ///collect all colIDs in barrells
    vector<collectionid_t> colIDSet;
    CollectionsInfo* pColsInfo = NULL;

    for (nEntry = 0; nEntry < nEntryCount; nEntry++)
    {
        pColsInfo = pBarrel->getAt(nEntry)->pCollectionsInfo_;
        pColsInfo->startIterator();
        while (pColsInfo->hasNext())
            colIDSet.push_back(pColsInfo->next()->getId());
    }
    sort(colIDSet.begin(), colIDSet.end());
    vector<collectionid_t>::iterator it = unique(colIDSet.begin(),colIDSet.end());
    colIDSet.resize(it - colIDSet.begin());

    for (vector<collectionid_t>::const_iterator p = colIDSet.begin(); p != colIDSet.end(); ++p)
    {
        bFinish = false;
        fieldid = 1;
        pFieldsInfo = NULL;

        for (nEntry = 0;nEntry < nEntryCount;nEntry++)
        {
            pEntry = pBarrel->getAt(nEntry);
            pEntry->setCurrColID(*p);
            pColInfo = pEntry->pCollectionsInfo_->getCollectionInfo(*p);
            pColInfo->getFieldsInfo()->startIterator();
        }

        while (!bFinish)
        {
        
            for (nEntry = 0;nEntry < nEntryCount;nEntry++)
            {
                pEntry = pBarrel->getAt(nEntry);
                pColInfo = pEntry->pCollectionsInfo_->getCollectionInfo(*p);

                if (NULL == pColInfo)
                {
                    bFinish = true;
                    break;
                }

                //if (pColInfo->getFieldsInfo()->numFields() > (fieldid-1))
                if(pColInfo->getFieldsInfo()->hasNext())
                {
                    //pFieldInfo = pColInfo->getFieldsInfo()->getField(fieldid);///get field information
                    pFieldInfo = pColInfo->getFieldsInfo()->next();
                    if (pFieldInfo)
                    {
                        if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())///it's a index field
                        {
                            if (pFieldMerger == NULL)
                            {
                                pFieldMerger = new FieldMerger(needSortingMerge, 
                                                                                 pIndexer_->getSkipInterval(), 
                                                                                 pIndexer_->getMaxSkipLevel());
                                pFieldMerger->setDirectory(pDirectory_);
                                if(NULL == pIndexer_->getIndexWriter()->pMemCache_) pIndexer_->getIndexWriter()->createMemCache();
                                pFieldMerger->initPostingMerger(pIndexer_->getIndexCompressType(), optimize_, pIndexer_->getIndexWriter()->pMemCache_);
                                if(pDocFilter_)
                                    pFieldMerger->setDocFilter(pDocFilter_);
                            }
                            pFieldInfo->setColID(*p);
                            pFieldMerger->addField(pEntry->pBarrelInfo_,pFieldInfo);///add to field merger
                        }
                    }
                }
            } // for

            if (pFieldInfo)
            {
                if (pFieldMerger && pFieldMerger->numFields() > 0)
                {
                    vocOff1 = pVocStream->getFilePointer();
                    dfiOff1 = pDStream->getFilePointer();
                    if (pOutputDesc->getPPostingOutput())
                        ptiOff1 = pOutputDesc->getPPostingOutput()->getFilePointer();

                    voffset = pFieldMerger->merge(pOutputDesc);
                    pFieldInfo->setIndexOffset(voffset);///set offset of this field's index data

                    vocOff2 = pVocStream->getFilePointer();
                    dfiOff2 = pDStream->getFilePointer();
                    ptiOff2 = pOutputDesc->getPPostingOutput()->getFilePointer();
                    pFieldInfo->setDistinctNumTerms(pFieldMerger->numMergedTerms());

                    pFieldInfo->setLength(vocOff2-vocOff1,dfiOff2-dfiOff1,ptiOff2-ptiOff1);

                    delete pFieldMerger;
                    pFieldMerger = NULL;
                }
                if (pFieldsInfo == NULL)
                    pFieldsInfo = new FieldsInfo();
                pFieldsInfo->addField(pFieldInfo);

                fieldid++;
                pFieldInfo = NULL;
            }
            else
            {
                bFinish = true;
            }
        } // while
        CollectionInfo* pCollectionInfo = new CollectionInfo(*p, pFieldsInfo);
        pCollectionInfo->setOwn(true);
        collectionsInfo.addCollection(pCollectionInfo);
    } // for

    {
    //delete all merged barrels
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(pIndexer_->mutex_);
    for (nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);
        pBarrelsInfo_->removeBarrel(pDirectory_,pEntry->pBarrelInfo_->getName());///delete merged barrels
    }

    if (pMergeBarrels_)
    {
        removeMergedBarrels(pBarrel);
    }
    pBarrel->clear();

    name = newBarrelName + ".fdi";
    IndexOutput* fieldsStream = pDirectory_->createOutput(name);
    //fieldsInfo.write(fieldsStream);//field information
    collectionsInfo.write(fieldsStream);
    fieldsStream->flush();
    delete fieldsStream;
    delete pOutputDesc;
    pOutputDesc = NULL;

    pNewBarrelInfo->setSearchable(true);
    pBarrelsInfo_->addBarrel(pNewBarrelInfo,false);

    ///sleep is necessary because if a query get termreader before this lock,
    ///the query has not been finished even the index file/term dictionary info has been changed
    ///500ms is used to let these queries finish their flow.
    //if(pIndexer_->isRealTime())
        //boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(5000));
    }

    //////////////////////////////////////////////////////////////////////////
    DVLOG(2)<< "<= IndexMerger::mergeBarrel(), barrel name: " << pBarrel->getIdentifier() << ", doc count: " << pNewBarrelInfo->getDocCount();

    MergeBarrelEntry* pNewEntry = new MergeBarrelEntry(pDirectory_,pNewBarrelInfo);
    addBarrel(pNewEntry);
}

void IndexMerger::removeMergedBarrels(MergeBarrel * pBarrel)
{
    if (!pMergeBarrels_)
        return;
    vector<MergeBarrelEntry*>::iterator iter = pMergeBarrels_->begin();
    size_t nEntry = 0;
    bool bRemoved = false;
    uint32_t nRemoved = 0;
    while (iter != pMergeBarrels_->end())
    {
        bRemoved = false;
        for (nEntry = 0;nEntry < pBarrel->size();nEntry++)
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

