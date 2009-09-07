#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/OnlineIndexMerger.h>
#include <ir/index_manager/index/OfflineIndexMerger.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

IndexWriter::IndexWriter(Indexer* pIndex)
        :pIndexBarrelWriter_(NULL)
        ,ppCachedDocs_(NULL)
        ,nNumCacheUsed_(0)
        ,pCurBarrelInfo_(NULL)
        ,pIndexMerger_(NULL)
        ,pMemCache_(NULL)
        ,pIndexer_(pIndex)
        ,pCurDocCount_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
    nNumCachedDocs_= pIndexer_->getIndexManagerConfig()->indexStrategy_.cacheDocs_;
    setupCache();
}

IndexWriter::~IndexWriter()
{
    destroyCache();
    if (pMemCache_)
    {
        delete pMemCache_;
        pMemCache_ = NULL;
    }
    if (pIndexBarrelWriter_)
        delete pIndexBarrelWriter_;
    if (pIndexMerger_)
        delete pIndexMerger_;
}

void IndexWriter::setupCache()
{
    if (ppCachedDocs_)
        destroyCache();
    ppCachedDocs_ = new IndexerDocument*[nNumCachedDocs_];
    for (int i = 0;i<nNumCachedDocs_;i++)
        ppCachedDocs_[i] = NULL;

}

void IndexWriter::clearCache()
{
    if (ppCachedDocs_)
    {
        for (int i = 0;i<nNumCachedDocs_;i++)
        {
            if (ppCachedDocs_[i] != NULL)
            {
                delete ppCachedDocs_[i];
                ppCachedDocs_[i] = NULL;
            }
        }
    }
    nNumCacheUsed_ = 0;
}

void IndexWriter::destroyCache()
{
    clearCache();
    delete[] ppCachedDocs_;
    ppCachedDocs_ = NULL;
    nNumCacheUsed_ = 0;
}

void IndexWriter::createMerger()
{
    if (!strcasecmp(pIndexer_->getIndexManagerConfig()->mergeStrategy_.strategy_.c_str(),"online"))
    {
        pIndexMerger_ = new OnlineIndexMerger(pIndexer_->getDirectory());
        pIndexMerger_->setParam(pIndexer_->getIndexManagerConfig()->mergeStrategy_.param_.c_str());
    }
    else if (!strcasecmp(pIndexer_->getIndexManagerConfig()->mergeStrategy_.strategy_.c_str(),"offline"))
        pIndexMerger_ = NULL;
    else
        SF1V5_THROW(ERROR_FILEIO,"Configuration values for index merging strategy have not been set" );
}

void IndexWriter::mergeIndex(IndexMerger* pMerger)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    pMerger->setDirectory(pIndexer_->getDirectory());
    ///there is a in-memory index
    if ((pIndexBarrelWriter_) && pCurDocCount_ && ((*pCurDocCount_) > 0))
    {
        IndexMerger* pTmp = pIndexMerger_;
        pIndexMerger_ = pMerger;
        mergeAndWriteCachedIndex();
        pIndexMerger_ = pTmp;
    }
    else
    {

        if (pCurBarrelInfo_)
        {
            pBarrelsInfo_->deleteLastBarrel();
            pCurBarrelInfo_ = NULL;
            pCurDocCount_ = NULL;
        }
        pIndexer_->setDirty(true);
        pMerger->merge(pBarrelsInfo_);
    }
}


void IndexWriter::createBarrelWriter()
{
    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;

    if (!pMemCache_)
        pMemCache_ = new MemCache((size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_);
    pIndexBarrelWriter_ = new IndexBarrelWriter(pIndexer_,pMemCache_,pCurBarrelInfo_->getName().c_str());
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pIndexBarrelWriter_->setCollectionsMeta(pIndexer_->getCollectionsMeta());
    createMerger();
}

void IndexWriter::mergeAndWriteCachedIndex()
{
    pIndexer_->setDirty(true);
    pIndexMerger_->merge(pBarrelsInfo_);
    pBarrelsInfo_->write(pIndexer_->getDirectory());
    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
    }

    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriter::flush()
{
    flushDocuments();
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    if (pLastBarrel == NULL)
        return;
    pLastBarrel->setBaseDocID(baseDocIDMap_);
    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        if (pIndexMerger_)
            pIndexMerger_->transferToDisk(pIndexBarrelWriter_->barrelName.c_str());
    }
    pIndexer_->setDirty(true);
    pLastBarrel->setWriter(NULL);
    pBarrelsInfo_->write(pIndexer_->getDirectory());
}

void IndexWriter::close()
{
    if (!pIndexBarrelWriter_)
        return;
    flush();
}

void IndexWriter::mergeAndWriteCachedIndex2()
{
    pIndexer_->setDirty(true);

    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    pLastBarrel->setBaseDocID(baseDocIDMap_);

    if (pIndexBarrelWriter_->cacheEmpty() == false)///memory index has not been written to database yet.
    {
        pIndexBarrelWriter_->close();
        pLastBarrel->setWriter(NULL);

        if (pIndexMerger_)
            pIndexMerger_->addToMerge(pBarrelsInfo_,pBarrelsInfo_->getLastBarrel());
		
        if (pIndexMerger_)
            pIndexMerger_->transferToDisk(pIndexBarrelWriter_->barrelName.c_str());
    }

    pBarrelsInfo_->addBarrel(pBarrelsInfo_->newBarrel().c_str(),0);
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurBarrelInfo_->setWriter(pIndexBarrelWriter_);
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriter::justWriteCachedIndex()
{
///Used for MANAGER_TYPE_DATAPROCESS
///It does not update barrel info, only flush indices to barrel "_0"
    pBarrelsInfo_->write(pIndexer_->getDirectory());
    pCurBarrelInfo_ = pBarrelsInfo_->getLastBarrel();
    pCurDocCount_ = &(pCurBarrelInfo_->nNumDocs);
    *pCurDocCount_ = 0;
}

void IndexWriter::addDocument(IndexerDocument* pDoc)
{
    //boost::mutex::scoped_lock lock(this->mutex_);
    ppCachedDocs_[nNumCacheUsed_++] = pDoc;

    if (isCacheFull())
        flushDocuments();
}

void IndexWriter::indexDocument(IndexerDocument* pDoc)
{
    if (!pIndexBarrelWriter_)
        createBarrelWriter();
    if (pIndexBarrelWriter_->cacheFull())
    {
         if(pIndexer_->getIndexerType() == MANAGER_TYPE_DATAPROCESS)
            justWriteCachedIndex();
        else
            ///merge index
            mergeAndWriteCachedIndex2();
        baseDocIDMap_.clear();
        DocId uniqueID;
        pDoc->getDocId(uniqueID);
        baseDocIDMap_[uniqueID.colId] = uniqueID.docId;
        pIndexBarrelWriter_->open(pCurBarrelInfo_->getName().c_str());

    }
    pIndexBarrelWriter_->addDocument(pDoc);
    (*pCurDocCount_)++;
}

void IndexWriter::flushDocuments()
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if (nNumCacheUsed_ <=0 )
        return;
    for (int i=0;i<nNumCacheUsed_;i++)
    {

        DocId uniqueID;
        ppCachedDocs_[i]->getDocId(uniqueID);
        if (baseDocIDMap_.find(uniqueID.colId) == baseDocIDMap_.end())
            baseDocIDMap_.insert(make_pair(uniqueID.colId,uniqueID.docId));
        indexDocument(ppCachedDocs_[i]);
    }

    clearCache();
}

BarrelInfo* IndexWriter::findDocumentInBarrels(collectionid_t colID, docid_t docID)
{
    for (int i = pBarrelsInfo_->getBarrelCount() - 1; i >= 0; i--)
    {
        BarrelInfo* pBarrelInfo = (*pBarrelsInfo_)[i];
        if ((pBarrelInfo->baseDocIDMap.find(colID) != pBarrelInfo->baseDocIDMap.end())&&
                (pBarrelInfo->baseDocIDMap[colID] <= docID))
            return pBarrelInfo;
    }
    return NULL;
}

bool IndexWriter::deleteDocument(IndexerDocument* pDoc)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    DocId uniqueID;
    pDoc->getDocId(uniqueID);
#if 1
    for (int i = pBarrelsInfo_->getBarrelCount() - 1; i >= 0; i--)
    {
        BarrelInfo* pBarrelInfo = (*pBarrelsInfo_)[i];
        if (pBarrelInfo == NULL)
            return false;
        IndexBarrelWriter* pBarrelWriter = pBarrelInfo->getWriter();
        bool inmemory = true;
        MemCache* tmpMemCache = NULL;
        if (NULL == pBarrelWriter)
        {
            ///TODO
            tmpMemCache = new MemCache((size_t)pIndexer_->getIndexManagerConfig()->advance_.uptightAlloc_.memSize_);
            cout<<"try to delete doc in barrel "<<pBarrelInfo->getName()<<endl;
            pBarrelWriter = new IndexBarrelWriter(pIndexer_,tmpMemCache,pBarrelInfo->getName().c_str());
            pBarrelWriter->setCollectionsMeta(pIndexer_->getCollectionsMeta());
            inmemory = false;
        }

        if (pBarrelWriter->deleteDocument(pDoc,inmemory))
            pBarrelInfo->deleteDocument();
        if (!inmemory)
            delete pBarrelWriter;
        if (tmpMemCache)
            delete tmpMemCache;
    }

    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);

    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (!iter->first.isForward())
        {
            pIndexer_->getBTreeIndexer()->remove(uniqueID.colId, iter->first.getPropertyId(), boost::get<PropertyType>(iter->second), uniqueID.docId);
        }
    }

#else
    BarrelInfo * pBarrelInfo = findDocumentInBarrels(uniqueID.collId, uniqueID.docId);
    if (pBarrelInfo == NULL)
        return false;

    IndexBarrelWriter* pBarrelWriter = pBarrelInfo->getWriter();
    bool inmemory = true;
    MemCache* tmpMemCache = NULL;
    if (NULL == pBarrelWriter)
    {
        ///TODO
        tmpMemCache = new MemCache((size_t)pIndexer_->getIndexManagerConfig()->advance_.uptightAlloc_.memSize_);
        cout<<"document lies in barrel "<<pBarrelInfo->getName()<<endl;
        pBarrelWriter = new IndexBarrelWriter(pIndexer_,tmpMemCache,pBarrelInfo->getName().c_str());
        pBarrelWriter->setCollectionsMeta(pIndexer_->getCollectionsMeta());
        inmemory = false;
    }

    pBarrelWriter->deleteDocument(pDoc,inmemory);

    pBarrelInfo->deleteDocument();

    if (!inmemory)
        delete pBarrelWriter;
    if (tmpMemCache)
        delete tmpMemCache;
#endif
    pIndexer_->setDirty(true);//flush barrelsinfo when Indexer quit
    return true;
}

bool IndexWriter::removeCollection(collectionid_t colID, count_t colCount)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    Directory* pDirectory = pIndexer_->getDirectory();

    BarrelInfo* pPreviousBarrelInfo = NULL;
    for (int i = 0; i < pBarrelsInfo_->getBarrelCount(); i++)
    {
        BarrelInfo* pBarrelInfo = (*pBarrelsInfo_)[i];

        if (pPreviousBarrelInfo)
        {
            count_t prevBarrelDocCount = pBarrelInfo->baseDocIDMap[colID] - pPreviousBarrelInfo->baseDocIDMap[colID];
            colCount -= prevBarrelDocCount;
            pPreviousBarrelInfo->setDocCount(pPreviousBarrelInfo->getDocCount() - prevBarrelDocCount);
        }
        pPreviousBarrelInfo = pBarrelInfo;
        pBarrelInfo->baseDocIDMap.erase(colID);
        if (pBarrelInfo->baseDocIDMap.empty())
        {
            pBarrelsInfo_->removeBarrel(pDirectory,pBarrelInfo->getName());
            pPreviousBarrelInfo = NULL;
            i--;
            continue;
        }

        string s = pBarrelInfo->getName() + ".fdi";
        IndexInput* fdiInput = pDirectory->openInput(s.c_str());
        CollectionsInfo* pCollectionsInfo = new CollectionsInfo();
        pCollectionsInfo->read(fdiInput);
        pCollectionsInfo->removeCollectionInfo(colID);
        fdiInput->close();
        delete fdiInput;
        pDirectory->deleteFile(s);
        IndexOutput* fieldsStream = pDirectory->createOutput(s.c_str());
        pCollectionsInfo->write(fieldsStream);
        delete fieldsStream;
    }
    if (pPreviousBarrelInfo)
    {
        pPreviousBarrelInfo->setDocCount(colCount);
    }
    pBarrelsInfo_->write(pDirectory);
    return true;
}


