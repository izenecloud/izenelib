#include <ir/index_manager/index/SingleIndexBarrelReader.h>
#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/Indexer.h>

#define MAX_MEM_POOL_SIZE_FOR_ONE_POSTING 10*1024*1024

using namespace izenelib::ir::indexmanager;

SingleIndexBarrelReader::SingleIndexBarrelReader(Indexer* pIndex, BarrelInfo* pBarrel)
        : IndexBarrelReader(pIndex)
        , pBarrelInfo_(pBarrel)
        , pMemCache_(NULL)
{
    pCollectionsInfo_ = new CollectionsInfo();
    open(pBarrelInfo_->getName().c_str());
}

SingleIndexBarrelReader::~SingleIndexBarrelReader(void)
{
    close();
    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
        delete iter->second;
    termReaderMap_.clear();
    delete pCollectionsInfo_;
    if(pMemCache_)
        delete pMemCache_;
}

void SingleIndexBarrelReader::open(const char* name)
{
    this->name_ = name;

    Directory* pDirectory = pIndexer_->getDirectory();
    string s = name;
    s+= ".fdi";
    IndexInput* pIndexInput = pDirectory->openInput(s.c_str());
    pCollectionsInfo_->read(pIndexInput);
    pIndexInput->close();
    delete pIndexInput;

    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); 
            iter != termReaderMap_.end(); ++iter)
        delete iter->second;

    termReaderMap_.clear();

    pCollectionsInfo_->startIterator();
    CollectionInfo* pColInfo = NULL;
    FieldsInfo* pFieldsInfo = NULL;
    TermReader* pTermReader = NULL;
    while (pCollectionsInfo_->hasNext())
    {
        pColInfo = pCollectionsInfo_->next();
        pFieldsInfo = pColInfo->getFieldsInfo();
        if (pFieldsInfo->numIndexFields() > 1)
        {
            pTermReader = new MultiFieldTermReader(pDirectory,name,pFieldsInfo);
        }
        else if (pFieldsInfo->numIndexFields() == 1)
        {
            pFieldsInfo->startIterator();
            FieldInfo* pFieldInfo;
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
                {
                    pTermReader = new DiskTermReader(pDirectory,name,pFieldInfo);
                    break;
                }
            }
        }
        else
        {
            SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the field number is 0.");
        }
        termReaderMap_.insert(pair<collectionid_t, TermReader*>(pColInfo->getId(),pTermReader));
    }

}

void SingleIndexBarrelReader::reopen()
{
    if(pBarrelInfo_->isModified())
    {
        for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); 
                iter != termReaderMap_.end(); ++iter)
            iter->second->reopen();
    }
}

TermReader* SingleIndexBarrelReader::termReader(collectionid_t colID, const char* field)
{
    TermReader* pTermReader = termReaderMap_[colID];
    if (pTermReader == NULL)
        return NULL;
    FieldsInfo* pFieldsInfo = pCollectionsInfo_->getCollectionInfo(colID)->getFieldsInfo();
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

TermReader* SingleIndexBarrelReader::termReader(collectionid_t colID)
{
    return termReaderMap_[colID];//->clone();
}

size_t SingleIndexBarrelReader::getDistinctNumTerms(collectionid_t colID, const std::string& property)
{
    FieldsInfo* pFieldsInfo = (*pCollectionsInfo_)[colID]->getFieldsInfo();
    FieldInfo* pFieldInfo = pFieldsInfo->getField(property.c_str());
    if(NULL == pFieldInfo)
        return 0;
    else
        return pFieldInfo->distinctNumTerms();
}

void SingleIndexBarrelReader::close()
{
    map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin();
    while (iter!=termReaderMap_.end())
    {
        if (iter->second)
            iter->second->close();
        iter++;
    }

}

