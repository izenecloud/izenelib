#include <ir/index_manager/index/InMemoryIndexBarrelReader.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/MultiFieldTermReader.h>


using namespace izenelib::ir::indexmanager;

InMemoryIndexBarrelReader::InMemoryIndexBarrelReader(IndexBarrelWriter* pIndexBarrelWriter_)
        :pIndexBarrelWriter(pIndexBarrelWriter_)
{
    CollectionsInfo* pCollectionsInfo = pIndexBarrelWriter->pCollectionsInfo;
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
        pCollectionIndexer = pIndexBarrelWriter->getCollectionIndexer(pColInfo->getId());
        if (pFieldsInfo->numIndexFields() == 1)
        {
            pFieldsInfo->startIterator();
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
                    break;
            }
            pTermReader = pCollectionIndexer->getFieldIndexer(pFieldInfo->getName())->termReader();
        }
        else if (pFieldsInfo->numIndexFields() > 1)
        {
            pTermReader = new MultiFieldTermReader();
            pFieldsInfo->startIterator();
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
                {
                    FieldIndexer* pFieldIndexer = pCollectionIndexer->getFieldIndexer(pFieldInfo->getName());
                    if (pFieldIndexer)
                    {
                        ((MultiFieldTermReader*)pTermReader)->addTermReader( pFieldInfo->getName(),pFieldIndexer->termReader());
                    }
                }
            }
        }

        termReaderMap.insert(pair<collectionid_t, TermReader*>(pColInfo->getId(), pTermReader));
    }
}

InMemoryIndexBarrelReader::~InMemoryIndexBarrelReader(void)
{
    close();

    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap.begin(); 
        iter != termReaderMap.end(); ++iter)
        delete iter->second;

    termReaderMap.clear();

    pIndexBarrelWriter = NULL;
}

TermReader* InMemoryIndexBarrelReader::termReader(collectionid_t colID)
{
    return termReaderMap[colID];//->clone();
}


TermReader* InMemoryIndexBarrelReader::termReader(collectionid_t colID, const char* field)
{
    TermReader* pTermReader = termReaderMap[colID];
    if (pTermReader == NULL)
        return NULL;
    FieldsInfo* pFieldsInfo = pIndexBarrelWriter->pCollectionsInfo->getCollectionInfo(colID)->getFieldsInfo();
    if (pFieldsInfo->numIndexFields() > 1)
        return ((MultiFieldTermReader*)pTermReader)->termReader(field);
    else
    {
        pFieldsInfo->startIterator();
        FieldInfo* pFieldInfo;
        while (pFieldsInfo->hasNext())
        {
            pFieldInfo = pFieldsInfo->next();
            if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
            {
                if (!strcasecmp(field,pFieldInfo->getName()))
                    return pTermReader;//->clone();
            }
        }
    }
    return NULL;
}

void InMemoryIndexBarrelReader::deleteDocumentPhysically(IndexerDocument* pDoc)
{
    pIndexBarrelWriter->deleteDocument(pDoc);
}

size_t InMemoryIndexBarrelReader::getDistinctNumTerms(collectionid_t colID, const std::string& property)
{
    CollectionsInfo* pCollectionsInfo = pIndexBarrelWriter->pCollectionsInfo;
    return (*pCollectionsInfo)[colID]->getFieldsInfo()->getField(property.c_str())->distinctNumTerms();
}

void InMemoryIndexBarrelReader::close()
{
}

