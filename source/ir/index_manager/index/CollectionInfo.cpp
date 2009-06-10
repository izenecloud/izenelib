#include <ir/index_manager/index/CollectionInfo.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

CollectionInfo::CollectionInfo(collectionid_t id_)
        :id(id_)
        ,ownFieldInfo(false)
{}

CollectionInfo::CollectionInfo(collectionid_t id_, FieldsInfo* pInfo)
        :id(id_)
        ,pFieldsInfo(pInfo)
        ,ownFieldInfo(false)
{
}

CollectionInfo::CollectionInfo(CollectionInfo& src)
        :id(src.id)
        ,ownFieldInfo(true)
{
    pFieldsInfo = new FieldsInfo(*(src.pFieldsInfo));
}

CollectionInfo::~CollectionInfo()
{
    if (ownFieldInfo&&pFieldsInfo)
        delete pFieldsInfo;
    pFieldsInfo = NULL;
}

CollectionsInfo::CollectionsInfo()
{
}

CollectionsInfo::CollectionsInfo(CollectionsInfo& src)
{
    for (map<collectionid_t, CollectionInfo*>::iterator iter = src.collectionsInfoMap.begin();
            iter != src.collectionsInfoMap.end(); ++iter)
    {
        CollectionInfo* pColInfo = new CollectionInfo(*(iter->second));
        collectionsInfoMap.insert(pair<collectionid_t, CollectionInfo*>(iter->first,pColInfo));
    }
}

CollectionsInfo::~CollectionsInfo()
{
    clear();
}

void CollectionsInfo::clear()
{
    for (map<collectionid_t, CollectionInfo*>::iterator iter = collectionsInfoMap.begin(); iter != collectionsInfoMap.end(); ++iter)
        delete iter->second;
    collectionsInfoMap.clear();
}

void CollectionsInfo::removeCollectionInfo(collectionid_t colID)
{
    CollectionInfo* pColInfo = collectionsInfoMap[colID];
    if (pColInfo)
    {
        delete pColInfo;
        collectionsInfoMap.erase(colID);
    }
}

void CollectionsInfo::read(IndexInput* pIndexInput)
{
    try
    {
        clear();

        int32_t count = pIndexInput->readInt(); ///<CollectionsCount(Int32)>
        if (count <= 0)
        {
            SF1V5_LOG(level::warn) << "CollectionsInfo::read():collection count <=0." << SF1V5_ENDL;
            return ;
        }

        CollectionInfo* pInfo = NULL;
        for (int32_t i = 0;i<count;i++)
        {
            int32_t colID = pIndexInput->readInt();///<Collection ID(Int32)>
            pInfo = new CollectionInfo(colID);

            FieldsInfo* pFieldsInfo = new FieldsInfo();
            pFieldsInfo->read(pIndexInput);
            pFieldsInfo->setColID(colID);

            pInfo->setFieldsInfo(pFieldsInfo);
            pInfo->setOwn(true);

            collectionsInfoMap.insert(pair<collectionid_t, CollectionInfo*>(colID,pInfo));

        }

    }
    catch (const IndexManagerException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (const bad_alloc& )
    {
        SF1V5_THROW(ERROR_OUTOFMEM,"CollectionsInfo.read():alloc memory failed.");
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNKNOWN,"CollectionsInfo::read failed.");
    }

}

void CollectionsInfo::write(IndexOutput* pIndexOutput)
{
    try
    {
        pIndexOutput->writeInt(numCollections());///<CollectionsCount(Int32)>
        CollectionInfo* pInfo;
        for (map<collectionid_t, CollectionInfo*>::iterator iter = collectionsInfoMap.begin(); iter != collectionsInfoMap.end(); ++iter)
        {
            pInfo = iter->second;
            pIndexOutput->writeInt(pInfo->getId());	///<CollectionID(Int32)>
            pInfo->getFieldsInfo()->write(pIndexOutput);
        }
    }
    catch (const IndexManagerException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (const bad_alloc& )
    {
        SF1V5_THROW(ERROR_OUTOFMEM,"CollectionsInfo.write():alloc memory failed.");
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNKNOWN,"CollectionsInfo.write() failed.");
    }

}

void CollectionsInfo::reset()
{
    for (map<collectionid_t, CollectionInfo*>::iterator iter = collectionsInfoMap.begin();iter != collectionsInfoMap.end(); ++iter)
    {
        iter->second->getFieldsInfo()->reset();
    }
}

