#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/Indexer.h>

#include <util/izene_log.h>
#include <util/ThreadModel.h>

#include <boost/thread.hpp>

using namespace izenelib::ir::indexmanager;

IndexBarrelWriter::IndexBarrelWriter(Indexer* pIndex,MemCache* pCache,const char* name)
        :barrelName(name)
        ,pIndexer(pIndex)
        ,pMemCache(pCache)
        ,pForwardIndexWriter_(NULL)

{
    pCollectionsInfo = new CollectionsInfo();
    pDirectory = pIndexer->getDirectory();

    if(pIndexer->indexingForward_)
        pForwardIndexWriter_ = new ForwardIndexWriter(pDirectory);
}

IndexBarrelWriter::~IndexBarrelWriter(void)
{
    pMemCache = NULL;

    if (pCollectionsInfo)
    {
        delete pCollectionsInfo;
        pCollectionsInfo = NULL;
    }

    for (CollectionIndexerMap::iterator iter = collectionIndexMap.begin(); iter != collectionIndexMap.end(); ++iter)
    {
        delete iter->second;
    }
    if(pForwardIndexWriter_)
    {
        delete pForwardIndexWriter_;
        pForwardIndexWriter_ = NULL;		
    }
    collectionIndexMap.clear();
}

void IndexBarrelWriter::open(const char* barrelName_)
{
    barrelName = barrelName_;
    pCollectionsInfo->reset();
}
void IndexBarrelWriter::close()
{
    //boost::mutex::scoped_lock lock(pIndexer->mutex_);
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(pIndexer->mutex_);

    if (cacheEmpty() == false)
    {
        writeCache();
        resetCache();
    }
}
void IndexBarrelWriter::rename(const char* newName)
{
    pDirectory->renameFiles(barrelName,newName);
    barrelName = newName;
}

void IndexBarrelWriter::addDocument(IndexerDocument* pDoc)
{
    DocId uniqueID;
    pDoc->getDocId(uniqueID);
    CollectionIndexer* pCollectionIndexer = collectionIndexMap[uniqueID.colId];
    if (NULL == pCollectionIndexer)
        SF1V5_THROW(ERROR_OUTOFRANGE,"IndexBarrelWriter::addDocument(): collection id does not belong to the range");
    pCollectionIndexer->addDocument(pDoc);
}

bool	IndexBarrelWriter::deleteDocument(IndexerDocument* pDoc)
{
    DocId uniqueID;
    pDoc->getDocId(uniqueID);
    CollectionIndexer* pCollectionIndexer = collectionIndexMap[uniqueID.colId];
    return pCollectionIndexer->deleteDocument(pDoc);
}

void IndexBarrelWriter::resetCache(bool bResetPosting /* = false */)
{
    if (bResetPosting)
    {
        for (CollectionIndexerMap::iterator p = collectionIndexMap.begin( ); p != collectionIndexMap.end( ); ++p)
            (*p).second->reset();
    }

    pMemCache->flushMem();
}

void IndexBarrelWriter::writeCache()
{
    try
    {
        DLOG(INFO) << "Write Index Barrel" << endl;

        string s = barrelName +".voc";
        IndexOutput* pVocOutput = pDirectory->createOutput(s.c_str());

        s = barrelName + ".dfp";
        IndexOutput* pDOutput = pDirectory->createOutput(s.c_str());

        s = barrelName + ".pop";
        IndexOutput* pPOutput = pDirectory->createOutput(s.c_str());

        OutputDescriptor desc(pVocOutput,pDOutput,pPOutput,true);

        for (CollectionIndexerMap::iterator p = collectionIndexMap.begin(); p != collectionIndexMap.end(); ++p)
            p->second->write(&desc);

        s = barrelName + ".fdi";
        IndexOutput* fdiOutput = pDirectory->createOutput(s.c_str());

        pCollectionsInfo->write(fdiOutput);
        fdiOutput->flush();
        delete fdiOutput;
        if(pForwardIndexWriter_)
            pForwardIndexWriter_->flush();
    }
    catch (const FileIOException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (std::bad_alloc& be)
    {
        string serror = be.what();
        SF1V5_THROW(ERROR_OUTOFMEM,"IndexBarrelWriter::writeCache():alloc memory failed.-" + serror);
    }
}


void IndexBarrelWriter::setCollectionsMeta(const std::map<std::string, IndexerCollectionMeta>& collectionsMeta)
{
    std::map<std::string, IndexerCollectionMeta>::const_iterator iter;
    collectionid_t colID;
    CollectionInfo* pCollectionInfo = NULL;
    CollectionIndexer* pCollectionIndexer = NULL;

    for (iter = collectionsMeta.begin(); iter != collectionsMeta.end(); iter++)
    {
        colID = (iter->second).getColId ();
        pCollectionIndexer = new CollectionIndexer(colID, pMemCache, pIndexer);
        pCollectionIndexer->setSchema((iter->second));
        pCollectionIndexer->setFieldIndexers();
        if(pForwardIndexWriter_)
            pCollectionIndexer->setForwardIndexWriter(pForwardIndexWriter_);
        collectionIndexMap.insert(make_pair(colID,pCollectionIndexer));
        pCollectionInfo = new CollectionInfo(colID, pCollectionIndexer->getFieldsInfo());
        pCollectionsInfo->addCollection(pCollectionInfo);
    }
}

