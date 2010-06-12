#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/Indexer.h>

#include <util/izene_log.h>
#include <util/ThreadModel.h>

#include <boost/thread.hpp>

using namespace izenelib::ir::indexmanager;

IndexBarrelWriter::IndexBarrelWriter(Indexer* pIndex,MemCache* pCache,const char* name)
        :barrelName_(name)
        ,pIndexer_(pIndex)
        ,pMemCache_(pCache)

{
    pCollectionsInfo_ = new CollectionsInfo();
    pDirectory_ = pIndexer_->getDirectory();
}

IndexBarrelWriter::~IndexBarrelWriter(void)
{
    pMemCache_ = NULL;

    if (pCollectionsInfo_)
        delete pCollectionsInfo_;

    for (CollectionIndexerMap::iterator iter = collectionIndexerMap_.begin(); 
        iter != collectionIndexerMap_.end(); ++iter)
        delete iter->second;
}

void IndexBarrelWriter::open(const char* barrelName)
{
    barrelName_ = barrelName;
    pCollectionsInfo_->reset();
}
void IndexBarrelWriter::close()
{
    if (cacheEmpty() == false)
    {
        writeCache();
        resetCache();
    }
    pDocFilter_ = NULL;
}
void IndexBarrelWriter::rename(const char* newName)
{
    pDirectory_->renameFiles(barrelName_,newName);
    barrelName_ = newName;
}

void IndexBarrelWriter::addDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);
    CollectionIndexer* pCollectionIndexer = collectionIndexerMap_[uniqueID.colId];
    if (NULL == pCollectionIndexer)
        SF1V5_THROW(ERROR_OUTOFRANGE,"IndexBarrelWriter::addDocument(): collection id does not belong to the range");
    pCollectionIndexer->addDocument(doc);
}

void IndexBarrelWriter::resetCache(bool bResetPosting)
{
    if (bResetPosting)
    {
        for (CollectionIndexerMap::iterator p = collectionIndexerMap_.begin( ); p != collectionIndexerMap_.end( ); ++p)
            (*p).second->reset();
    }

    pMemCache_->flushMem();
}

void IndexBarrelWriter::writeCache()
{
    try
    {
        DLOG(INFO) << "Write Index Barrel" << endl;

        string s = barrelName_ +".voc";
        IndexOutput* pVocOutput = pDirectory_->createOutput(s.c_str());

        s = barrelName_ + ".dfp";
        IndexOutput* pDOutput = pDirectory_->createOutput(s.c_str());

        s = barrelName_ + ".pop";
        IndexOutput* pPOutput = pDirectory_->createOutput(s.c_str());

        OutputDescriptor desc(pVocOutput,pDOutput,pPOutput,true);

        for (CollectionIndexerMap::iterator p = collectionIndexerMap_.begin(); p != collectionIndexerMap_.end(); ++p)
            p->second->write(&desc);

        s = barrelName_ + ".fdi";
        IndexOutput* fdiOutput = pDirectory_->createOutput(s.c_str());

        pCollectionsInfo_->write(fdiOutput);
        fdiOutput->flush();
        delete fdiOutput;
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
        pCollectionIndexer = new CollectionIndexer(colID, pMemCache_, pIndexer_);
        pCollectionIndexer->setSchema((iter->second));
        pCollectionIndexer->setFieldIndexers();
        collectionIndexerMap_.insert(make_pair(colID,pCollectionIndexer));
        pCollectionInfo = new CollectionInfo(colID, pCollectionIndexer->getFieldsInfo());
        pCollectionsInfo_->addCollection(pCollectionInfo);
    }
}

