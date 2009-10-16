#include <ir/index_manager/index/SingleIndexBarrelReader.h>
#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/Indexer.h>

using namespace izenelib::ir::indexmanager;

SingleIndexBarrelReader::SingleIndexBarrelReader(Indexer* pIndex, BarrelInfo* pBarrel,DiskIndexOpenMode mode)
        : IndexBarrelReader(pIndex)
        , pBarrelInfo_(pBarrel)
{
    pCollectionsInfo_ = new CollectionsInfo();
    open(pBarrelInfo_->getName().c_str(),mode);
}

SingleIndexBarrelReader::~SingleIndexBarrelReader(void)
{
    close();
    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
        delete iter->second;
    termReaderMap_.clear();
    delete pCollectionsInfo_;
}

void SingleIndexBarrelReader::open(const char* name,DiskIndexOpenMode mode)
{
    this->name_ = name;

    Directory* pDirectory = pIndexer->getDirectory();
    string s = name;
    s+= ".fdi";
    IndexInput* pIndexInput = pDirectory->openInput(s.c_str());
    pCollectionsInfo_->read(pIndexInput);
    pIndexInput->close();
    delete pIndexInput;

    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
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
            pTermReader = new MultiFieldTermReader(pDirectory,name,pFieldsInfo,mode);
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
                    pTermReader = new DiskTermReader(mode);
                    pTermReader->open(pDirectory,name,pFieldInfo);
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

