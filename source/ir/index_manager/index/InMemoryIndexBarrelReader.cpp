#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/InMemoryIndexBarrelReader.h>
#include <ir/index_manager/index/MultiFieldTermReader.h>


using namespace izenelib::ir::indexmanager;

InMemoryIndexBarrelReader::InMemoryIndexBarrelReader(IndexBarrelWriter* pIndexBarrelWriter)
        :pIndexBarrelWriter_(pIndexBarrelWriter)
{
    CollectionsInfo* pCollectionsInfo = pIndexBarrelWriter_->pCollectionsInfo_;
    pCollectionsInfo->startIterator();
    CollectionInfo* pColInfo = NULL;
    FieldsInfo* pFieldsInfo = NULL;
    FieldInfo* pFieldInfo = NULL;
    TermReader* pTermReader = NULL;
    CollectionIndexer* pCollectionIndexer = NULL;
    while (pCollectionsInfo->hasNext())
    {
        pColInfo = pCollectionsInfo->next();
        pFieldsInfo = pColInfo->getFieldsInfo();
        pCollectionIndexer = pIndexBarrelWriter_->getCollectionIndexer(pColInfo->getId());
        if (pFieldsInfo->numIndexFields() == 1)
        {
            pFieldsInfo->startIterator();
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())
                    break;
            }
            pTermReader = pCollectionIndexer->getFieldIndexer(pFieldInfo->getName())->termReader();
            pTermReader->setDocFilter(pIndexBarrelWriter_->getDocFilter());
        }
        else if (pFieldsInfo->numIndexFields() > 1)
        {
            pTermReader = new MultiFieldTermReader();
            pFieldsInfo->startIterator();
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())
                {
                    FieldIndexer* pFieldIndexer = pCollectionIndexer->getFieldIndexer(pFieldInfo->getName());
                    if (pFieldIndexer)
                    {
                        ((MultiFieldTermReader*)pTermReader)->addTermReader( pFieldInfo->getName(),pFieldIndexer->termReader());
                    }
                }
            }
            pTermReader->setDocFilter(pIndexBarrelWriter_->getDocFilter());
        }

        termReaderMap_.insert(pair<collectionid_t, TermReader*>(pColInfo->getId(), pTermReader));
    }
}

InMemoryIndexBarrelReader::~InMemoryIndexBarrelReader(void)
{
    close();

    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); 
        iter != termReaderMap_.end(); ++iter)
        delete iter->second;

    termReaderMap_.clear();

    pIndexBarrelWriter_ = NULL;
}

TermReader* InMemoryIndexBarrelReader::termReader(collectionid_t colID)
{
    return termReaderMap_[colID];//->clone();
}


TermReader* InMemoryIndexBarrelReader::termReader(collectionid_t colID, const char* field)
{
    TermReader* pTermReader = termReaderMap_[colID];
    if (pTermReader == NULL)
        return NULL;
    FieldsInfo* pFieldsInfo = pIndexBarrelWriter_->pCollectionsInfo_->getCollectionInfo(colID)->getFieldsInfo();
    if (pFieldsInfo->numIndexFields() > 1)
        return ((MultiFieldTermReader*)pTermReader)->termReader(field);
    else
    {
        pFieldsInfo->startIterator();
        FieldInfo* pFieldInfo;
        while (pFieldsInfo->hasNext())
        {
            pFieldInfo = pFieldsInfo->next();
            if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())
            {
                if (!strcasecmp(field,pFieldInfo->getName()))
                    return pTermReader;//->clone();
            }
        }
    }
    return NULL;
}

size_t InMemoryIndexBarrelReader::getDistinctNumTerms(collectionid_t colID, const std::string& property)
{
    CollectionsInfo* pCollectionsInfo = pIndexBarrelWriter_->pCollectionsInfo_;
    return (*pCollectionsInfo)[colID]->getFieldsInfo()->getField(property.c_str())->distinctNumTerms();
}

void InMemoryIndexBarrelReader::close()
{
}

