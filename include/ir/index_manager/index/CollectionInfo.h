/**
* @file        CollectionInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief associate the FieldsInfo object with each Collection
*/
#ifndef COLLECTIONINFO_H
#define COLLECTIONINFO_H

#include <ir/index_manager/utility/Logger.h>
#include <ir/index_manager/index/FieldInfo.h>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
*@brief Associate FieldsInfo object with each collection
*@note Each collection has different FieldsInfo
*/
class CollectionInfo
{
public:
    CollectionInfo(collectionid_t id_);
    CollectionInfo(collectionid_t id_, FieldsInfo* pInfo);
    CollectionInfo(CollectionInfo& src);
    ~CollectionInfo();
public:
    void setId(collectionid_t id)
    {
        this->id = id;
    }
    collectionid_t getId()
    {
        return id;
    }
    ///set the FieldsInfo object to associate it with collection id.
    void setFieldsInfo(FieldsInfo* pInfo)
    {
        pFieldsInfo = pInfo;
    }
    ///return the associated FieldsInfo object
    FieldsInfo* getFieldsInfo()
    {
        return pFieldsInfo;
    }

    void setOwn(bool own)
    {
        ownFieldInfo = own;
    }
private:
    collectionid_t id;
    FieldsInfo* pFieldsInfo;
    bool ownFieldInfo;
};
/**
*@brief A collection of CollectionInfo, it is composed of CollectionInfo
*@note It is the manager class of CollectionInfo
*/

class CollectionsInfo
{
public:
    CollectionsInfo();
    CollectionsInfo(CollectionsInfo& src);
    ~CollectionsInfo();
public:
    void addCollection(CollectionInfo* pColInfo)
    {
        collectionsInfoMap.insert(std::pair<collectionid_t, CollectionInfo*>(pColInfo->getId(),pColInfo));
    }
    /**
    * read collectionsInfo from "fdi" file
    */
    void read(IndexInput* pIndexInput);
    /**
    * read collectionsInfo from "fdi" file
    */
    void write(IndexOutput* pIndexOutput);

    void clear();

    void reset();

    inline CollectionInfo* getCollectionInfo(collectionid_t cid)
    {
        return collectionsInfoMap[cid];
    }

    int32_t numCollections()
    {
        return (int32_t)collectionsInfoMap.size();
    }

public:
    CollectionInfo* operator[](int32_t i)
    {
        return collectionsInfoMap[i];
    }
    void startIterator()
    {
        colInfosIterator = collectionsInfoMap.begin();
    }
    bool hasNext()
    {
        return (colInfosIterator != collectionsInfoMap.end());
    }
    CollectionInfo* next()
    {
        return (colInfosIterator++)->second;
    }
    void removeCollectionInfo(collectionid_t colID);
private:
    std::map<collectionid_t, CollectionInfo*> collectionsInfoMap;
    std::map<collectionid_t, CollectionInfo*>::iterator colInfosIterator;
};

}

NS_IZENELIB_IR_END

#endif
